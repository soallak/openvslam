#ifndef OPENVSLAM_MARKER_MODEL_ACURO_H
#define OPENVSLAM_MARKER_MODEL_ACURO_H

#include "openvslam/type.h"
#include "openvslam/marker_model/base.h"

#include <string>
#include <limits>

#include <opencv2/core.hpp>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json_fwd.hpp>

namespace openvslam {
namespace marker_model {

class aruco : public marker_model::base {
public:
    //! Constructor
    aruco(double width, int marker_size, int max_markers);

    //! Destructor
    virtual ~aruco();

    //! marker definition
    int marker_size_;
    int max_markers_;

    //! Encode marker_model information as JSON
    virtual nlohmann::json to_json() const;
};

std::ostream& operator<<(std::ostream& os, const aruco& params);

} // namespace marker_model
} // namespace openvslam

#endif // OPENVSLAM_MARKER_MODEL_ACURO_H
