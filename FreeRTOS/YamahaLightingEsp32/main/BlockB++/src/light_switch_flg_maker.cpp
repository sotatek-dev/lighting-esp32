#include "../include/light_switch_flg_maker.hpp"

LightSwitchFlgMaker::LightSwitchFlgMaker() {
    light_switch_flgs = {
        {"lighting_start_flg", false},
        {"lighting_end_flg", false},
        {"lighting_scene_change_flg", false},
        {"palette_change_flg", false},
        {"effect_change_flg", false},
        {"color_shift_flg", false}
    };
}

void LightSwitchFlgMaker::update_musical_change_flgs(std::map<std::string, blockb_any_type> new_musical_change_flgs) {
    musical_change_flgs = new_musical_change_flgs;
    return;
}

bool LightSwitchFlgMaker::determine_song_start() {
    if (std::holds_alternative<bool>(musical_change_flgs["silence_break_flg"])) {
        return std::get<bool>(musical_change_flgs["silence_break_flg"]);
    }
    return false;
}

bool LightSwitchFlgMaker::determine_song_end() {
    if (std::holds_alternative<bool>(musical_change_flgs["silence_start_flg"])) {
        return std::get<bool>(musical_change_flgs["silence_start_flg"]);
    }
    return false;
}

bool LightSwitchFlgMaker::determine_song_change() {
    if (std::holds_alternative<bool>(musical_change_flgs["song_alternation_flg"])) {
        return std::get<bool>(musical_change_flgs["song_alternation_flg"]);
    }
    return false;
}

bool LightSwitchFlgMaker::determine_palette_change() {
    if (std::holds_alternative<bool>(musical_change_flgs["no_change_16beats_flg"])) {
        return std::get<bool>(musical_change_flgs["no_change_16beats_flg"]);
    }
    return false;
}

bool LightSwitchFlgMaker::determine_effect_change() {
    if (std::holds_alternative<bool>(musical_change_flgs["no_change_8beats_flg"])) {
        return std::get<bool>(musical_change_flgs["no_change_8beats_flg"]);
    }
    return false;
}

bool LightSwitchFlgMaker::determine_color_shift() {
    if (std::holds_alternative<bool>(musical_change_flgs["no_change_4beats_flg"])) {
        return std::get<bool>(musical_change_flgs["no_change_4beats_flg"]);
    }
    return false;
}

std::map<std::string, bool> LightSwitchFlgMaker::exec_make_flgs() {
    light_switch_flgs["lighting_start_flg"] = determine_song_start();
    light_switch_flgs["lighting_end_flg"] = determine_song_end();
    light_switch_flgs["lighting_scene_change_flg"] = determine_song_change();
    light_switch_flgs["palette_change_flg"] = determine_palette_change();
    light_switch_flgs["effect_change_flg"] = determine_effect_change();
    light_switch_flgs["color_shift_flg"] = determine_color_shift();

    return light_switch_flgs;
}