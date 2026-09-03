#include "LIVMapper.h"
#include "log_init.h"

int main(int argc, char **argv)
{
  initFastLivoLogging(argv[0]);

  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);

  rclcpp::Node::SharedPtr nh;
  image_transport::ImageTransport it_(nh);
  LIVMapper mapper(nh, "laserMapping", options);
  mapper.initializeSubscribersAndPublishers(nh, it_);
  mapper.run(nh);

  appendLoopLog("===== FAST-LIVO2 session end =====");
  shutdownFastLivoLogging();
  rclcpp::shutdown();
  return 0;
}
