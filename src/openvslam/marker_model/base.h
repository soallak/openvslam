#ifndef OPENVSLAM_MARKER_MODEL_BASE_H
#define OPENVSLAM_MARKER_MODEL_BASE_H

#include "openvslam/type.h"

#include <string>
#include <limits>

#include <opencv2/core.hpp>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json_fwd.hpp>

namespace openvslam {
namespace marker_model {

class base {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    //! Constructor
    explicit base(double width);

    //! Destructor
    virtual ~base();

    //! marker geometry
    const double width_;
    eigen_alloc_vector<Vec3_t> corners_pos_;

    //! Encode marker_model information as JSON
    virtual nlohmann::json to_json() const;
};

std::ostream& operator<<(std::ostream& os, const base& params);

} // namespace marker_model
} // namespace openvslam

#endif // OPENVSLAM_MARKER_MODEL_BASE_H
