#ifndef OPENVSLAM_DATA_BOW_VOCABULARY_FWD_H
#define OPENVSLAM_DATA_BOW_VOCABULARY_FWD_H

#ifdef USE_DBOW2
#include <opencv2/core.hpp>

namespace DBoW2 {
class FORB;
// class FORB::TDescriptor; // can't do forward declaration for inner class.
template <class TDescriptor, class F>
class TemplatedVocabulary;
class BowVector;
class FeatureVector;
}  // namespace DBoW2
#else
namespace fbow {
class Vocabulary;
class BoWVector;
class BoWFeatVector;
}  // namespace fbow
#endif  // USE_DBOW2

namespace openvslam {
namespace data {

#ifdef USE_DBOW2

// FORB::TDescriptor is defined as cv::Mat
typedef DBoW2::TemplatedVocabulary<cv::Mat, DBoW2::FORB> bow_vocabulary;
typedef DBoW2::BowVector bow_vector;
typedef DBoW2::FeatureVector bow_feature_vector;

#else

typedef fbow::Vocabulary bow_vocabulary;
typedef fbow::BoWVector bow_vector;
typedef fbow::BoWFeatVector bow_feature_vector;

#endif  // USE_DBOW2

}  // namespace data
}  // namespace openvslam

#endif  // OPENVSLAM_DATA_BOW_VOCABULARY_FWD_H
