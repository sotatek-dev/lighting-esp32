#include "song_alternation_detector.hpp"
#include <Eigen/Dense>
#include <cmath>

SongAlternationDetector::SongAlternationDetector() 
    : history(30), // maxlen=30
      prev_detect_frame(-1000), 
      cooldown_frames(50)
{
    // Initialize features map with default values
    features["genre_change_flg"] = false;
    features["tempo_change_flg"] = false;
    features["surround_change_flg"] = false;
    features["change_score"] = 0;
}

void SongAlternationDetector::calc_feature() {
    // Calculate features based on the history of genre, tempo, and surround index
    // Features:
    // - genre_change_flg: True if the genre has changed in the last 30 frames
    // - tempo_change_flg: True if the tempo has changed by more than 20 in the last 30 frames
    // - surround_change_flg: True if the surround index has changed by 2 or more in the last 30 frames
    // - change_score: Sum of the above three flags, indicating the overall change score

    if (history.size() < 30) {
        features["genre_change_flg"] = false;
        features["tempo_change_flg"] = false;
        features["surround_change_flg"] = false;
        features["change_score"] = 0;
        return;
    }

    // Get history values
    auto genre_id = history.get("genre_id");
    auto tempo = history.get("tempo");
    auto surround = history.get("surround_index");
    
    int latest = genre_id.size() - 1;
    int past = genre_id.size() - 2;
    
    // Calculate change flags
    bool genre_change_flg = std::get<std::string>(genre_id[latest]) != std::get<std::string>(genre_id[past]);
    bool tempo_change_flg = std::abs(std::get<double>(tempo[latest]) - std::get<double>(tempo[past])) > 20;
    bool surround_change_flg = std::abs(std::get<double>(surround[latest]) - std::get<double>(surround[past])) >= 2;
    
    // Calculate change score
    int change_score = genre_change_flg + tempo_change_flg + surround_change_flg;
    
    // Update features
    features["genre_change_flg"] = genre_change_flg;
    features["tempo_change_flg"] = tempo_change_flg;
    features["surround_change_flg"] = surround_change_flg;
    features["change_score"] = change_score;
}

void SongAlternationDetector::update_history(const std::string& genre, double tempo, double surround_score) {
    history.append("genre_id", genre);
    history.append("tempo", tempo);
    history.append("surround_index", surround_score);
    calc_feature();
}

bool SongAlternationDetector::detect_song_alternation() const {
    // Detect song alternation based on the change score
    // Criteria:
    // - change_score >= 2: Significant change detected
    // - current_frame - self.prev_detect_frame > self.cooldown_frames: Ensure cooldown period has passed

    if (features.find("change_score") == features.end()) {
        return false;
    }
    
    int current_frame = history.get("genre_id").size() - 1;
    int change_score = std::get<int>(features.at("change_score"));
    
    if (change_score >= 2 && (current_frame - prev_detect_frame > cooldown_frames)) {
        prev_detect_frame = current_frame;
        return true;
    }
    return false;
}

bool SongAlternationDetector::get_change_flg() const {
    return detect_song_alternation();
}