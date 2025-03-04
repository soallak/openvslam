#ifndef OPENVSLAM_OPTIMIZER_G2O_SE3_EQUIRECTANGULAR_POSE_OPT_EDGE_H
#define OPENVSLAM_OPTIMIZER_G2O_SE3_EQUIRECTANGULAR_POSE_OPT_EDGE_H

#include <g2o/core/base_unary_edge.h>

#include "openvslam/optimize/internal/landmark_vertex.h"
#include "openvslam/optimize/internal/se3/shot_vertex.h"
#include "openvslam/type.h"

namespace openvslam {
namespace optimize {
namespace internal {
namespace se3 {

class equirectangular_pose_opt_edge final
    : public g2o::BaseUnaryEdge<2, Vec2_t, shot_vertex> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  equirectangular_pose_opt_edge();

  bool read(std::istream& is) override;

  bool write(std::ostream& os) const override;

  void computeError() override;

  void linearizeOplus() override;

  Vec2_t cam_project(const Vec3_t& pos_c) const;

  Vec3_t pos_w_;
  double cols_, rows_;
};

inline equirectangular_pose_opt_edge::equirectangular_pose_opt_edge()
    : g2o::BaseUnaryEdge<2, Vec2_t, shot_vertex>() {}

inline bool equirectangular_pose_opt_edge::read(std::istream& is) {
  for (unsigned int i = 0; i < 2; ++i) {
    is >> _measurement(i);
  }
  for (unsigned int i = 0; i < 2; ++i) {
    for (unsigned int j = i; j < 2; ++j) {
      is >> information()(i, j);
      if (i != j) {
        information()(j, i) = information()(i, j);
      }
    }
  }
  return true;
}

inline bool equirectangular_pose_opt_edge::write(std::ostream& os) const {
  for (unsigned int i = 0; i < 2; ++i) {
    os << measurement()(i) << " ";
  }
  for (unsigned int i = 0; i < 2; ++i) {
    for (unsigned int j = i; j < 2; ++j) {
      os << " " << information()(i, j);
    }
  }
  return os.good();
}

inline void equirectangular_pose_opt_edge::computeError() {
  const auto v1 = static_cast<const shot_vertex*>(_vertices.at(0));
  const Vec2_t obs(_measurement);
  _error = obs - cam_project(v1->estimate().map(pos_w_));
}

inline void equirectangular_pose_opt_edge::linearizeOplus() {
  auto vi = static_cast<shot_vertex*>(_vertices.at(0));
  const g2o::SE3Quat& cam_pose_cw = vi->shot_vertex::estimate();
  const Vec3_t pos_c = cam_pose_cw.map(pos_w_);

  const auto pcx = pos_c(0);
  const auto pcy = pos_c(1);
  const auto pcz = pos_c(2);
  const auto L = pos_c.norm();

  // 回転に対する微分
  const Vec3_t d_pc_d_rx(0, -pcz, pcy);
  const Vec3_t d_pc_d_ry(pcz, 0, -pcx);
  const Vec3_t d_pc_d_rz(-pcy, pcx, 0);
  // 並進に対する微分
  const Vec3_t d_pc_d_tx(1, 0, 0);
  const Vec3_t d_pc_d_ty(0, 1, 0);
  const Vec3_t d_pc_d_tz(0, 0, 1);

  // 状態ベクトルを x = [rx, ry, rz, tx, ty, tz] として，
  // 導関数ベクトル d_pcx_d_x, d_pcy_d_x, d_pcz_d_x を作成
  VecR_t<6> d_pcx_d_x;
  d_pcx_d_x << d_pc_d_rx(0), d_pc_d_ry(0), d_pc_d_rz(0), d_pc_d_tx(0),
      d_pc_d_ty(0), d_pc_d_tz(0);
  VecR_t<6> d_pcy_d_x;
  d_pcy_d_x << d_pc_d_rx(1), d_pc_d_ry(1), d_pc_d_rz(1), d_pc_d_tx(1),
      d_pc_d_ty(1), d_pc_d_tz(1);
  VecR_t<6> d_pcz_d_x;
  d_pcz_d_x << d_pc_d_rx(2), d_pc_d_ry(2), d_pc_d_rz(2), d_pc_d_tx(2),
      d_pc_d_ty(2), d_pc_d_tz(2);

  // 導関数ベクトル d_L_d_x を作成
  const Vec6_t d_L_d_x =
      (1.0 / L) * (pcx * d_pcx_d_x + pcy * d_pcy_d_x + pcz * d_pcz_d_x);

  // ヤコビ行列を作成
  MatRC_t<2, 6> jacobian = MatRC_t<2, 6>::Zero();
  jacobian.block<1, 6>(0, 0) = -(cols_ / (2 * M_PI)) *
                               (1.0 / (pcx * pcx + pcz * pcz)) *
                               (pcz * d_pcx_d_x - pcx * d_pcz_d_x);
  jacobian.block<1, 6>(1, 0) = -(rows_ / M_PI) *
                               (1.0 / (L * std::sqrt(pcx * pcx + pcz * pcz))) *
                               (L * d_pcy_d_x - pcy * d_L_d_x);

  // g2oの変数にセット
  // 姿勢に対する微分
  _jacobianOplusXi = jacobian;
}

inline Vec2_t equirectangular_pose_opt_edge::cam_project(
    const Vec3_t& pos_c) const {
  const double theta = std::atan2(pos_c(0), pos_c(2));
  const double phi = -std::asin(pos_c(1) / pos_c.norm());
  return {cols_ * (0.5 + theta / (2 * M_PI)), rows_ * (0.5 - phi / M_PI)};
}

}  // namespace se3
}  // namespace internal
}  // namespace optimize
}  // namespace openvslam

#endif  // OPENVSLAM_EQUIRECTANGULAR_POSE_OPT_EDGE_H
