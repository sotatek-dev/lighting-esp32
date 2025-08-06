#include <iomanip>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>

#ifdef USE_FREERTOS
// Filesystem not available in FreeRTOS
#else
#include <filesystem>
#include <fstream>
#endif
#include <random>
#include <sstream>
#include <iostream>
#include <exception> 
#include "controllers/data_receive_handler.hpp"
#include "models/blockc_types.hpp"

using json = nlohmann::json;

#ifdef USE_FREERTOS
// Filesystem namespace not available in FreeRTOS
#else
namespace fs = std::filesystem;
#endif

DataReceiveHandler::DataReceiveHandler(std::map<int, std::shared_ptr<LightScene>> light_scenes) {
    if (light_scenes.empty()) {
        create_default_light_scene();
        
        auto scene = this->light_scenes[1];
        create_default_effects(scene); 
    } else {
        this->light_scenes = std::move(light_scenes);
    }
    
    send_binary_enabled = true;
    binary_send_interval = std::chrono::milliseconds(1000 / DEFAULT_FPS);
}

void DataReceiveHandler::create_default_light_scene() {
    auto default_scene = std::make_shared<LightScene>(1);
    this->light_scenes[1] = default_scene;
}

void DataReceiveHandler::register_output_callback_function(const output_callback& cb) {
    output_callbacks.push_back(cb);
}

void DataReceiveHandler::notify_output_callbacks(const std::vector<BlockCOutputData>& data) {
    for (const auto& cb : output_callbacks) {
        for (const auto& message : data) {
            cb(message);
        }
    }
}

void DataReceiveHandler::handle_input_data(const BlockCInputData &data) {
    auto match_result = data_dispatcher.match_pattern(data);
    
    if (match_result.has_value()) {
        const std::string& regex_name = match_result->regex_name;
        std::smatch& matches = match_result->matches;
        
        // Dispatch only to the specified callback methods
        if (regex_name == "scene_load_effects") {
            scene_load_effects_callback(data, matches);
        }
        else if (regex_name == "scene_change_effect") {
            scene_change_effect_callback(data, matches);
        }
        else if (regex_name == "scene_change_palette") {
            scene_change_palette_callback(data, matches);
        }
        else if (regex_name == "scene_effect_segment") {
            scene_effect_segment_callback(data, matches);
        }
    } else {
        // No pattern matched
        std::cout << "No pattern matched for address: " << data.address << std::endl;
        throw std::runtime_error("No matching pattern found for address: " + data.address);
    }
}

static std::vector<std::vector<int>> vector_slicing(const std::vector<std::vector<int>>& vec, int start, int end) {
    if (start < 0) start = 0;
    if (end > static_cast<int>(vec.size())) end = static_cast<int>(vec.size());
    
    if (start >= end) {
        return {};  // Empty vector if invalid range
    }
    
    auto start_it = vec.begin() + start;
    auto end_it = vec.begin() + end;
    
    return std::vector<std::vector<int>>(start_it, end_it);
}

std::vector<std::vector<uint8_t>> DataReceiveHandler::make_color_binary(std::vector<std::vector<int>> colors) {
    int finished_count_position = 0;
    std::vector<std::vector<uint8_t>> response_data_list;
    
    for (int i = 0; i < static_cast<int>(DEFAULT_LED_SEP_COUNT.size()); i++) {
        std::vector<std::vector<int>> sep_colors = vector_slicing(
            colors, 
            finished_count_position, 
            finished_count_position + DEFAULT_LED_SEP_COUNT[i]
        );
        
        std::vector<uint8_t> response_data;

        if (!sep_colors.empty()) {
            for (const auto& color : sep_colors) {
                if (color.size() >= 3) {
                    int r = std::max(0, std::min(255, color[0]));
                    int g = std::max(0, std::min(255, color[1]));
                    int b = std::max(0, std::min(255, color[2]));
                    response_data.push_back(static_cast<uint8_t>(r));
                    response_data.push_back(static_cast<uint8_t>(g));
                    response_data.push_back(static_cast<uint8_t>(b));
                    response_data.push_back(static_cast<uint8_t>(0));
                } else {
                    // Handle malformed color data - add black pixel
                    response_data.push_back(0);
                    response_data.push_back(0);
                    response_data.push_back(0);
                    response_data.push_back(0);
                }
            }
        } else {
            response_data.push_back(0);
            response_data.push_back(0);
            response_data.push_back(0);
            response_data.push_back(0);
        }

        response_data_list.push_back(response_data);
        finished_count_position += DEFAULT_LED_SEP_COUNT[i];
    }
    
    return response_data_list;
}

