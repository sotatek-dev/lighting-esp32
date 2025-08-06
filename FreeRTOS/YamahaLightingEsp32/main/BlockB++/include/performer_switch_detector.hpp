#ifndef PERFORMER_SWITCH_DETECTOR_HPP
#define PERFORMER_SWITCH_DETECTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <Eigen/Dense>
#include "frame_history.hpp"
#include "blockb_types.hpp"
class PerformerSwitchDetector {
public:
    PerformerSwitchDetector();
    
    // Calculate features based on the history of volume levels
    void calc_feature();
    
    // Update the detector state based on input frame data
    void update_state(const std::map<std::string, blockb_any_type>& input_frame);
    
    // Detect performer switch based on the switch score
    bool detect_performer_switch() const;
    
    // Get the performer switch flag
    bool get_performer_switch_flg();

private:
    mutable int prev_detect_frame;
    int cooldown_frames;
    std::map<std::string, blockb_any_type> features;
    FrameHistory history;
};

#endif // PERFORMER_SWITCH_DETECTOR_HPP