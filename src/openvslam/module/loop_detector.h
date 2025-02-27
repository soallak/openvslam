#ifndef OPENVSLAM_MODULE_LOOP_DETECTOR_H
#define OPENVSLAM_MODULE_LOOP_DETECTOR_H

#include <atomic>
#include <memory>

#include "openvslam/data/bow_vocabulary.h"
#include "openvslam/module/type.h"
#include "openvslam/optimize/transform_optimizer.h"

namespace openvslam {

namespace data {
class keyframe;
class bow_database;
}  // namespace data

namespace module {

class loop_detector {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /**
   * Constructor
   */
  loop_detector(data::bow_database* bow_db, data::bow_vocabulary* bow_vocab,
                const YAML::Node& yaml_node,
                const bool fix_scale_in_Sim3_estimation);

  /**
   * Enable loop detection
   */
  void enable_loop_detector();

  /**
   * Disable loop detection
   */
  void disable_loop_detector();

  /**
   * Get the loop detector status
   */
  bool is_enabled() const;

  /**
   * Set the current keyframe
   */
  void set_current_keyframe(const std::shared_ptr<data::keyframe>& keyfrm);

  /**
   * Detect loop candidates using BoW vocabulary
   */
  bool detect_loop_candidates();

  /**
   * Validate loop candidates selected in detect_loop_candidate()
   */
  bool validate_candidates();

  /**
   * Get the selected candidate keyframe after loop detection and validation
   */
  std::shared_ptr<data::keyframe> get_selected_candidate_keyframe() const;

  /**
   * Get the estimated Sim3 from the world the the current
   */
  g2o::Sim3 get_Sim3_world_to_current() const;

  /**
   * Get the matches between the keypoint indices of the current keyframe and
   * the landmarks observed in the candidate
   */
  std::vector<std::shared_ptr<data::landmark>>
  current_matched_landmarks_observed_in_candidate() const;

  /**
   * Get the matches between the keypoint indices of the current keyframe and
   * the landmarks observed in covisibilities of the candidate
   */
  std::vector<std::shared_ptr<data::landmark>>
  current_matched_landmarks_observed_in_candidate_covisibilities() const;

  /**
   * Set the keyframe ID when loop correction is performed
   */
  void set_loop_correct_keyframe_id(const unsigned int loop_correct_keyfrm_id);

 private:
  /**
   * called by detect_loop_candidates
   */
  bool detect_loop_candidates_impl();

  /**
   * called by validate_candidates
   */
  bool validate_candidates_impl();

  /**
   * Compute the minimum score among covisibilities
   */
  float compute_min_score_in_covisibilities(
      const std::shared_ptr<data::keyframe>& keyfrm) const;

  /**
   * Find continuously detected keyframe sets
   */
  keyframe_sets find_continuously_detected_keyframe_sets(
      const keyframe_sets& prev_cont_detected_keyfrm_sets,
      const std::vector<std::shared_ptr<data::keyframe>>& keyfrms_to_search)
      const;

  /**
   * Select ONE candidate from the candidates via linear and nonlinear Sim3
   * validation
   */
  bool select_loop_candidate_via_Sim3(
      const std::vector<std::shared_ptr<data::keyframe>>& loop_candidates,
      std::shared_ptr<data::keyframe>& selected_candidate,
      g2o::Sim3& g2o_Sim3_world_to_curr,
      std::vector<std::shared_ptr<data::landmark>>&
          curr_match_lms_observed_in_cand) const;

  //! BoW database
  data::bow_database* bow_db_;
  //! BoW vocabulary
  data::bow_vocabulary* bow_vocab_;

  //! transform optimizer
  const optimize::transform_optimizer transform_optimizer_;

  //! flag which indicates the loop detector is enabled or not
  std::atomic<bool> loop_detector_is_enabled_{true};

  //! for stereo/RGBD models, fix scale when estimating Sim3
  const bool fix_scale_in_Sim3_estimation_;

  //! the threshold of the number of mutual matches after the Sim3 estimation
  const unsigned int num_final_matches_thr_;

  //! the threshold of the continuity of continuously detected keyframe set
  const unsigned int min_continuity_;

  //-----------------------------------------
  // variables for loop detection and correction

  //! current keyframe
  std::shared_ptr<data::keyframe> cur_keyfrm_;
  //! final loop candidate
  std::shared_ptr<data::keyframe> selected_candidate_ = nullptr;

  //! previously detected keyframe sets as loop candidate
  keyframe_sets cont_detected_keyfrm_sets_;
  //! loop candidate for validation
  std::vector<std::shared_ptr<data::keyframe>> loop_candidates_to_validate_;

  //! matches between the keypoint indices of the current keyframe and the
  //! landmarks observed in the candidate
  std::vector<std::shared_ptr<data::landmark>> curr_match_lms_observed_in_cand_;
  //! matches between the keypoint indices of the current keyframe and the
  //! landmarks observed in covisibilities of the candidate
  std::vector<std::shared_ptr<data::landmark>>
      curr_match_lms_observed_in_cand_covis_;

  //! the Sim3 camera pose of the current keyframe AFTER loop correction (in
  //! Mat44_t format)
  Mat44_t Sim3_world_to_curr_;
  //! the Sim3 camera pose of the current keyframe AFTER loop correction (in
  //! g2o::Sim3 format)
  g2o::Sim3 g2o_Sim3_world_to_curr_;

  //! the keyframe ID when the previouls loop correction was performed
  unsigned int prev_loop_correct_keyfrm_id_ = 0;
};

}  // namespace module
}  // namespace openvslam

#endif  // OPENVSLAM_MODULE_LOOP_DETECTOR_H
