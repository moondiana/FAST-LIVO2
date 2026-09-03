//
// 为自定义点类型 PointXYZIT 显式实例化 PCL NDT 相关模板。
// PCL 预编译库不含自定义点类型，必须在 looper 内完成实例化（与 loop/src 同库编译效果一致）。
//

#include "common/point_def.h"

#include <pcl/pcl_base.h>
#include <pcl/common/common.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/voxel_grid_covariance.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/registration/ndt.h>
#include <pcl/search/kdtree.h>

#include <pcl/impl/pcl_base.hpp>
#include <pcl/filters/impl/voxel_grid_covariance.hpp>
#include <pcl/filters/impl/voxel_grid.hpp>
#include <pcl/kdtree/impl/kdtree_flann.hpp>
#include <pcl/registration/impl/ndt.hpp>
#include <pcl/search/impl/kdtree.hpp>
#include <pcl/search/impl/search.hpp>

template class PCL_EXPORTS pcl::PCLBase<PointXYZIT>;
template class PCL_EXPORTS pcl::VoxelGrid<PointXYZIT>;
template class PCL_EXPORTS pcl::KdTreeFLANN<PointXYZIT, flann::L2_Simple<float>>;
template class PCL_EXPORTS pcl::search::KdTree<PointXYZIT, pcl::KdTreeFLANN<PointXYZIT, flann::L2_Simple<float>>>;
template class PCL_EXPORTS pcl::VoxelGridCovariance<PointXYZIT>;
template class PCL_EXPORTS pcl::NormalDistributionsTransform<PointXYZIT, PointXYZIT>;

template void PCL_EXPORTS pcl::getMinMax3D<PointXYZIT>(pcl::PointCloud<PointXYZIT>::ConstPtr const&,
                                                         const std::string&, float, float,
                                                         Eigen::Vector4f&, Eigen::Vector4f&, bool);
