#include "idle_state_detector.hpp"
#include <iostream>

IdleStateDetector::IdleStateDetector() : history(32), beat_counter(0) {
    // Initialize cooldown beats
    cooldown_beats = {4, 8, 16, 32};
    
    // Initialize idle counters and flags for each beat threshold
    for (const auto& beat : cooldown_beats) {
        idle_counters[beat] = 0;
        idle_flags[beat] = false;
    }
}

void IdleStateDetector::update_idle_state(const SectionProgressionDetector& section_detector, 
                                         const SongAlternationDetector& song_alternation_detector, 
                                         bool beat_flg) {
    // Append beat flag to history
    history.append("beat_flg", beat_flg);

    // If history is not fully populated yet, set all flags to false and return
    if (history.size() < 32) {
        idle_flags[4] = false;
        idle_flags[8] = false;
        idle_flags[16] = false;
        idle_flags[32] = false;
        return;
    }
    
    // Get the beat flag from two frames ago
    auto beat_history = history.get("beat_flg");
    bool previous_beat = false;
    if (beat_history.size() >= 2) {
        previous_beat = std::get<bool>(beat_history[beat_history.size() - 2]);
    }
    
    if (previous_beat) {
        // If we detected consecutive beats, treat it as a single beat and don't increment the counter
        for (const auto& cooldown_beat : cooldown_beats) {
            idle_flags[cooldown_beat] = false;
        }
    } else if (beat_flg) {
        // If this is a new beat (not consecutive), increment the counter
        beat_counter += 1;
        std::cout << "beat_counter " << beat_counter << std::endl;
        
        // Check if we've reached any of our cooldown thresholds
        for (const auto& cooldown_beat : cooldown_beats) {
            // Check if this is a cooldown beat (beat_counter is a multiple of the cooldown value)
            bool is_cooldown_beat = ((beat_counter != 0) && (beat_counter % cooldown_beat == 0));
            idle_flags[cooldown_beat] = is_cooldown_beat;
        }
  
        // Reset the counter if we've reached or exceeded 32 beats
        if (beat_counter >= 32) {
            beat_counter = 0;
        }
    }

    auto songhook_flg = section_detector.get_songhook_flg();
    auto performer_switch_flg = section_detector.get_performer_switch_flg();
    auto highlight_flg = section_detector.get_highlight_flg();
    auto song_alternation_flg = song_alternation_detector.get_change_flg();

    // If any changes were detected, reset the beat counter
    bool any_changes = songhook_flg || performer_switch_flg || highlight_flg || song_alternation_flg;
    if (any_changes) {
        std::cout << "section_results detected changes" << std::endl;
        beat_counter = 0;
    }
}

bool IdleStateDetector::get_no_change_4beats_flg() {
    return idle_flags[4];
}

bool IdleStateDetector::get_no_change_8beats_flg() {
    return idle_flags[8];
}

bool IdleStateDetector::get_no_change_16beats_flg() {
    return idle_flags[16];
}

bool IdleStateDetector::get_no_change_32beats_flg() {
    return idle_flags[32];
}