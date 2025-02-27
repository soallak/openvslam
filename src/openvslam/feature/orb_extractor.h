#ifndef OPENVSLAM_FEATURE_ORB_EXTRACTOR_H
#define OPENVSLAM_FEATURE_ORB_EXTRACTOR_H

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

#include "openvslam/feature/orb_extractor_node.h"
#include "openvslam/feature/orb_params.h"

namespace openvslam {
namespace feature {

class orb_extractor {
 public:
  orb_extractor() = delete;

  //! Constructor
  orb_extractor(const orb_params* orb_params, const unsigned int max_num_keypts,
                const std::vector<std::vector<float>>& mask_rects = {});

  //! Destructor
  virtual ~orb_extractor() = default;

  //! Extract keypoints and each descriptor of them
  void extract(const cv::_InputArray& in_image,
               const cv::_InputArray& in_image_mask,
               std::vector<cv::KeyPoint>& keypts,
               const cv::_OutputArray& out_descriptors);

  //! Get the maximum number of keypoints
  unsigned int get_max_num_keypoints() const;

  //! Set the maximum number of keypoints
  void set_max_num_keypoints(const unsigned int max_num_keypts);

  //! parameters for ORB extraction
  const orb_params* orb_params_;

  //! A vector of keypoint area represents mask area
  //! Each areas are denoted as form of [x_min / cols, x_max / cols, y_min /
  //! rows, y_max / rows]
  std::vector<std::vector<float>> mask_rects_;

  //! Image pyramid
  std::vector<cv::Mat> image_pyramid_;

 private:
  //! Initialize orb extractor
  void initialize();

  //! Calculate scale factors and sigmas
  void calc_scale_factors();

  //! Create a mask matrix that constructed by rectangles
  void create_rectangle_mask(const unsigned int cols, const unsigned int rows);

  //! Compute image pyramid
  void compute_image_pyramid(const cv::Mat& image);

  //! Compute fast keypoints for cells in each image pyramid
  void compute_fast_keypoints(
      std::vector<std::vector<cv::KeyPoint>>& all_keypts,
      const cv::Mat& mask) const;

  //! Pick computed keypoints on the image uniformly
  std::vector<cv::KeyPoint> distribute_keypoints_via_tree(
      const std::vector<cv::KeyPoint>& keypts_to_distribute, const int min_x,
      const int max_x, const int min_y, const int max_y,
      const unsigned int num_keypts) const;

  //! Initialize nodes that used for keypoint distribution tree
  std::list<orb_extractor_node> initialize_nodes(
      const std::vector<cv::KeyPoint>& keypts_to_distribute, const int min_x,
      const int max_x, const int min_y, const int max_y) const;

  //! Assign child nodes to the all node list
  void assign_child_nodes(
      const std::array<orb_extractor_node, 4>& child_nodes,
      std::list<orb_extractor_node>& nodes,
      std::vector<std::pair<int, orb_extractor_node*>>& leaf_nodes) const;

  //! Find keypoint which has maximum value of response
  std::vector<cv::KeyPoint> find_keypoints_with_max_response(
      std::list<orb_extractor_node>& nodes) const;

  //! Compute orientation for each keypoint
  void compute_orientation(const cv::Mat& image,
                           std::vector<cv::KeyPoint>& keypts) const;

  //! Correct keypoint's position to comply with the scale
  void correct_keypoint_scale(std::vector<cv::KeyPoint>& keypts_at_level,
                              const unsigned int level) const;

  //! Compute the gradient direction of pixel intensity in a circle around the
  //! point
  float ic_angle(const cv::Mat& image, const cv::Point2f& point) const;

  //! Compute orb descriptors for all keypoint
  void compute_orb_descriptors(const cv::Mat& image,
                               const std::vector<cv::KeyPoint>& keypts,
                               cv::Mat& descriptors) const;

  //! Compute orb descriptor of a keypoint
  void compute_orb_descriptor(const cv::KeyPoint& keypt, const cv::Mat& image,
                              uchar* desc) const;

  //! Number of feature points to be extracted
  unsigned int max_num_keypts_;

  //! BRIEF orientation
  static constexpr unsigned int fast_patch_size_ = 31;
  //! half size of FAST patch
  static constexpr int fast_half_patch_size_ = fast_patch_size_ / 2;

  //! size of maximum ORB patch radius
  static constexpr unsigned int orb_patch_radius_ = 19;

  //! rectangle mask has been already initialized or not
  bool mask_is_initialized_ = false;
  cv::Mat rect_mask_;

  //! Maximum number of keypoint of each level
  std::vector<unsigned int> num_keypts_per_level_;
  //! Index limitation that used for calculating of keypoint orientation
  std::vector<int> u_max_;
};

}  // namespace feature
}  // namespace openvslam

#endif  // OPENVSLAM_FEATURE_ORB_EXTRACTOR_H
