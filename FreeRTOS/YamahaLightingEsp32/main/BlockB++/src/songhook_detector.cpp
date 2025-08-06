#include "songhook_detector.hpp"
#include <Eigen/Dense>
#include <cmath>

SonghookDetector::SonghookDetector() 
    : prev_detect_frame(-1000), 
      cooldown_frames(100), 
      history(20) // maxlen=20
{
    // Initialize features map with default values
    features["excitement_rise_flg"] = false;
    features["volume_increase_flg"] = false;
    features["tempo_stable_flg"] = false;
    features["hook_score"] = 0;
}

void SonghookDetector::calc_feature() {
    // Calculate features based on the history of tempo, surround index, and volume levels
    // Features:
    // - excitement_rise_flg: True if the surround index has increased by 1 or more in the last 10 frames
    // - volume_increase_flg: True if the average volume has increased by more than 10% in the last 10 frames
    // - tempo_stable_flg: True if the tempo has remained stable within ±5 BPM in the last 10 frames
    // - hook_score: Sum of the above three flags, indicating the overall hook score

    if (history.size() < 20) {
        features["excitement_rise_flg"] = false;
        features["volume_increase_flg"] = false;
        features["tempo_stable_flg"] = false;
        features["hook_score"] = 0;
        return;
    }

    // Get history values
    auto surround_index_values = history.get("surround_index");
    auto volume_high_values = history.get("volume_high");
    auto volume_mid_values = history.get("volume_mid");
    auto volume_low_values = history.get("volume_low");
    auto tempo_values = history.get("tempo");
    
    // Check excitement rise
    double latest_surround = std::get<double>(surround_index_values[surround_index_values.size() - 1]);
    double past_surround = std::get<double>(surround_index_values[surround_index_values.size() - 11]);
    bool excitement_rise_flg = latest_surround - past_surround >= 1;
    
    // Calculate volume changes using Eigen
    Eigen::Vector3d vol_now, vol_before;
    vol_now << std::get<double>(volume_high_values[volume_high_values.size() - 1]),
               std::get<double>(volume_mid_values[volume_mid_values.size() - 1]),
               std::get<double>(volume_low_values[volume_low_values.size() - 1]);
               
    vol_before << std::get<double>(volume_high_values[volume_high_values.size() - 11]),
                  std::get<double>(volume_mid_values[volume_mid_values.size() - 11]),
                  std::get<double>(volume_low_values[volume_low_values.size() - 11]);
    
    // Calculate mean volumes
    double vol_now_mean = vol_now.mean();
    double vol_before_mean = vol_before.mean();
    bool volume_increase_flg = vol_now_mean > vol_before_mean * 1.1;
    
    // Check tempo stability using Eigen
    Eigen::VectorXd recent_tempo(10);
    for (int i = 0; i < 10; i++) {
        recent_tempo(i) = std::get<double>(tempo_values[tempo_values.size() - 10 + i]);
    }
    
    double avg_tempo = recent_tempo.mean();
    
    // Check if all tempos are within ±5 BPM of the average
    bool tempo_stable_flg = true;
    for (int i = 0; i < 10; i++) {
        if (std::abs(recent_tempo(i) - avg_tempo) >= 5) {
            tempo_stable_flg = false;
            break;
        }
    }
    
    // Calculate hook score (sum of boolean flags)
    int hook_score = excitement_rise_flg + volume_increase_flg + tempo_stable_flg;
    
    // Update features
    features["excitement_rise_flg"] = excitement_rise_flg;
    features["volume_increase_flg"] = volume_increase_flg;
    features["tempo_stable_flg"] = tempo_stable_flg;
    features["hook_score"] = hook_score;
}

void SonghookDetector::update_state(const std::map<std::string, blockb_any_type>& input_frame) {
    auto eq_levels = std::get<std::vector<double>>(input_frame.at("eq_levels"));
    
    history.append("tempo", std::get<double>(input_frame.at("tempo")));
    history.append("surround_index", eq_levels[0]);
    history.append("volume_high", eq_levels[1]);
    history.append("volume_mid", eq_levels[2]);
    history.append("volume_low", eq_levels[3]);
    
    calc_feature();
}

bool SonghookDetector::detect_songhook() const {
    // Detect songhook based on the hook score
    // Criteria:
    // - hook_score >= 2: Significant hook detected
    // - current_frame - self.prev_detect_frame > self.cooldown_frames: Ensure cooldown period has passed

    if (features.find("hook_score") == features.end()) {
        return false;
    }
    
    int current_frame = history.get("tempo").size() - 1;
    int hook_score = std::get<int>(features.at("hook_score"));
    
    if (hook_score >= 2 && (current_frame - prev_detect_frame > cooldown_frames)) {
        prev_detect_frame = current_frame;
        return true;
    }
    return false;
}

bool SonghookDetector::get_songhook_flg() const {
    return detect_songhook();
}