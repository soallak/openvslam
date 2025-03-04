#ifndef OPENVSLAM_DATA_MAP_DATABASE_H
#define OPENVSLAM_DATA_MAP_DATABASE_H

#include <memory>
#include <mutex>
#include <nlohmann/json_fwd.hpp>
#include <unordered_map>
#include <vector>

#include "openvslam/data/bow_vocabulary_fwd.h"
#include "openvslam/data/frame_statistics.h"

namespace openvslam {

namespace camera {
class base;
}  // namespace camera

namespace data {

class frame;
class keyframe;
class landmark;
class camera_database;
class orb_params_database;
class bow_database;

class map_database {
 public:
  /**
   * Constructor
   */
  map_database();

  /**
   * Destructor
   */
  ~map_database();

  /**
   * Add keyframe to the database
   * @param keyfrm
   */
  void add_keyframe(const std::shared_ptr<keyframe>& keyfrm);

  /**
   * Erase keyframe from the database
   * @param keyfrm
   */
  void erase_keyframe(const std::shared_ptr<keyframe>& keyfrm);

  /**
   * Add landmark to the database
   * @param lm
   */
  void add_landmark(std::shared_ptr<landmark>& lm);

  /**
   * Erase landmark from the database
   * @param lm
   */
  void erase_landmark(unsigned int id);

  /**
   * Set local landmarks
   * @param local_lms
   */
  void set_local_landmarks(
      const std::vector<std::shared_ptr<landmark>>& local_lms);

  /**
   * Get local landmarks
   * @return
   */
  std::vector<std::shared_ptr<landmark>> get_local_landmarks() const;

  /**
   * Get all of the keyframes in the database
   * @return
   */
  std::vector<std::shared_ptr<keyframe>> get_all_keyframes() const;

  /**
   * Get closest keyframes to a given 2d pose
   * @param pose Given 2d pose
   * @param normal_vector normal vector of plane
   * @param distance_threshold Maximum distance where close keyframes could be
   * found
   * @param angle_threshold Maximum angle between given pose and close keyframes
   * @return Vector closest keyframes
   */
  std::vector<std::shared_ptr<keyframe>> get_close_keyframes_2d(
      const Mat44_t& pose, const Vec3_t& normal_vector,
      const double distance_threshold, const double angle_threshold) const;

  /**
   * Get closest keyframes to a given pose
   * @param pose Given pose
   * @param distance_threshold Maximum distance where close keyframes could be
   * found
   * @param angle_threshold Maximum angle between given pose and close keyframes
   * @return Vector closest keyframes
   */
  std::vector<std::shared_ptr<keyframe>> get_close_keyframes(
      const Mat44_t& pose, const double distance_threshold,
      const double angle_threshold) const;

  /**
   * Get the number of keyframes
   * @return
   */
  unsigned get_num_keyframes() const;

  /**
   * Get all of the landmarks in the database
   * @return
   */
  std::vector<std::shared_ptr<landmark>> get_all_landmarks() const;

  /**
   * Get the last keyframe added to the database
   * @return shared pointer to the last keyframe added to the database
   */
  std::shared_ptr<keyframe> get_last_inserted_keyframe() const;

  /**
   * Get the number of landmarks
   * @return
   */
  unsigned int get_num_landmarks() const;

  /**
   * Update frame statistics
   * @param frm
   * @param is_lost
   */
  void update_frame_statistics(const data::frame& frm, const bool is_lost) {
    std::lock_guard<std::mutex> lock(mtx_map_access_);
    frm_stats_.update_frame_statistics(frm, is_lost);
  }

  /**
   * Replace a keyframe which will be erased in frame statistics
   * @param old_keyfrm
   * @param new_keyfrm
   */
  void replace_reference_keyframe(
      const std::shared_ptr<data::keyframe>& old_keyfrm,
      const std::shared_ptr<data::keyframe>& new_keyfrm) {
    std::lock_guard<std::mutex> lock(mtx_map_access_);
    frm_stats_.replace_reference_keyframe(old_keyfrm, new_keyfrm);
  }

  /**
   * Get frame statistics
   * @return
   */
  frame_statistics get_frame_statistics() const {
    std::lock_guard<std::mutex> lock(mtx_map_access_);
    return frm_stats_;
  }

  /**
   * Clear the database
   */
  void clear();

  /**
   * Load keyframes and landmarks from JSON
   * @param cam_db
   * @param orb_params_db
   * @param bow_vocab
   * @param json_keyfrms
   * @param json_landmarks
   */
  void from_json(camera_database* cam_db, orb_params_database* orb_params_db,
                 bow_vocabulary* bow_vocab, const nlohmann::json& json_keyfrms,
                 const nlohmann::json& json_landmarks);

  /**
   * Dump keyframes and landmarks as JSON
   * @param json_keyfrms
   * @param json_landmarks
   */
  void to_json(nlohmann::json& json_keyfrms, nlohmann::json& json_landmarks);

  //! origin keyframe
  std::shared_ptr<keyframe> origin_keyfrm_ = nullptr;

  //! mutex for locking ALL access to the database
  //! (NOTE: cannot used in map_database class)
  static std::mutex mtx_database_;

 private:
  /**
   * Decode JSON and register keyframe information to the map database
   * (NOTE: objects which are not constructed yet will be set as nullptr)
   * @param cam_db
   * @param orb_params_db
   * @param bow_vocab
   * @param id
   * @param json_keyfrm
   */
  void register_keyframe(camera_database* cam_db,
                         orb_params_database* orb_params_db,
                         bow_vocabulary* bow_vocab, const unsigned int id,
                         const nlohmann::json& json_keyfrm);

  /**
   * Decode JSON and register landmark information to the map database
   * (NOTE: objects which are not constructed yet will be set as nullptr)
   * @param id
   * @param json_landmark
   */
  void register_landmark(const unsigned int id,
                         const nlohmann::json& json_landmark);

  /**
   * Decode JSON and register essential graph information
   * (NOTE: keyframe database must be completely constructed before calling this
   * function)
   * @param id
   * @param json_keyfrm
   */
  void register_graph(const unsigned int id, const nlohmann::json& json_keyfrm);

  /**
   * Decode JSON and register keyframe-landmark associations
   * (NOTE: keyframe and landmark database must be completely constructed before
   * calling this function)
   * @param keyfrm_id
   * @param json_keyfrm
   */
  void register_association(const unsigned int keyfrm_id,
                            const nlohmann::json& json_keyfrm);

  //! mutex for mutual exclusion controll between class methods
  mutable std::mutex mtx_map_access_;

  //-----------------------------------------
  // keyframe and landmark database

  //! IDs and keyframes
  std::unordered_map<unsigned int, std::shared_ptr<keyframe>> keyframes_;
  //! IDs and landmarks
  std::unordered_map<unsigned int, std::shared_ptr<landmark>> landmarks_;

  //! The last keyframe added to the database
  std::shared_ptr<keyframe> last_inserted_keyfrm_ = nullptr;

  //! local landmarks
  std::vector<std::shared_ptr<landmark>> local_landmarks_;

  //-----------------------------------------
  // frame statistics for odometry evaluation

  //! frame statistics
  frame_statistics frm_stats_;
};

}  // namespace data
}  // namespace openvslam

#endif  // OPENVSLAM_DATA_MAP_DATABASE_H
