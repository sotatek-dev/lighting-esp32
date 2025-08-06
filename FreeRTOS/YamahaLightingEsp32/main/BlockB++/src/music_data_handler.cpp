#include "music_data_handler.hpp"

MusicDataHandler::MusicDataHandler() {}

void MusicDataHandler::register_callback_function(callback cb) {
    func_callbacks.push_back(cb);
}

void MusicDataHandler::notify_callbacks(const std::vector<BlockBOutputData> &data) {
    for (const auto &cb : func_callbacks) {
        for (const auto &message : data) {
            cb(message);
        }
    }
}

void MusicDataHandler::music_data_handle(MusicAnalyzedData &data) {
    std::vector<double> eq_levels = {data.allpass_dB, data.LPF200_dB, data.BPF500_dB, data.BPF2000_dB, data.BPF4000_dB, data.HPF6000_dB};
    interpretor.update(data.beat, eq_levels, data.tempo, data.tempo_confidence, data.genreID, data.surround_score);
    std::map<std::string, blockb_any_type> musical_change_flgs = interpretor.detect_musical_change_flgs();
    light_switcher.update_genre(interpretor.get_latest_genre());
    light_switcher.update_tempo(interpretor.get_latest_tempo());
    light_switcher.update_light_switch_flgs(musical_change_flgs);
    auto light_switch_data = light_switcher.light_switch_flag();
    notify_callbacks(light_switch_data);

    if (interpretor.get_frame_count() % 50 == 0) {
        auto dimmer_data = light_switcher.adopt_dimmer_time_to_tempo();
        notify_callbacks(dimmer_data);
    }

}