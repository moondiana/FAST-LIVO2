add_library(miao.utils SHARED
        sampler.cc
)

target_include_directories(miao.utils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${EIGEN3_INCLUDE_DIR}
        ${Sophus_INCLUDE_DIRS}
)

target_link_libraries(miao.utils
        glog::glog
        Eigen3::Eigen
        ${Sophus_LIBRARIES}
)