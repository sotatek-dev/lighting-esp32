#include "section_progression_detector.hpp"
#include <map>
#include <string>
#include <vector>
#include <variant>

SectionProgressionDetector::SectionProgressionDetector() 
{
    // Initialize the sub-detectors
    songhook_detector = std::make_unique<SonghookDetector>();
    performer_switch_detector = std::make_unique<PerformerSwitchDetector>();
    highlight_detector = std::make_unique<HighlightDetector>();
}

void SectionProgressionDetector::update_state(bool beat_flg, const std::vector<double>& eq_levels, 
                                              double tempo, double tempo_confidence) 
{
    // Note: The variant type must match exactly what's expected in the detector classes
    std::map<std::string, blockb_any_type> input_frame;
    input_frame["beat_flg"] = beat_flg;
    input_frame["eq_levels"] = eq_levels;
    input_frame["tempo"] = tempo;
    input_frame["tempo_confidence"] = tempo_confidence;
    
    // Update songhook detector and calculate features
    songhook_detector->update_state(input_frame);
    songhook_detector->calc_feature();
    
    // Update performer switch detector and calculate features
    performer_switch_detector->update_state(input_frame);
    performer_switch_detector->calc_feature();
    
    // Update highlight detector and calculate features
    highlight_detector->update_state(input_frame);
    highlight_detector->calc_feature();
}

bool SectionProgressionDetector::get_songhook_flg() const
{
    // Return the songhook detection flag
    return songhook_detector->detect_songhook();
}

bool SectionProgressionDetector::get_performer_switch_flg() const 
{
    // Return the performer switch detection flag
    return performer_switch_detector->detect_performer_switch();
}

bool SectionProgressionDetector::get_highlight_flg() const 
{
    // Return the highlight detection flag
    return highlight_detector->detect_highlight();
}