#ifndef OPENVSLAM_MODULE_LOCAL_MAP_UPDATER_H
#define OPENVSLAM_MODULE_LOCAL_MAP_UPDATER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace openvslam {

namespace data {
class frame;
class keyframe;
class landmark;
}  // namespace data

namespace module {

class local_map_updater {
 public:
  using keyframe_weights_t =
      std::unordered_map<std::shared_ptr<data::keyframe>, unsigned int>;

  //! Constructor
  explicit local_map_updater(const data::frame& curr_frm,
                             const unsigned int max_num_local_keyfrms);

  //! Destructor
  ~local_map_updater() = default;

  //! Get the local keyframes
  std::vector<std::shared_ptr<data::keyframe>> get_local_keyframes() const;

  //! Get the local landmarks
  std::vector<std::shared_ptr<data::landmark>> get_local_landmarks() const;

  //! Get the nearest covisibility
  std::shared_ptr<data::keyframe> get_nearest_covisibility() const;

  //! Acquire the new local map
  bool acquire_local_map(std::unordered_set<unsigned int>& outlier_ids);

 private:
  //! Find the local keyframes
  bool find_local_keyframes();

  //! Compute keyframe weights
  keyframe_weights_t count_keyframe_weights() const;

  //! Find the first-order local keyframes
  auto find_first_local_keyframes(
      const keyframe_weights_t& keyfrm_weights,
      std::unordered_set<unsigned int>& already_found_ids)
      -> std::vector<std::shared_ptr<data::keyframe>>;

  //! Find the second-order local keyframes
  auto find_second_local_keyframes(
      const std::vector<std::shared_ptr<data::keyframe>>& first_local_keyframes,
      std::unordered_set<unsigned int>& already_found_ids) const
      -> std::vector<std::shared_ptr<data::keyframe>>;

  //! Find the local landmarks
  bool find_local_landmarks(std::unordered_set<unsigned int>& outlier_ids);

  // frame ID
  const unsigned int frm_id_;
  // landmark associations
  const std::vector<std::shared_ptr<data::landmark>> frm_lms_;
  // the number of keypoints
  const unsigned int num_keypts_;
  // maximum number of the local keyframes
  const unsigned int max_num_local_keyfrms_;

  // found local keyframes
  std::vector<std::shared_ptr<data::keyframe>> local_keyfrms_;
  // found local landmarks
  std::vector<std::shared_ptr<data::landmark>> local_lms_;
  // the nearst keyframe in covisibility graph, which will be found in
  // find_first_local_keyframes()
  std::shared_ptr<data::keyframe> nearest_covisibility_;
};

}  // namespace module
}  // namespace openvslam

#endif  // OPENVSLAM_MODULE_LOCAL_MAP_UPDATER_H
