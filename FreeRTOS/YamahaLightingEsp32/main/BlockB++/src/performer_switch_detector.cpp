#include "performer_switch_detector.hpp"
#include <Eigen/Dense>
#include <cmath>

PerformerSwitchDetector::PerformerSwitchDetector() 
    : prev_detect_frame(-1000), 
      cooldown_frames(80), 
      history(15) // maxlen=15
{
    // Initialize features map with default values
    features["band_balance_shift_flg"] = false;
    features["volume_band_ratio_diff"] = 0.0;
    features["switch_score"] = 0;
}

void PerformerSwitchDetector::calc_feature() {
    // Calculate features based on the history of volume levels
    // Features:
    // - band_balance_shift_flg: True if the cosine similarity of volume ratios is less than 0.90
    // - volume_band_ratio_diff: Absolute difference in volume ratios, indicating the change in band balance
    // - switch_score: Sum of the above two flags, indicating the overall switch score

    if (history.size() < 15) {
        features["band_balance_shift_flg"] = false;
        features["volume_band_ratio_diff"] = 0.0;
        features["switch_score"] = 0;
        return;
    }

    // Get history values
    auto volume_high_values = history.get("volume_high");
    auto volume_mid_values = history.get("volume_mid");
    auto volume_low_values = history.get("volume_low");
    
    // Current values (latest)
    double high_now = std::get<double>(volume_high_values[volume_high_values.size() - 1]);
    double mid_now = std::get<double>(volume_mid_values[volume_mid_values.size() - 1]);
    double low_now = std::get<double>(volume_low_values[volume_low_values.size() - 1]);
    
    // Past values
    double high_past = std::get<double>(volume_high_values[volume_high_values.size() - 11]);
    double mid_past = std::get<double>(volume_mid_values[volume_mid_values.size() - 11]);
    double low_past = std::get<double>(volume_low_values[volume_low_values.size() - 11]);
    
    // Calculate totals with small epsilon to avoid division by zero
    double total_now = high_now + mid_now + low_now + 1e-5;
    double total_past = high_past + mid_past + low_past + 1e-5;
    
    // Calculate ratios using Eigen
    Eigen::Vector3d ratio_now, ratio_past;
    ratio_now << high_now / total_now, mid_now / total_now, low_now / total_now;
    ratio_past << high_past / total_past, mid_past / total_past, low_past / total_past;
    
    // Add small epsilon to avoid numerical issues
    ratio_now += Eigen::Vector3d::Constant(1e-5);
    ratio_past += Eigen::Vector3d::Constant(1e-5);
    
    // Calculate cosine similarity
    double cos_sim = ratio_now.dot(ratio_past) / (ratio_now.norm() * ratio_past.norm());
    
    // Determine if there's a significant shift in balance
    bool band_balance_shift_flg = cos_sim < 0.90;
    
    // Calculate volume band ratio difference
    double volume_band_ratio_diff = (ratio_now - ratio_past).cwiseAbs().sum();
    
    // Calculate switch score
    int switch_score = band_balance_shift_flg + (volume_band_ratio_diff > 0.3 ? 1 : 0);
    
    // Update features
    features["band_balance_shift_flg"] = band_balance_shift_flg;
    features["volume_band_ratio_diff"] = volume_band_ratio_diff;
    features["switch_score"] = switch_score;
}

void PerformerSwitchDetector::update_state(const std::map<std::string, blockb_any_type>& input_frame) {
    auto eq_levels = std::get<std::vector<double>>(input_frame.at("eq_levels"));
    
    history.append("volume_high", eq_levels[1]);
    history.append("volume_mid", eq_levels[2]);
    history.append("volume_low", eq_levels[3]);
    
    calc_feature();
}

bool PerformerSwitchDetector::detect_performer_switch() const {
    // Detect performer switch based on the switch score
    // Criteria:
    // - switch_score >= 2: Significant switch detected
    // - current_frame - self.prev_detect_frame > self.cooldown_frames: Ensure cooldown period has passed

    if (features.find("switch_score") == features.end()) {
        return false;
    }
    
    int current_frame = history.get("volume_high").size() - 1;
    int switch_score = std::get<int>(features.at("switch_score"));
    
    if (switch_score >= 2 && (current_frame - prev_detect_frame > cooldown_frames)) {
        prev_detect_frame = current_frame;
        return true;
    }
    return false;
}

bool PerformerSwitchDetector::get_performer_switch_flg() {
    return detect_performer_switch();
}