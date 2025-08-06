#include "highlight_detector.hpp"
#include <Eigen/Dense>
#include <cmath>

HighlightDetector::HighlightDetector() 
    : prev_detect_frame(-1000), 
      cooldown_frames(100), 
      history(30) // maxlen=30
{
    // Initialize features map with default values
    features["surge_score"] = 0;
    features["long_rise_score"] = 0;
    features["tempo_confidence"] = 0.0;
    features["highlight_score"] = 0;
}

void HighlightDetector::calc_feature() {
    // Calculate features based on the history of volume levels and tempo confidence
    // Features:
    // - surge_score: True if the high volume has increased by more than 20 in the last 5 frames
    // - long_rise_score: True if the total volume has increased by more than 30 in the last 20 frames
    // - tempo_confidence: Confidence level of the tempo, indicating the stability of the tempo
    // - highlight_score: Sum of the above three flags, indicating the overall highlight score

    if (history.size() < 30) {
        features["surge_score"] = 0;
        features["long_rise_score"] = 0;
        features["tempo_confidence"] = 0.0;
        features["highlight_score"] = 0;
        return;
    }

    // Get history values
    auto volume_high_values = history.get("volume_high");
    auto volume_mid_values = history.get("volume_mid");
    auto volume_low_values = history.get("volume_low");
    auto tempo_confidence_values = history.get("tempo_confidence");
    
    // Index calculations
    int latest = volume_high_values.size() - 1;
    int short_past = volume_high_values.size() - 6;
    int long_past = volume_high_values.size() - 21;
    
    // Calculate surge score
    double surge = std::get<double>(volume_high_values[latest]) - std::get<double>(volume_high_values[short_past]);
    int surge_score = (surge > 20) ? 1 : 0;
    
    // Calculate long rise score (total volume increase)
    double now_total = std::get<double>(volume_high_values[latest]) + 
                       std::get<double>(volume_mid_values[latest]) + 
                       std::get<double>(volume_low_values[latest]);
    
    double past_total = std::get<double>(volume_high_values[long_past]) + 
                        std::get<double>(volume_mid_values[long_past]) + 
                        std::get<double>(volume_low_values[long_past]);
    
    int long_rise_score = (now_total - past_total > 30) ? 1 : 0;
    
    // Get tempo confidence
    double tempo_conf = std::get<double>(tempo_confidence_values[latest]);
    int tempo_conf_score = (tempo_conf > 0.6) ? 1 : 0;
    
    // Calculate highlight score
    int highlight_score = surge_score + long_rise_score + tempo_conf_score;
    
    // Update features
    features["surge_score"] = surge_score;
    features["long_rise_score"] = long_rise_score;
    features["tempo_confidence"] = tempo_conf;
    features["highlight_score"] = highlight_score;
}

void HighlightDetector::update_state(const std::map<std::string, blockb_any_type>& input_frame) {
    auto eq_levels = std::get<std::vector<double>>(input_frame.at("eq_levels"));
    double tempo_confidence = std::get<double>(input_frame.at("tempo_confidence"));
    
    history.append("volume_high", eq_levels[1]);
    history.append("volume_mid", eq_levels[2]);
    history.append("volume_low", eq_levels[3]);
    history.append("tempo_confidence", tempo_confidence);
    
    calc_feature();
}

bool HighlightDetector::detect_highlight() const {
    // Detect highlight based on the highlight score
    // Criteria:
    // - highlight_score >= 2: Significant highlight detected
    // - current_frame - self.prev_detect_frame > self.cooldown_frames: Ensure cooldown period has passed

    if (features.find("highlight_score") == features.end()) {
        return false;
    }
    
    int current_frame = history.get("volume_high").size() - 1;
    int highlight_score = std::get<int>(features.at("highlight_score"));
    
    if (highlight_score >= 2 && (current_frame - prev_detect_frame > cooldown_frames)) {
        prev_detect_frame = current_frame;
        return true;
    }
    return false;
}

bool HighlightDetector::get_highlight_flg() {
    return detect_highlight();
}