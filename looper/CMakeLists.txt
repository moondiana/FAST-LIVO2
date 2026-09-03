# Loop closure module extracted from lightning-lm for FAST-LIVO2 integration.
cmake_minimum_required(VERSION 3.10)
project(looper)

find_package(glog REQUIRED)
find_package(PCL REQUIRED COMPONENTS common filters registration search kdtree io)
find_package(yaml-cpp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Sophus REQUIRED)
find_package(TBB REQUIRED)

if(NOT Sophus_INCLUDE_DIRS AND TARGET Sophus::Sophus)
  get_target_property(Sophus_INCLUDE_DIRS Sophus::Sophus INTERFACE_INCLUDE_DIRECTORIES)
endif()
if(NOT Sophus_LIBRARIES AND TARGET Sophus::Sophus)
  set(Sophus_LIBRARIES Sophus::Sophus)
endif()

set(LOOPER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(
  ${LOOPER_SRC_DIR}
  ${LOOPER_SRC_DIR}/core/miao
)

add_subdirectory(${LOOPER_SRC_DIR}/core/miao)

add_library(looper SHARED
  ${LOOPER_SRC_DIR}/core/loop_closing/loop_closing.cc
  ${LOOPER_SRC_DIR}/core/loop_closing/pcl_ndt_pointxyzit_inst.cpp
  ${LOOPER_SRC_DIR}/common/nav_state.cc
  ${LOOPER_SRC_DIR}/utils/pointcloud_utils.cc
  ${LOOPER_SRC_DIR}/utils/timer.cc
  ${LOOPER_SRC_DIR}/io/yaml_io.cc
)

target_include_directories(looper PUBLIC
  ${LOOPER_SRC_DIR}
  ${LOOPER_SRC_DIR}/core/miao
  ${EIGEN3_INCLUDE_DIR}
  ${Sophus_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
)

target_link_libraries(looper PUBLIC
  miao.core
  miao.utils
  ${PCL_LIBRARIES}
  pcl_common
  pcl_filters
  pcl_registration
  pcl_search
  pcl_kdtree
  glog::glog
  yaml-cpp
  TBB::tbb
  Eigen3::Eigen
  ${Sophus_LIBRARIES}
)

target_compile_features(looper PUBLIC cxx_std_17)
