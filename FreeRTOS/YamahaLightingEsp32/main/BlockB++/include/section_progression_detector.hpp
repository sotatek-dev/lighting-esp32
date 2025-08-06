#ifndef SECTION_PROGRESSION_DETECTOR_HPP
#define SECTION_PROGRESSION_DETECTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include "songhook_detector.hpp"
#include "performer_switch_detector.hpp"
#include "highlight_detector.hpp"

class SectionProgressionDetector {
public:
    // Constructor
    SectionProgressionDetector();
    
    // Update the state with new input data
    void update_state(bool beat_flg, const std::vector<double>& eq_levels, double tempo, double tempo_confidence);
    
    // Get flag indicating if a song hook was detected
    bool get_songhook_flg() const;
    
    // Get flag indicating if a performer switch was detected
    bool get_performer_switch_flg() const;
    
    // Get flag indicating if a highlight was detected
    bool get_highlight_flg() const;
    
    // Public member variables to match Python implementation
    std::unique_ptr<SonghookDetector> songhook_detector;
    std::unique_ptr<PerformerSwitchDetector> performer_switch_detector;
    std::unique_ptr<HighlightDetector> highlight_detector;
};

#endif // SECTION_PROGRESSION_DETECTOR_HPP