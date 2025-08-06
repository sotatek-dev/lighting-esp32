#ifndef LIGHTING_SWITCHER_HPP
#define LIGHTING_SWITCHER_HPP
#include "blockb_types.hpp"
#include "light_switch_flg_maker.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <functional>

class LightingSwitcher {
public:
    LightingSwitcher();
    void update_genre(int genre_id);
    void update_tempo(int tempo);
    void update_light_switch_flgs(std::map<std::string, blockb_any_type> musical_change_flgs);
    std::vector<BlockBOutputData> light_switch_flag();
    std::vector<BlockBOutputData> adopt_dimmer_time_to_tempo();
    
private:
    int genre_id;
    int tempo;
    int current_scene_id;
    int current_palette_id;
    int current_effect_id;
    std::vector<int> effect_ids;
    std::vector<std::string> palette_ids;
    std::vector<std::string> json_name_list;
    int segment_num;
    std::vector<std::vector<int>> segment_color_ids;
    std::vector<std::string> seg_color_shift_messages;
    blockb_any_type musical_change_flgs;
    std::map<std::string, bool> light_switch_flgs;
    std::vector<int> tempo_anchor;
    std::vector<float> dimmer_time_ratio_anchor;
    LightSwitchFlgMaker *light_switch_flg_maker;
    
    // Index variables for sequential selection
    int current_palette_index;
    int current_effect_index;
    
    double calc_dimmer_time_ratio();
    std::string get_next_palette_id();
    int get_next_effect_id();
};

#endif  //LIGHTING_SWITCHER_HPP