void DataReceiveHandler::send_led_binary_data() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto current_time_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(current_time.time_since_epoch()).count();
    
    if (!send_binary_enabled || (current_time - last_binary_send_time) < binary_send_interval) {
        return;
    }
    
    last_binary_send_time = current_time;
    
    std::vector<std::vector<int>> led_colors;
    
    if (!light_scenes.empty()) {
        std::optional<int> current_scene_id = std::nullopt;
        
        if (!current_scene_id.has_value() && !light_scenes.empty()) {
            current_scene_id = light_scenes.begin()->first;  // Get first scene ID
            for (const auto& [scene_id, scene] : light_scenes) {
                if (scene_id < current_scene_id.value()) {
                    current_scene_id = scene_id;  // Find minimum scene ID
                }
            }
        }
        
        if (current_scene_id.has_value() && light_scenes.find(current_scene_id.value()) != light_scenes.end()) {
            led_colors = light_scenes[current_scene_id.value()]->get_led_output();
        }
    }
    
    if (led_colors.empty()) {
        return;
    }
    
    try {
        auto binary_data = make_color_binary(led_colors);
        
        for (int i = 0; i < static_cast<int>(DEFAULT_LED_SEP_COUNT.size()); i++) {
            if (DEFAULT_LED_SEP_COUNT[i] == 0) {
                // When DEFAULT_LED_SEP_COUNT[i] is 0, send binary_data[0] to the i-th address
                if (!binary_data.empty() && !binary_data[0].empty()) {
                    std::vector<BlockCOutputData> data_send;
                    BlockCOutputData output_data;
                    output_data.address = LED_BINARY_OSC_ADDRESS;
                    output_data.address += "/";
                    output_data.address += std::to_string(i);
                    output_data.payload = binary_data[0];
                    data_send.push_back(output_data);
                    notify_output_callbacks(data_send);
                }
            }
            else if (i < static_cast<int>(binary_data.size()) && !binary_data[i].empty() && binary_data[i].size() > 0) {
                std::vector<BlockCOutputData> data_send;
                BlockCOutputData output_data;
                output_data.address = LED_BINARY_OSC_ADDRESS;
                output_data.address += "/";
                output_data.address += std::to_string(i);
                output_data.payload = binary_data[i];
                data_send.push_back(output_data);
                notify_output_callbacks(data_send);
            }
        }
        
        
    } catch (const std::exception& e) {
        std::cerr << "Error sending LED binary data: " << e.what() << std::endl;
        throw std::runtime_error("Error sending LED binary data: " + std::string(e.what()));
    }
}

