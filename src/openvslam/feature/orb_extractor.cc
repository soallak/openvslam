/*******************************************************************************

                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Copyright (C) 2009, Willow Garage Inc., all rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "openvslam/feature/orb_extractor.h"

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

#include "openvslam/feature/orb_point_pairs.h"
#include "openvslam/type.h"
#include "openvslam/util/trigonometric.h"

#ifdef USE_SSE_ORB
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#endif  // USE_SSE_ORB

#include <iostream>

namespace openvslam {
namespace feature {

orb_extractor::orb_extractor(const orb_params* orb_params,
                             const unsigned int max_num_keypts,
                             const std::vector<std::vector<float>>& mask_rects)
    : orb_params_(orb_params),
      mask_rects_(mask_rects),
      max_num_keypts_(max_num_keypts) {
  // initialize parameters
  initialize();
}

void orb_extractor::extract(const cv::_InputArray& in_image,
                            const cv::_InputArray& in_image_mask,
                            std::vector<cv::KeyPoint>& keypts,
                            const cv::_OutputArray& out_descriptors) {
  if (in_image.empty()) {
    return;
  }

  // get cv::Mat of image
  const auto image = in_image.getMat();
  assert(image.type() == CV_8UC1);

  // build image pyramid
  compute_image_pyramid(image);

  // mask initialization
  if (!mask_is_initialized_ && !mask_rects_.empty()) {
    create_rectangle_mask(image.cols, image.rows);
    mask_is_initialized_ = true;
  }

  std::vector<std::vector<cv::KeyPoint>> all_keypts;

  // select mask to use
  if (!in_image_mask.empty()) {
    // Use image_mask if it is available
    const auto image_mask = in_image_mask.getMat();
    assert(image_mask.type() == CV_8UC1);
    compute_fast_keypoints(all_keypts, image_mask);
  } else if (!rect_mask_.empty()) {
    // Use rectangle mask if it is available and image_mask is not used
    assert(rect_mask_.type() == CV_8UC1);
    compute_fast_keypoints(all_keypts, rect_mask_);
  } else {
    // Do not use any mask if all masks are unavailable
    compute_fast_keypoints(all_keypts, cv::Mat());
  }

  cv::Mat descriptors;

  unsigned int num_keypts = 0;
  for (unsigned int level = 0; level < orb_params_->num_levels_; ++level) {
    num_keypts += all_keypts.at(level).size();
  }
  if (num_keypts == 0) {
    out_descriptors.release();
  } else {
    out_descriptors.create(num_keypts, 32, CV_8U);
    descriptors = out_descriptors.getMat();
  }

  keypts.clear();
  keypts.reserve(num_keypts);

  unsigned int offset = 0;
  for (unsigned int level = 0; level < orb_params_->num_levels_; ++level) {
    auto& keypts_at_level = all_keypts.at(level);
    const auto num_keypts_at_level = keypts_at_level.size();

    if (num_keypts_at_level == 0) {
      continue;
    }

    cv::Mat blurred_image = image_pyramid_.at(level).clone();
    cv::GaussianBlur(blurred_image, blurred_image, cv::Size(7, 7), 2, 2,
                     cv::BORDER_REFLECT_101);

    cv::Mat descriptors_at_level =
        descriptors.rowRange(offset, offset + num_keypts_at_level);
    compute_orb_descriptors(blurred_image, keypts_at_level,
                            descriptors_at_level);

    offset += num_keypts_at_level;

    correct_keypoint_scale(keypts_at_level, level);

    keypts.insert(keypts.end(), keypts_at_level.begin(), keypts_at_level.end());
  }
}

unsigned int orb_extractor::get_max_num_keypoints() const {
  return max_num_keypts_;
}

void orb_extractor::set_max_num_keypoints(const unsigned int max_num_keypts) {
  max_num_keypts_ = max_num_keypts;
  initialize();
}

void orb_extractor::initialize() {
  // resize buffers according to the number of levels
  image_pyramid_.resize(orb_params_->num_levels_);
  num_keypts_per_level_.resize(orb_params_->num_levels_);

  // compute the desired number of keypoints per scale
  double desired_num_keypts_per_scale =
      max_num_keypts_ * (1.0 - 1.0 / orb_params_->scale_factor_) /
      (1.0 - std::pow(1.0 / orb_params_->scale_factor_,
                      static_cast<double>(orb_params_->num_levels_)));
  unsigned int total_num_keypts = 0;
  for (unsigned int level = 0; level < orb_params_->num_levels_ - 1; ++level) {
    num_keypts_per_level_.at(level) = std::round(desired_num_keypts_per_scale);
    total_num_keypts += num_keypts_per_level_.at(level);
    desired_num_keypts_per_scale *= 1.0 / orb_params_->scale_factor_;
  }
  num_keypts_per_level_.at(orb_params_->num_levels_ - 1) = std::max(
      static_cast<int>(max_num_keypts_) - static_cast<int>(total_num_keypts),
      0);

  // Preparate  for computation of orientation
  u_max_.resize(fast_half_patch_size_ + 1);
  const unsigned int vmax =
      std::floor(fast_half_patch_size_ * std::sqrt(2.0) / 2 + 1);
  const unsigned int vmin =
      std::ceil(fast_half_patch_size_ * std::sqrt(2.0) / 2);
  for (unsigned int v = 0; v <= vmax; ++v) {
    u_max_.at(v) = std::round(
        std::sqrt(fast_half_patch_size_ * fast_half_patch_size_ - v * v));
  }
  for (unsigned int v = fast_half_patch_size_, v0 = 0; vmin <= v; --v) {
    while (u_max_.at(v0) == u_max_.at(v0 + 1)) {
      ++v0;
    }
    u_max_.at(v) = v0;
    ++v0;
  }
}

void orb_extractor::create_rectangle_mask(const unsigned int cols,
                                          const unsigned int rows) {
  if (rect_mask_.empty()) {
    rect_mask_ = cv::Mat(rows, cols, CV_8UC1, cv::Scalar(255));
  }
  // draw masks
  for (const auto& mask_rect : mask_rects_) {
    // draw black rectangle
    const unsigned int x_min = std::round(cols * mask_rect.at(0));
    const unsigned int x_max = std::round(cols * mask_rect.at(1));
    const unsigned int y_min = std::round(rows * mask_rect.at(2));
    const unsigned int y_max = std::round(rows * mask_rect.at(3));
    cv::rectangle(rect_mask_, cv::Point2i(x_min, y_min),
                  cv::Point2i(x_max, y_max), cv::Scalar(0), -1, cv::LINE_AA);
  }
}

void orb_extractor::compute_image_pyramid(const cv::Mat& image) {
  image_pyramid_.at(0) = image;
  for (unsigned int level = 1; level < orb_params_->num_levels_; ++level) {
    // determine the size of an image
    const double scale = orb_params_->scale_factors_.at(level);
    const cv::Size size(std::round(image.cols * 1.0 / scale),
                        std::round(image.rows * 1.0 / scale));
    // resize
    cv::resize(image_pyramid_.at(level - 1), image_pyramid_.at(level), size, 0,
               0, cv::INTER_LINEAR);
  }
}

void orb_extractor::compute_fast_keypoints(
    std::vector<std::vector<cv::KeyPoint>>& all_keypts,
    const cv::Mat& mask) const {
  all_keypts.resize(orb_params_->num_levels_);

  // An anonymous function which checks mask(image or rectangle)
  auto is_in_mask = [&mask](const unsigned int y, const unsigned int x,
                            const float scale_factor) {
    return mask.at<unsigned char>(y * scale_factor, x * scale_factor) == 0;
  };

  constexpr unsigned int overlap = 6;
  constexpr unsigned int cell_size = 64;

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
  for (unsigned int level = 0; level < orb_params_->num_levels_; ++level) {
    const float scale_factor = orb_params_->scale_factors_.at(level);

    constexpr unsigned int min_border_x = orb_patch_radius_;
    constexpr unsigned int min_border_y = orb_patch_radius_;
    const unsigned int max_border_x =
        image_pyramid_.at(level).cols - orb_patch_radius_;
    const unsigned int max_border_y =
        image_pyramid_.at(level).rows - orb_patch_radius_;

    const unsigned int width = max_border_x - min_border_x;
    const unsigned int height = max_border_y - min_border_y;

    const unsigned int num_cols = std::ceil(width / cell_size) + 1;
    const unsigned int num_rows = std::ceil(height / cell_size) + 1;

    std::vector<cv::KeyPoint> keypts_to_distribute;
    keypts_to_distribute.reserve(max_num_keypts_ * 10);

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
    for (unsigned int i = 0; i < num_rows; ++i) {
      const unsigned int min_y = min_border_y + i * cell_size;
      if (max_border_y - overlap <= min_y) {
        continue;
      }
      unsigned int max_y = min_y + cell_size + overlap;
      if (max_border_y < max_y) {
        max_y = max_border_y;
      }

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
      for (unsigned int j = 0; j < num_cols; ++j) {
        const unsigned int min_x = min_border_x + j * cell_size;
        if (max_border_x - overlap <= min_x) {
          continue;
        }
        unsigned int max_x = min_x + cell_size + overlap;
        if (max_border_x < max_x) {
          max_x = max_border_x;
        }

        // Pass FAST computation if one of the corners of a patch is in the mask
        if (!mask.empty()) {
          if (is_in_mask(min_y, min_x, scale_factor) ||
              is_in_mask(max_y, min_x, scale_factor) ||
              is_in_mask(min_y, max_x, scale_factor) ||
              is_in_mask(max_y, max_x, scale_factor)) {
            continue;
          }
        }

        std::vector<cv::KeyPoint> keypts_in_cell;
        cv::FAST(image_pyramid_.at(level)
                     .rowRange(min_y, max_y)
                     .colRange(min_x, max_x),
                 keypts_in_cell, orb_params_->ini_fast_thr_, true);

        // Re-compute FAST keypoint with reduced threshold if enough keypoint
        // was not got
        if (keypts_in_cell.empty()) {
          cv::FAST(image_pyramid_.at(level)
                       .rowRange(min_y, max_y)
                       .colRange(min_x, max_x),
                   keypts_in_cell, orb_params_->min_fast_thr_, true);
        }

        if (keypts_in_cell.empty()) {
          continue;
        }

        // Collect keypoints for every scale
#ifdef USE_OPENMP
#pragma omp critical
#endif
        {
          for (auto& keypt : keypts_in_cell) {
            keypt.pt.x += j * cell_size;
            keypt.pt.y += i * cell_size;
            // Check if the keypoint is in the mask
            if (!mask.empty() &&
                is_in_mask(min_border_y + keypt.pt.y, min_border_x + keypt.pt.x,
                           scale_factor)) {
              continue;
            }
            keypts_to_distribute.push_back(keypt);
          }
        }
      }
    }

    std::vector<cv::KeyPoint>& keypts_at_level = all_keypts.at(level);
    keypts_at_level.reserve(max_num_keypts_);

    // Distribute keypoints via tree
    keypts_at_level = distribute_keypoints_via_tree(
        keypts_to_distribute, min_border_x, max_border_x, min_border_y,
        max_border_y, num_keypts_per_level_.at(level));

    // Keypoint size is patch size modified by the scale factor
    const unsigned int scaled_patch_size =
        fast_patch_size_ * orb_params_->scale_factors_.at(level);

    for (auto& keypt : keypts_at_level) {
      // Translation correction (scale will be corrected after ORB description)
      keypt.pt.x += min_border_x;
      keypt.pt.y += min_border_y;
      // Set the other information
      keypt.octave = level;
      keypt.size = scaled_patch_size;
    }
  }

  // Compute orientations
  for (unsigned int level = 0; level < orb_params_->num_levels_; ++level) {
    compute_orientation(image_pyramid_.at(level), all_keypts.at(level));
  }
}

std::vector<cv::KeyPoint> orb_extractor::distribute_keypoints_via_tree(
    const std::vector<cv::KeyPoint>& keypts_to_distribute, const int min_x,
    const int max_x, const int min_y, const int max_y,
    const unsigned int num_keypts) const {
  auto nodes =
      initialize_nodes(keypts_to_distribute, min_x, max_x, min_y, max_y);

  // Forkable leaf nodes list
  // The pool is used when a forking makes nodes more than a limited number
  std::vector<std::pair<int, orb_extractor_node*>> leaf_nodes_pool;
  leaf_nodes_pool.reserve(nodes.size() * 10);

  // A flag denotes if enough keypoints have been distributed
  bool is_filled = false;

  while (true) {
    const unsigned int prev_size = nodes.size();

    auto iter = nodes.begin();
    leaf_nodes_pool.clear();

    // Fork node and remove the old one from nodes
    while (iter != nodes.end()) {
      if (iter->is_leaf_node_) {
        iter++;
        continue;
      }

      // Divide node and assign to the leaf node pool
      const auto child_nodes = iter->divide_node();
      assign_child_nodes(child_nodes, nodes, leaf_nodes_pool);
      // Remove the old node
      iter = nodes.erase(iter);
    }

    // Stop iteration when the number of nodes is over the designated size or
    // new node is not generated
    if (num_keypts <= nodes.size() || nodes.size() == prev_size) {
      is_filled = true;
      break;
    }

    // If all nodes number is more than limit, keeping nodes are selected by
    // next step
    if (num_keypts < nodes.size() + leaf_nodes_pool.size()) {
      is_filled = false;
      break;
    }
  }

  while (!is_filled) {
    // Select nodes so that keypoint number is just same as designeted number
    const unsigned int prev_size = nodes.size();

    auto prev_leaf_nodes_pool = leaf_nodes_pool;
    leaf_nodes_pool.clear();

    int max_num_keypts = 0;
    for (const auto& prev_leaf_node : prev_leaf_nodes_pool) {
      if (max_num_keypts < prev_leaf_node.first) {
        max_num_keypts = prev_leaf_node.first;
      }
    }

    // Do processes from the node which has much more keypoints
    for (int target_num_keypts = max_num_keypts; target_num_keypts > 0;
         --target_num_keypts) {
      for (const auto& prev_leaf_node : prev_leaf_nodes_pool) {
        if (prev_leaf_node.first == target_num_keypts) {
          // Divide node and assign to the leaf node pool
          const auto child_nodes = prev_leaf_node.second->divide_node();
          assign_child_nodes(child_nodes, nodes, leaf_nodes_pool);
          // Remove the old node
          nodes.erase(prev_leaf_node.second->iter_);

          if (num_keypts <= nodes.size()) {
            is_filled = true;
            break;
          }
        }
      }
    }

    // Stop dividing if the number of nodes is reached to the limit or there are
    // no dividable nodes
    if (is_filled || num_keypts <= nodes.size() || nodes.size() == prev_size) {
      is_filled = true;
      break;
    }
  }

  return find_keypoints_with_max_response(nodes);
}

std::list<orb_extractor_node> orb_extractor::initialize_nodes(
    const std::vector<cv::KeyPoint>& keypts_to_distribute, const int min_x,
    const int max_x, const int min_y, const int max_y) const {
  // The aspect ratio of the target area for keypoint detection
  const auto ratio = static_cast<double>(max_x - min_x) / (max_y - min_y);
  // The width and height of the patches allocated to the initial node
  double delta_x, delta_y;
  // The number of columns or rows
  unsigned int num_x_grid, num_y_grid;

  if (ratio > 1) {
    // If the aspect ratio is greater than 1, the patches are made in a
    // horizontal direction
    num_x_grid = std::round(ratio);
    num_y_grid = 1;
    delta_x = static_cast<double>(max_x - min_x) / num_x_grid;
    delta_y = max_y - min_y;
  } else {
    // If the aspect ratio is equal to or less than 1, the patches are made in a
    // vertical direction
    num_x_grid = 1;
    num_y_grid = std::round(1 / ratio);
    delta_x = max_x - min_y;
    delta_y = static_cast<double>(max_y - min_y) / num_y_grid;
  }

  // The number of the initial nodes
  const unsigned int num_initial_nodes = num_x_grid * num_y_grid;

  // A list of node
  std::list<orb_extractor_node> nodes;

  // Initial node objects
  std::vector<orb_extractor_node*> initial_nodes;
  initial_nodes.resize(num_initial_nodes);

  // Create initial node substances
  for (unsigned int i = 0; i < num_initial_nodes; ++i) {
    orb_extractor_node node;

    // x / y index of the node's patch in the grid
    const unsigned int ix = i % num_x_grid;
    const unsigned int iy = i / num_x_grid;

    node.pt_begin_ = cv::Point2i(delta_x * ix, delta_y * iy);
    node.pt_end_ = cv::Point2i(delta_x * (ix + 1), delta_y * (iy + 1));
    node.keypts_.reserve(keypts_to_distribute.size());

    nodes.push_back(node);
    initial_nodes.at(i) = &nodes.back();
  }

  // Assign all keypoints to initial nodes which own keypoint's position
  for (const auto& keypt : keypts_to_distribute) {
    // x / y index of the patch where the keypt is placed
    const unsigned int ix = keypt.pt.x / delta_x;
    const unsigned int iy = keypt.pt.y / delta_y;

    const unsigned int node_idx = ix + iy * num_x_grid;
    initial_nodes.at(node_idx)->keypts_.push_back(keypt);
  }

  auto iter = nodes.begin();
  while (iter != nodes.end()) {
    // Remove empty nodes
    if (iter->keypts_.empty()) {
      iter = nodes.erase(iter);
      continue;
    }
    // Set the leaf node flag if the node has only one keypoint
    iter->is_leaf_node_ = (iter->keypts_.size() == 1);
    iter++;
  }

  return nodes;
}

void orb_extractor::assign_child_nodes(
    const std::array<orb_extractor_node, 4>& child_nodes,
    std::list<orb_extractor_node>& nodes,
    std::vector<std::pair<int, orb_extractor_node*>>& leaf_nodes) const {
  for (const auto& child_node : child_nodes) {
    if (child_node.keypts_.empty()) {
      continue;
    }
    nodes.push_front(child_node);
    if (child_node.keypts_.size() == 1) {
      continue;
    }
    leaf_nodes.emplace_back(
        std::make_pair(child_node.keypts_.size(), &nodes.front()));
    // Keep the self iterator to remove from std::list randomly
    nodes.front().iter_ = nodes.begin();
  }
}

std::vector<cv::KeyPoint> orb_extractor::find_keypoints_with_max_response(
    std::list<orb_extractor_node>& nodes) const {
  // A vector contains result keypoint
  std::vector<cv::KeyPoint> result_keypts;
  result_keypts.reserve(nodes.size());

  // Store keypoints which has maximum response in the node patch
  for (auto& node : nodes) {
    auto& node_keypts = node.keypts_;
    auto& keypt = node_keypts.at(0);
    double max_response = keypt.response;

    for (unsigned int k = 1; k < node_keypts.size(); ++k) {
      if (node_keypts.at(k).response > max_response) {
        keypt = node_keypts.at(k);
        max_response = node_keypts.at(k).response;
      }
    }

    result_keypts.push_back(keypt);
  }

  return result_keypts;
}

void orb_extractor::compute_orientation(
    const cv::Mat& image, std::vector<cv::KeyPoint>& keypts) const {
  for (auto& keypt : keypts) {
    keypt.angle = ic_angle(image, keypt.pt);
  }
}

void orb_extractor::correct_keypoint_scale(
    std::vector<cv::KeyPoint>& keypts_at_level,
    const unsigned int level) const {
  if (level == 0) {
    return;
  }
  const float scale_at_level = orb_params_->scale_factors_.at(level);
  for (auto& keypt_at_level : keypts_at_level) {
    keypt_at_level.pt *= scale_at_level;
  }
}

float orb_extractor::ic_angle(const cv::Mat& image,
                              const cv::Point2f& point) const {
  int m_01 = 0, m_10 = 0;

  const uchar* const center =
      &image.at<uchar>(cvRound(point.y), cvRound(point.x));

  for (int u = -fast_half_patch_size_; u <= fast_half_patch_size_; ++u) {
    m_10 += u * center[u];
  }

  const auto step = static_cast<int>(image.step1());
  for (int v = 1; v <= fast_half_patch_size_; ++v) {
    int v_sum = 0;
    const int d = u_max_.at(v);
    for (int u = -d; u <= d; ++u) {
      const int val_plus = center[u + v * step];
      const int val_minus = center[u - v * step];
      v_sum += (val_plus - val_minus);
      m_10 += u * (val_plus + val_minus);
    }
    m_01 += v * v_sum;
  }

  return cv::fastAtan2(m_01, m_10);
}

void orb_extractor::compute_orb_descriptors(
    const cv::Mat& image, const std::vector<cv::KeyPoint>& keypts,
    cv::Mat& descriptors) const {
  descriptors = cv::Mat::zeros(keypts.size(), 32, CV_8UC1);

  for (unsigned int i = 0; i < keypts.size(); ++i) {
    compute_orb_descriptor(keypts.at(i), image, descriptors.ptr(i));
  }
}

void orb_extractor::compute_orb_descriptor(const cv::KeyPoint& keypt,
                                           const cv::Mat& image,
                                           uchar* desc) const {
  const float angle = keypt.angle * M_PI / 180.0;
  const float cos_angle = util::cos(angle);
  const float sin_angle = util::sin(angle);

  const uchar* const center =
      &image.at<uchar>(cvRound(keypt.pt.y), cvRound(keypt.pt.x));
  const auto step = static_cast<int>(image.step);

#ifdef USE_SSE_ORB
#if !((defined _MSC_VER && defined _M_X64) || \
      (defined __GNUC__ && defined __x86_64__ && defined __SSE3__) || CV_SSE3)
#error \
    "The processor is not compatible with SSE. Please configure the CMake with -DUSE_SSE_ORB=OFF."
#endif

  const __m128 _trig1 = _mm_set_ps(cos_angle, sin_angle, cos_angle, sin_angle);
  const __m128 _trig2 =
      _mm_set_ps(-sin_angle, cos_angle, -sin_angle, cos_angle);
  __m128 _point_pairs;
  __m128 _mul1;
  __m128 _mul2;
  __m128 _vs;
  __m128i _vi;
  alignas(16) int32_t ii[4];

#define COMPARE_ORB_POINTS(shift)                                             \
  (_point_pairs = _mm_load_ps(orb_point_pairs + shift),                       \
   _mul1 = _mm_mul_ps(_point_pairs, _trig1),                                  \
   _mul2 = _mm_mul_ps(_point_pairs, _trig2), _vs = _mm_hadd_ps(_mul1, _mul2), \
   _vi = _mm_cvtps_epi32(_vs),                                                \
   _mm_store_si128(reinterpret_cast<__m128i*>(ii), _vi),                      \
   center[ii[0] * step + ii[2]] < center[ii[1] * step + ii[3]])

#else

#define GET_VALUE(shift)                                        \
  (center[cvRound(*(orb_point_pairs + shift) * sin_angle +      \
                  *(orb_point_pairs + shift + 1) * cos_angle) * \
              step +                                            \
          cvRound(*(orb_point_pairs + shift) * cos_angle -      \
                  *(orb_point_pairs + shift + 1) * sin_angle)])

#define COMPARE_ORB_POINTS(shift) (GET_VALUE(shift) < GET_VALUE(shift + 2))

#endif

  // interval: (X, Y) x 2 points x 8 pairs = 32
  static constexpr unsigned interval = 32;

  for (unsigned int i = 0; i < orb_point_pairs_size / interval; ++i) {
    int32_t val = COMPARE_ORB_POINTS(i * interval);
    val |= COMPARE_ORB_POINTS(i * interval + 4) << 1;
    val |= COMPARE_ORB_POINTS(i * interval + 8) << 2;
    val |= COMPARE_ORB_POINTS(i * interval + 12) << 3;
    val |= COMPARE_ORB_POINTS(i * interval + 16) << 4;
    val |= COMPARE_ORB_POINTS(i * interval + 20) << 5;
    val |= COMPARE_ORB_POINTS(i * interval + 24) << 6;
    val |= COMPARE_ORB_POINTS(i * interval + 28) << 7;
    desc[i] = static_cast<uchar>(val);
  }

#undef GET_VALUE
#undef COMPARE_ORB_POINTS
}

}  // namespace feature
}  // namespace openvslam
