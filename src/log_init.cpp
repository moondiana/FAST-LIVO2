#include "log_init.h"

#include <glog/logging.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace {
std::mutex g_loop_log_mutex;
std::string g_log_root;
std::string g_log_base;

std::string currentTimeTag() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm_buf{};
  localtime_r(&t, &tm_buf);
  std::ostringstream oss;
  oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

std::string sessionFolderName() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm_buf{};
  localtime_r(&t, &tm_buf);
  std::ostringstream oss;
  oss << std::put_time(&tm_buf, "%Y-%m-%d_%H-%M-%S");
  return oss.str();
}

bool isAbsolutePath(const std::string& path) {
  if (path.empty()) return false;
  if (path[0] == '/') return true;
  return path.size() > 1 && path[1] == ':';
}

std::string stripLogPrefix(std::string path) {
  if (path.rfind("Log/", 0) == 0) {
    return path.substr(4);
  }
  if (path.rfind("Log\\", 0) == 0) {
    return path.substr(4);
  }
  return path;
}

void createSessionSubdirs(const std::string& session_dir) {
  std::filesystem::create_directories(session_dir + "/glog");
  std::filesystem::create_directories(session_dir + "/PCD");
  std::filesystem::create_directories(session_dir + "/PCD1");
  std::filesystem::create_directories(session_dir + "/result");
  std::filesystem::create_directories(session_dir + "/Colmap/sparse/0");
  std::filesystem::create_directories(session_dir + "/Colmap/images");
  std::filesystem::create_directories(session_dir + "/ref_cur_combine");
}

void writeSessionInfo(const std::string& session_dir) {
  std::ofstream fout(session_dir + "/session_info.txt", std::ios::out | std::ios::trunc);
  if (!fout.is_open()) return;
  fout << "start_time: " << currentTimeTag() << "\n";
  fout << "session_dir: " << session_dir << "\n";
  fout << "glog_dir: " << session_dir << "/glog\n";
  fout << "loop_log: " << session_dir << "/loop_closing.log\n";
  fout << "imu_log: " << session_dir << "/imu.txt\n";
  fout << "lio_log: " << session_dir << "/mat_pre.txt , " << session_dir << "/mat_out.txt\n";
}
}  // namespace

void initFastLivoLogging(const char* argv0) {
  g_log_base = std::string(ROOT_DIR) + "Log";
  g_log_root = g_log_base + "/" + sessionFolderName();

  std::filesystem::create_directories(g_log_base);
  createSessionSubdirs(g_log_root);
  writeSessionInfo(g_log_root);

  google::InitGoogleLogging(argv0);
  google::InstallFailureSignalHandler();

  FLAGS_log_dir = g_log_root + "/glog";
  FLAGS_alsologtostderr = true;
  FLAGS_colorlogtostderr = true;
  FLAGS_logtostderr = false;
  FLAGS_stderrthreshold = google::INFO;
  FLAGS_max_log_size = 100;

  appendLoopLog("===== FAST-LIVO2 session start =====");
  appendLoopLog("session dir: " + g_log_root);

  std::cout << "\033[1;32m[ Log ] Session logs -> " << g_log_root << "\033[0m" << std::endl;
  std::cout << "[ Log ] glog      -> glog/" << std::endl;
  std::cout << "[ Log ] loop      -> loop_closing.log" << std::endl;
  std::cout << "[ Log ] imu/lio   -> imu.txt, mat_pre.txt, mat_out.txt" << std::endl;
}

void shutdownFastLivoLogging() { google::ShutdownGoogleLogging(); }

std::string getFastLivoLogDir() {
  return g_log_root.empty() ? g_log_base.empty() ? std::string(ROOT_DIR) + "Log" : g_log_base : g_log_root;
}

std::string getFastLivoLogBaseDir() {
  return g_log_base.empty() ? std::string(ROOT_DIR) + "Log" : g_log_base;
}

std::string resolveFastLivoLogPath(const std::string& path) {
  if (path.empty()) return getFastLivoLogDir();
  if (isAbsolutePath(path)) return path;
  const std::string rel = stripLogPrefix(path);
  return getFastLivoLogDir() + "/" + rel;
}

std::string resolveRootRelativePath(const std::string& path) {
  if (path.empty()) return std::string(ROOT_DIR);
  if (isAbsolutePath(path)) return path;
  return std::string(ROOT_DIR) + path;
}

void appendLoopLog(const std::string& message) {
  const std::string log_path = getFastLivoLogDir() + "/loop_closing.log";

  std::lock_guard<std::mutex> lock(g_loop_log_mutex);
  std::ofstream fout(log_path, std::ios::app);
  if (fout.is_open()) {
    fout << "[" << currentTimeTag() << "] " << message << std::endl;
  }
}
