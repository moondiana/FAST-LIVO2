#include "loop_manager.h"
#include "log_init.h"

#include "common_lib.h"

#ifdef G_m_s2
#undef G_m_s2
#endif

#include "common/keyframe.h"
#include "common/nav_state.h"
#include "core/loop_closing/loop_closing.h"

#include <pcl/common/transforms.h>
#include <pcl/io/pcd_io.h>

#include <cmath>
#include <iostream>

namespace {
lightning::CloudPtr toLooperCloud(const PointCloudXYZI::Ptr& cloud_body) {
  auto cloud = std::make_shared<lightning::PointCloudType>();
  cloud->reserve(cloud_body->size());
  for (const auto& pt : cloud_body->points) {
    lightning::PointType out_pt;
    out_pt.x = pt.x;
    out_pt.y = pt.y;
    out_pt.z = pt.z;
    out_pt.intensity = pt.intensity;
    out_pt.time = 0.0;
    cloud->push_back(out_pt);
  }
  return cloud;
}

lightning::NavState toNavState(const StatesGroup& state, double timestamp) {
  lightning::NavState ns;
  ns.timestamp_ = timestamp;
  ns.pos_ = state.pos_end;
  ns.rot_ = lightning::SO3(state.rot_end);
  ns.vel_ = state.vel_end;
  ns.bg_ = state.bias_g;
  return ns;
}
}  // namespace

LoopManager::~LoopManager() { finish(); }

void LoopManager::init(const Options& options) {
  options_ = options;
  if (!options_.enable) {
    return;
  }

  lightning::LoopClosing::Options lc_options;
  lc_options.verbose_ = true;
  lc_options.online_mode_ = options_.online_mode;
  lc_options.loop_kf_gap_ = options_.loop_kf_gap;
  lc_options.min_id_interval_ = options_.min_id_interval;
  lc_options.closest_id_th_ = options_.closest_id_th;
  lc_options.max_range_ = options_.max_range;
  lc_options.ndt_score_th_ = options_.ndt_score_th;
  lc_options.with_height_ = options_.with_height;

  loop_closing_ = std::make_shared<lightning::LoopClosing>(lc_options);
  loop_closing_->SetLoopClosedCB([this]() {
    ++loop_count_;
    appendLoopLog("LOOP CLOSED, total loops=" + std::to_string(loop_count_));
  });
  loop_closing_->Init("");

  const std::string msg = "[ Loop ] Loop closing enabled (online=" +
                          std::string(options_.online_mode ? "true" : "false") + ", kf_dis=" +
                          std::to_string(options_.kf_dis_th) + "m, kf_angle=" +
                          std::to_string(options_.kf_angle_deg) + "deg)";
  std::cout << "\033[1;32m" << msg << "\033[0m" << std::endl;
  appendLoopLog(msg);
}

void LoopManager::tryAddKeyframe(const StatesGroup& state, const PointCloudXYZI::Ptr& cloud_body, double timestamp) {
  
  if (!enabled() || cloud_body == nullptr || cloud_body->empty()) {
    return;
  }

  const double kf_angle_th = options_.kf_angle_deg * M_PI / 180.0;
  const lightning::SE3 cur_pose(lightning::SO3(state.rot_end), state.pos_end);

  {
    std::lock_guard<std::mutex> lock(keyframes_mutex_);
    if (!keyframes_.empty()) {
      const auto& last_kf = keyframes_.back();
      const lightning::SE3 last_pose = last_kf->GetLIOPose();
      const double dist = (last_pose.translation() - cur_pose.translation()).norm();
      const double angle = (last_pose.so3().inverse() * cur_pose.so3()).log().norm();
      if (dist <= options_.kf_dis_th && angle <= kf_angle_th) {
        return;
      }
    }
  }

  auto cloud = toLooperCloud(cloud_body);
  std::shared_ptr<lightning::Keyframe> kf;
  size_t kf_id = 0;
  {
    std::lock_guard<std::mutex> lock(keyframes_mutex_);
    kf_id = keyframes_.size();
    kf = std::make_shared<lightning::Keyframe>(kf_id, cloud, toNavState(state, timestamp));
    if (!keyframes_.empty()) {
      const auto& last_kf = keyframes_.back();
      const lightning::SE3 delta = last_kf->GetLIOPose().inverse() * kf->GetLIOPose();
      kf->SetOptPose(last_kf->GetOptPose() * delta);
    } else {
      kf->SetOptPose(kf->GetLIOPose());
    }
    keyframes_.push_back(kf);
  }

  loop_closing_->AddKF(kf);

  const std::string msg = "[ Loop ] add keyframe " + std::to_string(kf_id) +
                          ", cloud_size=" + std::to_string(cloud_body->size());
  std::cout << msg << std::endl;
  appendLoopLog(msg);
}

