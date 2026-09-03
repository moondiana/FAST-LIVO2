add_library(miao.core SHARED
        graph/graph.cc
        graph/optimizer.cc

        common/string_tools.cc

        math/marginal_covariance_cholesky.cc

        opti_algo/optimization_algorithm.cc
        opti_algo/opti_algo_gauss_newton.cc
        opti_algo/opti_algo_lm.cc
        opti_algo/opti_algo_dogleg.cc

        solver/solver.cc
)

target_include_directories(miao.core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${EIGEN3_INCLUDE_DIR}
        ${Sophus_INCLUDE_DIRS}
)

target_link_libraries(
        miao.core
        glog::glog
        TBB::tbb
        Eigen3::Eigen
        ${Sophus_LIBRARIES}
)