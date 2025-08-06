#ifndef MUSIC_DATA_HANDLER_HPP
#define MUSIC_DATA_HANDLER_HPP

#include "lighting_switcher.hpp"
#include "music_to_light_interpretor.hpp"

using callback = std::function<void(const BlockBOutputData &data)>;
class MusicDataHandler {
public:
    MusicDataHandler();
    
    
    void register_callback_function(callback cb);
    void music_data_handle(MusicAnalyzedData &data);
    
private:
    void notify_callbacks(const std::vector<BlockBOutputData> &data);
    MusicToLightInterpretor interpretor; 
    LightingSwitcher light_switcher;
    std::vector<callback> func_callbacks;
};

#endif //MUSIC_DATA_HANDLER_HPP