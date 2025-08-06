#ifndef LIGHT_SWITCH_FLG_MAKER_HPP
#define LIGHT_SWITCH_FLG_MAKER_HPP

#include <map>
#include <string>
#include "blockb_types.hpp"
class LightSwitchFlgMaker {
private:
    std::map<std::string, bool> light_switch_flgs;
    std::map<std::string, blockb_any_type> musical_change_flgs;

public:
    LightSwitchFlgMaker();
    void update_musical_change_flgs(std::map<std::string, blockb_any_type> new_musical_change_flgs);
    bool determine_song_start();
    bool determine_song_end();
    bool determine_song_change();
    bool determine_palette_change();
    bool determine_effect_change();
    bool determine_color_shift();
    
    // Returns the current state of all light switch flags
    std::map<std::string, bool> exec_make_flgs();
};

#endif  //LIGHT_SWITCH_FLG_MAKER_HPP