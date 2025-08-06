#ifndef SONG_ALTERNATION_DETECTOR_HPP
#define SONG_ALTERNATION_DETECTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <Eigen/Dense>
#include "frame_history.hpp"
#include "blockb_types.hpp"
class SongAlternationDetector {
public:
    SongAlternationDetector();
    
    // Calculate features based on the history of genre, tempo, and surround index
    void calc_feature();
    
    // Update the detector state based on input parameters
    void update_history(const std::string& genre, double tempo, double surround_score);
    
    // Detect song alternation based on the change score
    bool detect_song_alternation() const;
    
    // Get the song alternation flag
    bool get_change_flg() const;

private:
    // History to match Python implementation
    FrameHistory history;
    mutable int prev_detect_frame;
    int cooldown_frames;
    std::map<std::string, blockb_any_type> features;
};

#endif // SONG_ALTERNATION_DETECTOR_HPP