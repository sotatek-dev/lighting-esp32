#include "music_to_light_interpretor.hpp"
#include <iostream>
#include <algorithm>
#include <string>

MusicToLightInterpretor::MusicToLightInterpretor() : frame_count(0), latest_genre(0), latest_tempo(0) {
    // Initialize the detectors and adapttors
    section_progress_detector = std::make_unique<SectionProgressionDetector>();
    song_alternation_detector = std::make_unique<SongAlternationDetector>();
    idle_state_detector = std::make_unique<IdleStateDetector>();
    dimmer_time_adaptor = std::make_unique<DimmerTimeAdaptor>();
    tempo_classifier = std::make_unique<TempoClassifier>();
}

void MusicToLightInterpretor::update(bool beat_flg, const std::vector<double>& eq_levels, 
                                    double tempo, double tempo_confidence, 
                                    int genre, int surround_score) {
    // Update the frame count
    frame_count += 1;
    
    // Update all the detectors
    section_progress_detector->update_state(beat_flg, eq_levels, tempo, tempo_confidence);
    
    song_alternation_detector->update_history(std::to_string(genre), tempo, static_cast<double>(surround_score));
    
    idle_state_detector->update_idle_state(*section_progress_detector, *song_alternation_detector, beat_flg);
    dimmer_time_adaptor->update_dimmer_time(tempo);
    tempo_classifier->update_tempo_class(tempo);

    // Store the latest genre and tempo
    latest_genre = genre;
    latest_tempo = static_cast<int>(tempo);
}

std::map<std::string, blockb_any_type> MusicToLightInterpretor::detect_musical_change_flgs() {
    // Create a map that can hold different value types
    std::map<std::string, blockb_any_type> musical_change_flgs;
    
    // Add all the flags from the detectors
    musical_change_flgs["highlight_flg"] = section_progress_detector->get_highlight_flg();
    musical_change_flgs["songhook_flg"] = section_progress_detector->get_songhook_flg();
    musical_change_flgs["performer_switch_flg"] = section_progress_detector->get_performer_switch_flg();
    musical_change_flgs["song_alternation_flg"] = song_alternation_detector->get_change_flg();
    musical_change_flgs["no_change_4beats_flg"] = idle_state_detector->get_no_change_4beats_flg();
    musical_change_flgs["no_change_8beats_flg"] = idle_state_detector->get_no_change_8beats_flg();
    musical_change_flgs["no_change_16beats_flg"] = idle_state_detector->get_no_change_16beats_flg();
    musical_change_flgs["no_change_32beats_flg"] = idle_state_detector->get_no_change_32beats_flg();
    
    // Add the additional fields from the Python implementation
    musical_change_flgs["dimmer_period_percentage"] = dimmer_time_adaptor->get_period_percentage();
    musical_change_flgs["tempo_class"] = tempo_classifier->get_class();
    musical_change_flgs["frame"] = frame_count;
    
    // These flags are hardcoded to false in the Python code
    musical_change_flgs["silence_break_flg"] = false;
    musical_change_flgs["silence_start_flg"] = false;
    
    return musical_change_flgs;
}

int MusicToLightInterpretor::get_frame_count() {
    return frame_count;
}

int MusicToLightInterpretor::get_latest_genre() {
    return latest_genre;
}

int MusicToLightInterpretor::get_latest_tempo() {
    return latest_tempo;
}