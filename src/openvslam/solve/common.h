#ifndef OPENVSLAM_SOLVE_UTIL_H
#define OPENVSLAM_SOLVE_UTIL_H

#include <opencv2/core.hpp>
#include <vector>

#include "openvslam/type.h"

namespace openvslam {
namespace solve {

void normalize(const std::vector<cv::KeyPoint>& keypts,
               std::vector<cv::Point2f>& normalized_pts, Mat33_t& transform);

}  // namespace solve
}  // namespace openvslam

#endif  // OPENVSLAM_SOLVE_UTIL_H
