#ifndef OPENVSLAM_MODULE_RELOCALIZER_H
#define OPENVSLAM_MODULE_RELOCALIZER_H

#include <yaml-cpp/node/node.h>

#include <memory>

#include "openvslam/match/bow_tree.h"
#include "openvslam/match/projection.h"
#include "openvslam/match/robust.h"
#include "openvslam/optimize/pose_optimizer.h"
#include "openvslam/solve/pnp_solver.h"

namespace openvslam {

namespace data {
class frame;
class bow_database;
}  // namespace data

namespace module {

class relocalizer {
 public:
  //! Constructor
  explicit relocalizer(const double bow_match_lowe_ratio = 0.75,
                       const double proj_match_lowe_ratio = 0.9,
                       const double robust_match_lowe_ratio = 0.8,
                       const unsigned int min_num_bow_matches = 20,
                       const unsigned int min_num_valid_obs = 50);

  explicit relocalizer(const YAML::Node& yaml_node);

  //! Destructor
  virtual ~relocalizer();

  //! Relocalize the specified frame
  bool relocalize(data::bow_database* bow_db, data::frame& curr_frm);

  //! Relocalize the specified frame by given candidates list
  bool reloc_by_candidates(
      data::frame& curr_frm,
      const std::vector<std::shared_ptr<openvslam::data::keyframe>>&
          reloc_candidates,
      bool use_robust_matcher = false);

 private:
  //! Extract valid (non-deleted) landmarks from landmark vector
  std::vector<unsigned int> extract_valid_indices(
      const std::vector<std::shared_ptr<data::landmark>>& landmarks) const;

  //! Setup PnP solver with the specified 2D-3D matches
  std::unique_ptr<solve::pnp_solver> setup_pnp_solver(
      const std::vector<unsigned int>& valid_indices,
      const eigen_alloc_vector<Vec3_t>& bearings,
      const std::vector<cv::KeyPoint>& keypts,
      const std::vector<std::shared_ptr<data::landmark>>& matched_landmarks,
      const std::vector<float>& scale_factors) const;

  //! minimum threshold of the number of BoW matches
  const unsigned int min_num_bow_matches_;
  //! minimum threshold of the number of valid (= inlier after pose
  //! optimization) matches
  const unsigned int min_num_valid_obs_;

  //! BoW matcher
  const match::bow_tree bow_matcher_;
  //! projection matcher
  const match::projection proj_matcher_;
  //! robust matcher
  const match::robust robust_matcher_;
  //! pose optimizer
  const optimize::pose_optimizer pose_optimizer_;
};

}  // namespace module
}  // namespace openvslam

#endif  // OPENVSLAM_MODULE_RELOCALIZER_H
