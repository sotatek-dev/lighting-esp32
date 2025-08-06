#ifndef MUSIC_TO_LIGHT_INTERPRETOR_HPP
#define MUSIC_TO_LIGHT_INTERPRETOR_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include "section_progression_detector.hpp"
#include "song_alternation_detector.hpp"
#include "idle_state_detector.hpp"
#include "dimmer_time_adaptor.hpp"
#include "tempo_classifier.hpp"

class MusicToLightInterpretor {
public:
    MusicToLightInterpretor();
    
    void update(bool beat_flg, const std::vector<double>& eq_levels, double tempo, 
                double tempo_confidence, int genre, int surround_score);
    
    std::map<std::string, blockb_any_type> detect_musical_change_flgs();
    int get_frame_count();
    int get_latest_genre();
    int get_latest_tempo();
    
private:
    int frame_count;
    int latest_genre;
    int latest_tempo;
    std::unique_ptr<SectionProgressionDetector> section_progress_detector;
    std::unique_ptr<SongAlternationDetector> song_alternation_detector;
    std::unique_ptr<IdleStateDetector> idle_state_detector;
    std::unique_ptr<DimmerTimeAdaptor> dimmer_time_adaptor;
    std::unique_ptr<TempoClassifier> tempo_classifier;
};

#endif // MUSIC_TO_LIGHT_INTERPRETOR_HPP