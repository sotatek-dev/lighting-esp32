#include "lighting_switcher.hpp"
#include <random>
#include <iostream>

LightingSwitcher::LightingSwitcher() {
    light_switch_flg_maker = new LightSwitchFlgMaker;
    genre_id = 1;
    tempo = 0;

    current_effect_id = 1;
    effect_ids = {1, 2, 3, 4, 5, 6, 7, 8};
    palette_ids = {"A", "B", "C", "D", "E"};
    json_name_list = {"test_effects.json", "classic.json", "dance.json", "jazz.json", "pop&rock.json"};
    
    segment_num = 10;
    segment_color_ids = std::vector<std::vector<int>>(segment_num, std::vector<int>(4, -1));
    seg_color_shift_messages = std::vector<std::string>(segment_num, "");
    tempo_anchor = {60, 80, 100, 120, 140};
    dimmer_time_ratio_anchor = {3, 1, 0.7, 0.5, 0.3};

    // Initialize indices for sequential selection
    current_palette_index = 0;
    current_effect_index = 0;
}

void LightingSwitcher::update_genre(int genre_id) {
    this->genre_id = genre_id;
}

void LightingSwitcher::update_tempo(int tempo) {
    this->tempo = tempo;
}

void LightingSwitcher::update_light_switch_flgs(std::map<std::string, blockb_any_type> musical_change_flgs) {
    light_switch_flg_maker->update_musical_change_flgs(musical_change_flgs);
    light_switch_flgs = light_switch_flg_maker->exec_make_flgs();
}

std::vector<BlockBOutputData> LightingSwitcher::light_switch_flag() {
    std::vector<BlockBOutputData> data_send;
    BlockBOutputData data_tmp;
    if (light_switch_flgs["lighting_start_flg"]) {
        std::cout << "[SLOG] : lighting_start_flg" << std::endl;
        std::string scene_json = json_name_list[genre_id];
        data_tmp.address = "/scene/1/load_effects";
        data_tmp.data = scene_json;
        data_send.push_back(data_tmp);
    } else if (light_switch_flgs["lighting_end_flg"]) {
        std::cout << "[SLOG] : lighting_end_flg" << std::endl;
        data_tmp.address = "/scene/1/change_effect";
        data_tmp.data = 0;
        data_send.push_back(data_tmp);
    } else if (light_switch_flgs["lighting_scene_change_flg"]) {
        std::cout << "[SLOG] : lighting_scene_change_flg" << std::endl;
        std::string scene_json = json_name_list[genre_id];
        data_tmp.address = "/scene/1/load_effects";
        data_tmp.data = scene_json;
        data_send.push_back(data_tmp);
    } else if (light_switch_flgs["palette_change_flg"]) {
        std::cout << "[SLOG] : palette_change_flg" << std::endl;
        std::string next_palette_id = get_next_palette_id();
        data_tmp.address = "/scene/1/change_palette";
        data_tmp.data = next_palette_id;
        data_send.push_back(data_tmp);
    } else if (light_switch_flgs["effect_change_flg"]) {
        std::cout << "[SLOG] : effect_change_flg" << std::endl;
        int next_effect_id = get_next_effect_id();
        data_tmp.address = "/scene/1/change_effect";
        data_tmp.data = next_effect_id;
        current_effect_id = next_effect_id;
        data_send.push_back(data_tmp);
    } else if (light_switch_flgs["color_shift_flg"]) {
        std::cout << "[SLOG] : color_shift_flg" << std::endl;
        for (int seg_id = 0; seg_id < segment_num; seg_id++) {
            std::vector<int>& seg_colors = segment_color_ids[seg_id];
            for (int seg_color_idx = 0 ; seg_color_idx < seg_colors.size(); seg_color_idx++) {
                seg_colors[seg_color_idx] = (seg_colors[seg_color_idx] + 1) % 5;
            }
            data_tmp.address = "/scene/1/effect/"+std::to_string(current_effect_id) + "/segment/" + std::to_string(seg_id) + "/color";
            data_tmp.data =  seg_colors;
            data_send.push_back(data_tmp);
        }
    }
    return data_send;
}


std::vector<BlockBOutputData> LightingSwitcher::adopt_dimmer_time_to_tempo() {
    int seg_id_tmp;
    std::vector<BlockBOutputData> dimmer_data;
    double dimmer_time_ratio = calc_dimmer_time_ratio();
    // std::cout << "[SLOG] : Updating dimmer time ratio to " << dimmer_time_ratio << std::endl;
    
    for (int i = 0; i < segment_num; i++) {
        BlockBOutputData data_tmp;
        seg_id_tmp = i + 1;
        data_tmp.address = "/scene/1/effect/" + std::to_string(current_effect_id) + "/segment/" + std::to_string(seg_id_tmp) + "/dimmer_time_ratio";
        data_tmp.data = dimmer_time_ratio;
        dimmer_data.push_back(data_tmp);
    }
    return dimmer_data;
}

double LightingSwitcher::calc_dimmer_time_ratio() {
    // Safety check for empty vectors
    if (tempo_anchor.empty() || dimmer_time_ratio_anchor.empty()) {
        return 1.0; // Default fallback value
    }
    
    double dimmer_time_ratio = 1.0; // Default value
    if (tempo < tempo_anchor[0]) {
        dimmer_time_ratio = dimmer_time_ratio_anchor[0];
    } else if (tempo > tempo_anchor.back()) {
        dimmer_time_ratio = dimmer_time_ratio_anchor.back();
    } else {
        for (int i = 0; i < tempo_anchor.size() - 1; i++) {
            if (tempo <= tempo_anchor[i + 1]) {
                double a = static_cast<double>(tempo_anchor[i]);
                double b = static_cast<double>(tempo_anchor[i + 1]);
                double t = static_cast<double>(tempo);
                dimmer_time_ratio = ((b - t) * dimmer_time_ratio_anchor[i] + 
                                   (t - a) * dimmer_time_ratio_anchor[i + 1]) / (b - a);
                break;
            }
        }
    }
    return dimmer_time_ratio;
}

std::string LightingSwitcher::get_next_palette_id() {
    // Increment index and wrap around using modulo operator
    current_palette_index = (current_palette_index + 1) % palette_ids.size();
    // Get palette ID at the current index
    return palette_ids[current_palette_index];
}

int LightingSwitcher::get_next_effect_id() {
    // Increment index and wrap around using modulo operator
    current_effect_index = (current_effect_index + 1) % effect_ids.size();
    // Get effect ID at the current index
    return effect_ids[current_effect_index];
}

