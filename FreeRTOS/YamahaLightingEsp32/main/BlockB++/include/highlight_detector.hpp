#ifndef HIGHLIGHT_DETECTOR_HPP
#define HIGHLIGHT_DETECTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <variant>
#include "Eigen/Dense"
#include "frame_history.hpp"
#include "blockb_types.hpp"
class HighlightDetector {
public:
    HighlightDetector();
    
    // Calculate features based on the history of volume levels and tempo confidence
    void calc_feature();
    
    // Update the detector state based on input frame data
    void update_state(const std::map<std::string, blockb_any_type>& input_frame);
    
    // Detect highlight based on the highlight score
    bool detect_highlight() const;
    
    // Get the highlight flag
    bool get_highlight_flg();

private:
    mutable int prev_detect_frame;
    int cooldown_frames;
    std::map<std::string, blockb_any_type> features;
    FrameHistory history;
};

#endif // HIGHLIGHT_DETECTOR_HPP
