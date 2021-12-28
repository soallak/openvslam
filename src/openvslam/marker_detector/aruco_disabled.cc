#include "openvslam/marker_detector/aruco.h"
#include "openvslam/data/marker2d.h"

namespace openvslam {
namespace marker_detector {
aruco::aruco(const camera::base* camera,
             const std::shared_ptr<marker_model::base>& marker_model,
             const unsigned int num_iter,
             const double reproj_error_threshold)
    : base(camera, marker_model, num_iter, reproj_error_threshold) {
}

bool aruco::is_valid() {
    return false;
}

void aruco::detect_2d(const cv::_InputArray& in_image, std::vector<std::vector<cv::Point2f>>& corners, std::vector<int>& ids) const {
    (void)(in_image);
    (void)(corners);
    (void)(ids);
}
} // namespace marker_detector
} // namespace openvslam
