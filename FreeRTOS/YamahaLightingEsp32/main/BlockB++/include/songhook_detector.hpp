#ifndef SONGHOOK_DETECTOR_HPP
#define SONGHOOK_DETECTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <Eigen/Dense>
#include "frame_history.hpp"
#include "blockb_types.hpp"
class SonghookDetector {
public:
    SonghookDetector();
    
    // Calculate features based on the history of tempo, surround index, and volume levels
    void calc_feature();
    
    // Update the detector state based on input frame data
    void update_state(const std::map<std::string, blockb_any_type>& input_frame);
    
    // Detect songhook based on the hook score
    bool detect_songhook() const;
    
    // Get the songhook flag
    bool get_songhook_flg() const;

private:
    mutable int prev_detect_frame;
    int cooldown_frames;
    std::map<std::string, blockb_any_type> features;
    FrameHistory history;
};

#endif // SONGHOOK_DETECTOR_HPP