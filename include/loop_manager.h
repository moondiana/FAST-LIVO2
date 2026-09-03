#ifndef LOOP_MANAGER_H
#define LOOP_MANAGER_H

#include <utils/types.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct StatesGroup;

namespace lightning {
class Keyframe;
class LoopClosing;
}  // namespace lightning

class LoopManager {
 public:
  struct Options {
    bool enable = false;
    bool online_mode = true;
    double kf_dis_th = 2.0;
    double kf_angle_deg = 15.0;
    int loop_kf_gap = 20;
    int min_id_interval = 20;
    int closest_id_th = 50;
    double max_range = 20.0;
    double ndt_score_th = 1.3;
    bool with_height = false;
    bool save_corrected_pcd = true;
    std::string corrected_pcd_path = "Log/PCD/loop_corrected_points.pcd";
  };

  LoopManager() = default;
  ~LoopManager();

  void init(const Options& options);
  bool enabled() const { return options_.enable && loop_closing_ != nullptr; }

  void tryAddKeyframe(const StatesGroup& state, const PointCloudXYZI::Ptr& cloud_body, double timestamp);
  void finish();
  bool saveCorrectedPointCloud(const std::string& root_dir) const;

  size_t keyframeCount() const;
  size_t loopCount() const { return loop_count_; }

 private:
  Options options_;
  std::shared_ptr<lightning::LoopClosing> loop_closing_;
  std::vector<std::shared_ptr<lightning::Keyframe>> keyframes_;
  mutable std::mutex keyframes_mutex_;
  size_t loop_count_ = 0;
};

#endif  // LOOP_MANAGER_H