void LoopManager::finish() {
  if (loop_closing_ != nullptr) {
    appendLoopLog("[ Loop ] finish loop closing, keyframes=" + std::to_string(keyframeCount()) +
                  ", loops=" + std::to_string(loop_count_));
    loop_closing_.reset();
  }
}

size_t LoopManager::keyframeCount() const {
  std::lock_guard<std::mutex> lock(keyframes_mutex_);
  return keyframes_.size();
}

bool LoopManager::saveCorrectedPointCloud(const std::string& root_dir) const {
  if (!options_.save_corrected_pcd) {
    return false;
  }

  std::vector<std::shared_ptr<lightning::Keyframe>> keyframes_copy;
  {
    std::lock_guard<std::mutex> lock(keyframes_mutex_);
    keyframes_copy = keyframes_;
  }

  if (keyframes_copy.empty()) {
    appendLoopLog("[ Loop ] No keyframes, skip corrected map save.");
    std::cout << "[ Loop ] No keyframes, skip corrected map save." << std::endl;
    return false;
  }

  PointCloudXYZI::Ptr merged(new PointCloudXYZI());
  for (const auto& kf : keyframes_copy) {
    if (kf == nullptr || kf->GetCloud() == nullptr || kf->GetCloud()->empty()) {
      continue;
    }

    PointCloudXYZI::Ptr body_cloud(new PointCloudXYZI());
    body_cloud->reserve(kf->GetCloud()->size());
    for (const auto& pt : kf->GetCloud()->points) {
      PointType out_pt;
      out_pt.x = pt.x;
      out_pt.y = pt.y;
      out_pt.z = pt.z;
      out_pt.intensity = pt.intensity;
      out_pt.normal_x = 0;
      out_pt.normal_y = 0;
      out_pt.normal_z = 0;
      out_pt.curvature = 0;
      body_cloud->push_back(out_pt);
    }

    const lightning::SE3 opt_pose = kf->GetOptPose();
    PointCloudXYZI::Ptr world_cloud(new PointCloudXYZI());
    pcl::transformPointCloud(*body_cloud, *world_cloud, opt_pose.matrix());
    *merged += *world_cloud;
  }

  if (merged->empty()) {
    appendLoopLog("[ Loop ] Corrected map is empty, skip save.");
    std::cout << "[ Loop ] Corrected map is empty, skip save." << std::endl;
    return false;
  }

  std::string path = options_.corrected_pcd_path;
  if (path.empty()) {
    path = "Log/PCD/loop_corrected_points.pcd";
  }
  path = resolveFastLivoLogPath(path);

  pcl::PCDWriter writer;
  writer.writeBinary(path, *merged);
  const std::string msg = "[ Loop ] Corrected map saved to: " + path + " (keyframes=" +
                          std::to_string(keyframes_copy.size()) + ", points=" + std::to_string(merged->size()) +
                          ", loops=" + std::to_string(loop_count_) + ")";
  std::cout << "\033[1;32m" << msg << "\033[0m" << std::endl;
  appendLoopLog(msg);
  return true;
}
