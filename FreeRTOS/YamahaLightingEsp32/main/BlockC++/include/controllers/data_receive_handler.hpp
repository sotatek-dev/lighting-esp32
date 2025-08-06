#ifndef DATA_RECEIVE_HANDLER_HPP
#define DATA_RECEIVE_HANDLER_HPP

#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include "light_effect.hpp"
#include "light_segment.hpp"
#include "light_scene.hpp"
#include "config.hpp"
#include "blockc_types.hpp"
#include "data_dispatcher.hpp"

// Output callback type for BlockC
using output_callback = std::function<void(const BlockCOutputData &data)>;

class DataReceiveHandler {
public:
    DataReceiveHandler(std::map<int, std::shared_ptr<LightScene>> light_scenes);
    
    // Input data handling
    void handle_input_data(const BlockCInputData &data);
    
    // Output callback registration
    void register_output_callback_function(const output_callback& cb);
    
    void send_led_binary_data();

    void scene_update();
private:
    std::map<int, std::shared_ptr<LightScene>> light_scenes;
    bool send_binary_enabled;
    std::chrono::high_resolution_clock::time_point last_binary_send_time;
    std::chrono::milliseconds binary_send_interval;
    DataDispatcher data_dispatcher;
    
    // Output callback vector
    std::vector<output_callback> output_callbacks;

    // Output notification function
    void notify_output_callbacks(const std::vector<BlockCOutputData>& data);
    
    std::vector<std::vector<uint8_t>> make_color_binary(std::vector<std::vector<int>> colors);

    // Only keep the specified callback functions
    void scene_load_effects_callback(const BlockCInputData& data, std::smatch& match);
    void scene_change_effect_callback(const BlockCInputData& data, std::smatch& match);
    void scene_change_palette_callback(const BlockCInputData& data, std::smatch& match);
    void scene_effect_segment_callback(const BlockCInputData& data, std::smatch& match);
    
    void create_default_light_scene();

    void create_default_effects(std::shared_ptr<LightScene> scene, int num_effects = 8);
    void create_default_segments(std::shared_ptr<LightEffect> effect, int count = 10);
};

#endif // DATA_RECEIVE_HANDLER_HPP