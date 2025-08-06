#ifndef IDLE_STATE_DETECTOR_HPP
#define IDLE_STATE_DETECTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <memory>
#include "frame_history.hpp"
#include "section_progression_detector.hpp"
#include "song_alternation_detector.hpp"

class IdleStateDetector {
public:
    // Constructor
    IdleStateDetector();
    
    // Update the idle state based on section detector, song alternation detector and beat flag
    void update_idle_state(const SectionProgressionDetector& section_detector, 
                          const SongAlternationDetector& song_alternation_detector, 
                          bool beat_flg);
    
    // Get idle state flags
    bool get_no_change_4beats_flg();
    bool get_no_change_8beats_flg();
    bool get_no_change_16beats_flg();
    bool get_no_change_32beats_flg();
    
private:
    FrameHistory history;
    int beat_counter;
    std::vector<int> cooldown_beats;
    std::map<int, int> idle_counters;
    std::map<int, bool> idle_flags;
};

#endif // IDLE_STATE_DETECTOR_HPP