void DataReceiveHandler::scene_load_effects_callback(const BlockCInputData& data, std::smatch& match) {
    // Extract scene_id from regex match
    int scene_id = std::stoi(match[1].str());
    
    std::string file_path = std::get<std::string>(data.data);
    
#ifdef __ZEPHYR__
    // Zephyr-specific handling - just acknowledge the request
    std::cout << "Scene load effects not supported in Zephyr - file: " << file_path << std::endl;
    
    std::vector<BlockCOutputData> data_send;
    BlockCOutputData output_data;
    output_data.address = "/scene/" + std::to_string(scene_id) + "/load_error";
    output_data.payload = std::string("File operations not supported in Zephyr RTOS");
    data_send.push_back(output_data);
    notify_output_callbacks(data_send);
    return;
#elif defined(USE_FREERTOS)
    // FreeRTOS-specific handling - just acknowledge the request
    std::cout << "Scene load effects not supported in FreeRTOS - file: " << file_path << std::endl;
    
    std::vector<BlockCOutputData> data_send;
    BlockCOutputData output_data;
    output_data.address = "/scene/" + std::to_string(scene_id) + "/load_error";
    output_data.payload = std::string("File operations not supported in FreeRTOS");
    data_send.push_back(output_data);
    notify_output_callbacks(data_send);
    return;
#else
    try {
        fs::path p(file_path);
        if (!p.is_absolute()) {
            p = fs::absolute(p);
        }
        
        if (!fs::exists(p)) {
            std::cerr << "File not found: " << p.string() << std::endl;
            
            return;
        }
        
        auto new_scene = LightScene::load_from_json(p.string());
        if (!new_scene) {
            throw std::runtime_error("Failed to load scene from JSON file");
        }
        
        new_scene->set_scene_ID(scene_id);

        auto new_palettes = new_scene->get_palettes();
        if (!new_palettes.empty()) {
            for (const auto& [palette_id, colors] : new_palettes) {
                // Update global palette cache if needed (ignoring mobile part as requested)
                std::cout << "Loaded palette " << palette_id << " with " << colors.size() << " colors" << std::endl;
            }
        }
        
        for (const auto& [effect_id, effect] : new_scene->get_effects()) {
            for (const auto& [segment_id, segment] : effect->get_segments()) {
                segment->set_scene(new_scene);
            }
        }
        
        if (light_scenes.find(scene_id) != light_scenes.end()) {
            auto old_scene = light_scenes[scene_id];

            auto current_palettes = old_scene->get_palettes();

            old_scene->set_effects(new_scene->get_effects());
            
            auto new_current_effect = new_scene->get_current_effect_id();
            auto updated_effects = old_scene->get_effects();
            if (new_current_effect.has_value()) {
                old_scene->set_current_effect_id(new_current_effect.value());
            } else if (!updated_effects.empty()) {
                old_scene->set_current_effect_id(updated_effects.begin()->first);
            } else {
                old_scene->set_current_effect_id(std::nullopt);
            }

            if (!new_palettes.empty()) {
                old_scene->set_palettes(new_palettes);
                old_scene->set_current_palette(new_scene->get_current_palette());
            } else {
                // Keep existing palettes if new scene doesn't have any
                old_scene->set_palettes(current_palettes);
            }
            
            for (const auto& [effect_id, effect] : old_scene->get_effects()) {
                effect->set_scene(old_scene);  
                
                // Set palette if effect has one 
                if (!effect->get_current_palette().empty()) {
                    effect->set_palette(effect->get_current_palette());
                }
                
                for (const auto& [segment_id, segment] : effect->get_segments()) {
                    segment->set_scene(old_scene);  
                    // Update RGB color based on current palette 
                    segment->set_rgb_color(segment->calculate_rgb(old_scene->get_current_palette()));
                }
            }
            
            std::cout << "Successfully loaded effects from " << p.string() << std::endl;
            
        } else {
            std::cerr << "Scene " << scene_id << " not found" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading effects from file: " << e.what() << std::endl;
    }
#endif
}

void DataReceiveHandler::scene_change_effect_callback(const BlockCInputData& data, std::smatch& match) {
    int scene_id = std::stoi(match[1].str());
    
    int effect_id = std::get<int>(data.data);
    
    std::cout << "Received change_effect request: Scene " << scene_id << ", Effect " << effect_id << std::endl;

    if (light_scenes.find(scene_id) == light_scenes.end()) {
        std::cout << "Scene " << scene_id << " not found" << std::endl;
        return;
    }
    
    auto scene = light_scenes[scene_id];
    
    auto effect = scene->get_light_effects(effect_id);
    if (effect == nullptr) {
        std::cout << "Effect " << effect_id << " not found in scene " << scene_id << std::endl;
        return;
    }
    
    if (scene->get_current_effect_id().has_value() && scene->get_current_effect_id().value() == effect_id) {
        std::cout << "Effect " << effect_id << " is already active in scene " << scene_id << std::endl;
        return;
    }
    
    scene->set_transition_params(
        effect_id,      // next_effect_idx
        std::nullopt,   // next_palette_idx
        0.0f,          // fade_in_time
        0.0f           // fade_out_time
    );
    
    scene->set_effect_transition(true);
    
    std::cout << "Started transition to effect " << effect_id << " in scene " << scene_id << std::endl;
}

void DataReceiveHandler::scene_change_palette_callback(const BlockCInputData& data, std::smatch& match) {
    int scene_id = std::stoi(match[1].str());
    
    // BlockB always sends string (palette_id from get_next_palette_id() - "A", "B", "C", "D", "E")
    std::string palette_id = std::get<std::string>(data.data);
    
    std::cout << "Received OSC change_palette for scene: /scene/" << scene_id << "/change_palette - Palette: " << palette_id << std::endl;
    
    if (light_scenes.find(scene_id) == light_scenes.end()) {
        std::cout << "Scene " << scene_id << " not found" << std::endl;
        return;
    }
    
    auto scene = light_scenes[scene_id];
    
    std::optional<std::string> target_palette = std::nullopt;
    
    auto palettes = scene->get_palettes();
    if (palettes.find(palette_id) != palettes.end()) {
        target_palette = palette_id;
    }
    
    if (target_palette.has_value()) {
        scene->set_transition_params(
            std::nullopt,           // next_effect_idx
            target_palette.value(), // next_palette_idx
            0.0f,                   // fade_in_time
            0.0f                    // fade_out_time
        );
        
        scene->set_palette_transition(true);
        
        std::cout << "Started palette transition to " << target_palette.value() 
                  << " for scene " << scene_id << std::endl;
        
    } else {
        std::cout << "Invalid palette ID: " << palette_id << " or palette not found in scene " << scene_id << std::endl;
    }
}

void DataReceiveHandler::scene_effect_segment_callback(const BlockCInputData& data, std::smatch& match) {
    int scene_id = std::stoi(match[1].str());
    int effect_id = std::stoi(match[2].str());
    int segment_id = std::stoi(match[3].str());
    std::string param_name = match[4].str();
    
    if (light_scenes.find(scene_id) == light_scenes.end()) {
        std::cout << "Scene " << scene_id << " not found" << std::endl;
        return;
    }
    
    auto scene = light_scenes[scene_id];
    
    auto effect = scene->get_light_effects(effect_id);
    if (effect == nullptr) {
        std::cout << "Effect " << effect_id << " not found in scene " << scene_id << std::endl;
        return;
    }
    
    auto segments_map = effect->get_segments();
    if (segments_map.find(segment_id) == segments_map.end()) {
        std::cout << "Segment " << segment_id << " not found in effect " << effect_id << std::endl;
        return;
    }
    
    auto segment = segments_map[segment_id];
    bool ui_updated = false;

    // Handle the two specific parameters that BlockB sends
    if (param_name == "color") {
        // BlockB always sends std::vector<int> for color (from seg_colors in color_shift_flg)
        auto color_vec = std::get<std::vector<int>>(data.data);
        
        segment->update_param("color", color_vec);
        std::cout << "Updated colors directly: [" << color_vec[0] << ", " 
                  << color_vec[1] << ", " << color_vec[2] << ", " << color_vec[3] << "]" << std::endl;
        ui_updated = true;
    }
    else if (param_name == "dimmer_time_ratio") {
        // BlockB always sends std::string for dimmer_time_ratio (from std::to_string in adopt_dimmer_time_to_tempo)
        float value = static_cast<float>(std::get<double>(data.data));
        float ratio = std::max(0.1f, value);
        segment->update_param("dimmer_time_ratio", ratio);
        ui_updated = true;
    }
    else {
        // BlockB only sends these two parameters, so this should not happen
        std::cout << "Unknown parameter from BlockB: " << param_name << std::endl;
    }
}

void DataReceiveHandler::scene_update() {
    for (const auto& [id, scene] : light_scenes) {
        scene->update();
    }
}

void DataReceiveHandler::create_default_effects(std::shared_ptr<LightScene> scene, int num_effects) {
    for (int effect_id = 0; effect_id < num_effects + 1; effect_id++) {
        std::shared_ptr<LightEffect> effect = std::make_shared<LightEffect>(effect_id, DEFAULT_LED_COUNT, DEFAULT_FPS);
        create_default_segments(effect);
        scene->add_effect(effect_id, effect);
    }
}

void DataReceiveHandler::create_default_segments(std::shared_ptr<LightEffect> effect, int count) {
    for (int i = 1; i < count + 1; i++) {
        int temp_value = i % 6;
        std::vector<int> temp = {temp_value, temp_value, temp_value, temp_value};
        std::shared_ptr<LightSegment> segment = std::make_shared<LightSegment>(1, temp, 
            DEFAULT_TRANSPARENCY, DEFAULT_LENGTH, DEFAULT_MOVE_SPEED, DEFAULT_MOVE_RANGE, DEFAULT_INITIAL_POSITION,
            DEFAULT_IS_EDGE_REFLECT, DEFAULT_DIMMER_TIME, 1.0);
        segment->set_gradient(false);
        segment->set_fade(false);
        effect->add_segment(i, segment);
    }
}
