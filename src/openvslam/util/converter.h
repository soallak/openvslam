#ifndef OPENVSLAM_UTIL_CONVERTER_H
#define OPENVSLAM_UTIL_CONVERTER_H

#include <Eigen/Dense>
#include <opencv2/core/core.hpp>

#include "openvslam/type.h"

namespace g2o {
class SE3Quat;
class Sim3;
}  // namespace g2o

namespace openvslam {
namespace util {

class converter {
 public:
  //! descriptor vector
  static std::vector<cv::Mat> to_desc_vec(const cv::Mat& desc);

  //! to SE3 of g2o
  static g2o::SE3Quat to_g2o_SE3(const Mat44_t& cam_pose);

  //! to Eigen::Mat/Vec
  static Mat44_t to_eigen_mat(const g2o::SE3Quat& g2o_SE3);
  static Mat44_t to_eigen_mat(const g2o::Sim3& g2o_Sim3);
  static Mat44_t to_eigen_cam_pose(const Mat33_t& rot, const Vec3_t& trans);

  //! inverse pose
  static Mat44_t inverse_pose(const Mat44_t& pose_cw);

  //! from/to angle axis
  static Vec3_t to_angle_axis(const Mat33_t& rot_mat);
  static Mat33_t to_rot_mat(const Vec3_t& angle_axis);

  //! to homogeneous coordinates
  template <typename T>
  static Vec3_t to_homogeneous(const cv::Point_<T>& pt) {
    return Vec3_t{pt.x, pt.y, 1.0};
  }

  //! to skew symmetric matrix
  static Mat33_t to_skew_symmetric_mat(const Vec3_t& vec);
};

}  // namespace util
}  // namespace openvslam

#endif  // OPENVSLAM_UTIL_CONVERTER_H
