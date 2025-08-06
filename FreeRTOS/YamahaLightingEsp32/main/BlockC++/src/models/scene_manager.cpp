#include "models/scene_manager.hpp"
#include "models/light_scene.hpp"
#include "models/light_effect.hpp"
#include "models/light_segment.hpp"
#include "config.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Logger setup (equivalent to Python's logging configuration)
// Note: In a real implementation, you might want to use a proper logging library like spdlog
static void log_info(const std::string& message) {
    std::cout << "[INFO] color_signal_system: " << message << std::endl;
}

static void log_error(const std::string& message) {
    std::cerr << "[ERROR] color_signal_system: " << message << std::endl;
}

SceneManager::SceneManager() {
    /**
     * Initialize SceneManager.
     */
    
    // self.scenes = {}
    scenes = std::map<int, std::shared_ptr<LightScene>>();
    
    current_scene = std::nullopt;
    
    next_scene_idx = std::nullopt;
    
    next_effect_idx = std::nullopt;
    
    next_palette_idx = std::nullopt;
    
    fade_in_time = 0.0f;
    
    fade_out_time = 0.0f;
    
    transition_start_time = 0.0f;
    
    is_transitioning = false;
    
    transition_opacity = 1.0f;
    
    osc_handler = nullptr;
}

void SceneManager::add_scene(int scene_ID, std::shared_ptr<LightScene> scene) {
    scenes[scene_ID] = scene;
    
    if (!current_scene.has_value()) {
        current_scene = scene_ID;
    }
}

void SceneManager::remove_scene(int scene_ID) {
    if (scenes.find(scene_ID) != scenes.end()) {
        
        if (current_scene.has_value() && scene_ID == current_scene.value()) {
            
            if (scenes.size() > 1) {
                
                for (const auto& pair : scenes) {
                    if (pair.first != scene_ID) {
                        current_scene = pair.first;
                        break;
                    }
                }
                
            } else {
                current_scene = std::nullopt;
            }
        }
        
        scenes.erase(scene_ID);
    }
}

void SceneManager::switch_scene(int scene_ID) {
    if (scenes.find(scene_ID) != scenes.end()) {
        
        if (fade_in_time > 0.0f || fade_out_time > 0.0f) {
            
            next_scene_idx = scene_ID;
            
            is_transitioning = true;
            
            transition_start_time = 0.0f;
            
            transition_opacity = 0.0f;
            
        } else {
            current_scene = scene_ID;
            
            next_scene_idx = std::nullopt;
        }
    }
}

void SceneManager::set_transition_params(std::optional<int> next_scene_idx,
                                        std::optional<int> next_effect_idx,
                                        std::optional<std::string> next_palette_idx,
                                        float fade_in_time,
                                        float fade_out_time) {
    this->next_scene_idx = next_scene_idx;
    
    this->next_effect_idx = next_effect_idx;
    
    this->next_palette_idx = next_palette_idx;
    
    this->fade_in_time = std::max(0.0f, fade_in_time);
    
    this->fade_out_time = std::max(0.0f, fade_out_time);
    
    if (next_scene_idx.has_value() || next_effect_idx.has_value() || next_palette_idx.has_value()) {
        
        is_transitioning = true;
        
        transition_start_time = 0.0f;
        
        transition_opacity = 0.0f;
    }
}

void SceneManager::update() {
    if (!current_scene.has_value() || scenes.find(current_scene.value()) == scenes.end()) {
        return;
    }
    
    auto current_scene_ptr = scenes[current_scene.value()];
    
    if (is_transitioning) {
        
        auto current_effect_id = current_scene_ptr->get_current_effect_id();
        const auto& scene_effects = current_scene_ptr->get_effects();
        if (current_effect_id.has_value() && scene_effects.find(current_effect_id.value()) != scene_effects.end()) {
            auto effect = scene_effects.at(current_effect_id.value());
            if (effect) {
                transition_start_time += 1.0f / effect->get_fps();
            } else {
                transition_start_time += 0.03f;
            }
        } else {
            transition_start_time += 0.03f;
        }
        
        if (transition_start_time <= fade_out_time) {
            
            transition_opacity = 1.0f - (transition_start_time / fade_out_time);
            
        } else if (transition_start_time <= fade_out_time + 0.1f) {
            
            transition_opacity = 0.0f;
            
            if (transition_start_time >= fade_out_time) {
                
                if (next_scene_idx.has_value() && scenes.find(next_scene_idx.value()) != scenes.end()) {
                    
                    current_scene = next_scene_idx.value();
                }
                
                current_scene_ptr = scenes[current_scene.value()];
                
                if (next_effect_idx.has_value()) {
                    const auto& scene_effects = current_scene_ptr->get_effects();
                    if (scene_effects.find(next_effect_idx.value()) != scene_effects.end()) {
                        current_scene_ptr->switch_effect(next_effect_idx.value());
                    }
                }
                
                if (next_palette_idx.has_value()) {
                    
                    const auto& scene_palettes = current_scene_ptr->get_palettes();
                    if (scene_palettes.find(next_palette_idx.value()) != scene_palettes.end()) {
                        current_scene_ptr->set_palette(next_palette_idx.value());
                    }
                    
                }
            }
            
        } else if (transition_start_time <= fade_out_time + 0.1f + fade_in_time) {
            
            float elapsed = transition_start_time - (fade_out_time + 0.1f);
            
            transition_opacity = elapsed / fade_in_time;
            
        } else {
            transition_opacity = 1.0f;
            
            is_transitioning = false;
            
            next_scene_idx = std::nullopt;
            
            next_effect_idx = std::nullopt;
            
            next_palette_idx = std::nullopt;
        }
    }
    
    current_scene_ptr->update();
    
    if (osc_handler != nullptr) {
    }
}

std::vector<std::vector<int>> SceneManager::get_led_output() {
    if (!current_scene.has_value() || scenes.find(current_scene.value()) == scenes.end()) {
        return std::vector<std::vector<int>>();
    }
    
    std::vector<std::vector<int>> led_colors = scenes[current_scene.value()]->get_led_output();
    
    if (is_transitioning && transition_opacity < 1.0f) {
        
        for (size_t i = 0; i < led_colors.size(); ++i) {
            for (size_t j = 0; j < led_colors[i].size(); ++j) {
                led_colors[i][j] = static_cast<int>(led_colors[i][j] * transition_opacity);
            }
        }
    }
    
    return led_colors;
}

void SceneManager::save_scenes_to_json(const std::string& file_path) {
    json data;
    data["scenes"] = json::array();
    
    if (current_scene.has_value()) {
        data["current_scene"] = current_scene.value();
    } else {
        data["current_scene"] = nullptr;
    }
    
    data["transition_params"] = {
        {"fade_in_time", fade_in_time},
        {"fade_out_time", fade_out_time}
    };
    
    for (const auto& [scene_id, scene] : scenes) {
        
        json scene_data;
        scene_data["scene_ID"] = scene->get_scene_id();
        
        auto current_effect_id = scene->get_current_effect_id();
        if (current_effect_id.has_value()) {
            scene_data["current_effect_ID"] = current_effect_id.value();
        } else {
            scene_data["current_effect_ID"] = nullptr;
        }
        
        scene_data["current_palette"] = scene->get_current_palette();
        scene_data["palettes"] = scene->get_palettes();
        scene_data["effects"] = json::object();
        
        const auto& scene_effects = scene->get_effects();
        for (const auto& scene_effect : scene_effects) {
            auto effect = scene_effect.second;
            if (!effect) continue;
            
            json effect_data;
            effect_data["effect_ID"] = effect->get_effect_id();
            effect_data["led_count"] = effect->get_led_count();
            effect_data["fps"] = effect->get_fps();
            effect_data["segments"] = json::object();
            
            const auto& effect_segments = effect->get_segments();
            for (const auto& effect_segment : effect_segments) {
                auto segment = effect_segment.second;
                if (!segment) continue;
                
                auto segment_dict = segment->to_dict();
                json segment_data = json::object();
                for (const auto& [param_key, param_value] : segment_dict) {
                    // Convert blockc_any_type to JSON
                    segment_data[param_key] = std::visit([](const auto& v) -> json {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, int>) {
                            return v;
                        } else if constexpr (std::is_same_v<T, float>) {
                            return v;
                        } else if constexpr (std::is_same_v<T, bool>) {
                            return v;
                        } else if constexpr (std::is_same_v<T, std::string>) {
                            return v;
                        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                            return json(v);
                        } else if constexpr (std::is_same_v<T, std::vector<float>>) {
                            return json(v);
                        } else {
                            return json(); // Default case
                        }
                    }, param_value);
                }
                effect_data["segments"][std::to_string(effect_segment.first)] = segment_data;
            }
            
            scene_data["effects"][std::to_string(scene_effect.first)] = effect_data;
        }
        
        data["scenes"].push_back(scene_data);
    }
    
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            log_error("Failed to open file for writing: " + file_path);
            return;
        }
        
        file << data.dump(4); // indent=4 equivalent
        file.close();
        
        log_info("Successfully saved scenes to JSON: " + file_path);
        
    } catch (const std::exception& e) {
        log_error("Error saving scenes to JSON: " + std::string(e.what()));
    }
}

bool SceneManager::load_scenes_from_json(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            log_error("Failed to open file for reading: " + file_path);
            return false;
        }
        
        json data;
        file >> data;
        file.close();
        
        scenes.clear();
        
        if (data.contains("scenes") && data["scenes"].is_array()) {
            for (const auto& scene_data : data["scenes"]) {
                
                int scene_id = scene_data["scene_ID"];
                auto scene = std::make_shared<LightScene>(scene_id);
                
                if (scene_data.contains("palettes")) {
                    std::map<std::string, std::vector<std::vector<int>>> palettes;
                    for (const auto& [palette_id, colors] : scene_data["palettes"].items()) {
                        palettes[palette_id] = colors.get<std::vector<std::vector<int>>>();
                        
                        update_palette_cache(palette_id, colors.get<std::vector<std::vector<int>>>());
                    }
                    scene->set_palettes(palettes);
                }
                
                if (scene_data.contains("current_palette")) {
                    scene->set_current_palette(scene_data["current_palette"]);
                }
                
                if (scene_data.contains("effects") && scene_data["effects"].is_object()) {
                    for (const auto& [effect_id_str, effect_data] : scene_data["effects"].items()) {
                        
                        int effect_id = std::stoi(effect_id_str);
                        
                        auto effect = std::make_shared<LightEffect>(
                            effect_id,
                            effect_data["led_count"],
                            effect_data["fps"]
                        );
                        
                        effect->set_scene(scene);
                        
                        if (effect_data.contains("segments") && effect_data["segments"].is_object()) {
                            for (const auto& [segment_id_str, segment_data] : effect_data["segments"].items()) {
                                
                                int segment_id = std::stoi(segment_id_str);
                                
                                std::map<std::string, blockc_any_type> segment_dict;
                                
                                // Helper function to convert JSON to blockc_any_type
                                auto convert_json_to_any_type = [](const json& j_val) -> blockc_any_type {
                                    if (j_val.is_number_integer()) {
                                        return j_val.get<int>();
                                    } else if (j_val.is_number_float()) {
                                        return j_val.get<float>();
                                    } else if (j_val.is_boolean()) {
                                        return j_val.get<bool>();
                                    } else if (j_val.is_string()) {
                                        return j_val.get<std::string>();
                                    } else if (j_val.is_array() && !j_val.empty()) {
                                        if (j_val[0].is_number_integer()) {
                                            return j_val.get<std::vector<int>>();
                                        } else if (j_val[0].is_number_float()) {
                                            return j_val.get<std::vector<float>>();
                                        }
                                    }
                                    return 0; // Default fallback
                                };
                                
                                // Convert each JSON field to blockc_any_type
                                for (const auto& [key, value] : segment_data.items()) {
                                    segment_dict[key] = convert_json_to_any_type(value);
                                }
                                
                                auto segment = LightSegment::from_dict(segment_dict);
                                
                                segment->set_scene(scene);
                                
                                auto rgb_color = segment->calculate_rgb(scene->get_current_palette());
                                segment->set_rgb_color(rgb_color);
                                
                                effect->add_segment(segment_id, segment);
                            }
                        }
                        
                        scene->add_effect(effect_id, effect);
                    }
                }
                
                if (scene_data.contains("current_effect_ID") && !scene_data["current_effect_ID"].is_null()) {
                    scene->set_current_effect_id(scene_data["current_effect_ID"]);
                } else if (scene->get_effects().size() > 0) {
                    std::vector<int> effect_ids;
                    for (const auto& effect_pair : scene->get_effects()) {
                        effect_ids.push_back(effect_pair.first);
                    }
                    if (!effect_ids.empty()) {
                        scene->set_current_effect_id(*std::min_element(effect_ids.begin(), effect_ids.end()));
                    }
                }
                
                add_scene(scene->get_scene_id(), scene);
            }
        }
        
        if (data.contains("current_scene") && !data["current_scene"].is_null()) {
            int current_scene_id = data["current_scene"];
            if (scenes.find(current_scene_id) != scenes.end()) {
                current_scene = current_scene_id;
            }
        } else if (!scenes.empty()) {
            // Find minimum scene ID
            auto min_scene = std::min_element(scenes.begin(), scenes.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; });
            current_scene = min_scene->first;
        }
        
        if (data.contains("transition_params")) {
            const auto& transition_params = data["transition_params"];
            
            if (transition_params.contains("fade_in_time")) {
                fade_in_time = transition_params["fade_in_time"];
            } else {
                fade_in_time = 0.0f;
            }
            
            if (transition_params.contains("fade_out_time")) {
                fade_out_time = transition_params["fade_out_time"];
            } else {
                fade_out_time = 0.0f;
            }
        }
        
        log_info("Successfully loaded scenes from JSON: " + file_path);
        
        return true;
        
    } catch (const std::exception& e) {
        log_error("Error loading JSON file: " + std::string(e.what()));
        return false;
    }
}

int SceneManager::create_new_scene(std::optional<int> scene_ID) {
    int actual_scene_id;
    if (!scene_ID.has_value()) {
        actual_scene_id = get_next_available_scene_id();
    } else {
        actual_scene_id = scene_ID.value();
    }
    
    auto scene = std::make_shared<LightScene>(actual_scene_id);
    
    auto effect = std::make_shared<LightEffect>(
        1,                    // effect_ID=1
        DEFAULT_LED_COUNT,    // led_count
        DEFAULT_FPS          // fps
    );
    
    auto segment = std::make_shared<LightSegment>(
        1,                              // segment_ID=1
        std::vector<int>{0, 1, 2, 3},  // color=[0, 1, 2, 3]
        DEFAULT_TRANSPARENCY,           // transparency
        DEFAULT_LENGTH,                 // length
        DEFAULT_MOVE_SPEED,            // move_speed
        DEFAULT_MOVE_RANGE,            // move_range
        DEFAULT_INITIAL_POSITION,      // initial_position
        DEFAULT_IS_EDGE_REFLECT,       // is_edge_reflect
        DEFAULT_DIMMER_TIME            // dimmer_time
    );
    
    effect->add_segment(1, segment);
    
    scene->add_effect(1, effect);
    
    add_scene(actual_scene_id, scene);
    
    return actual_scene_id;
}

int SceneManager::get_next_available_scene_id() const {
    int scene_id = 1;
    while (scenes.find(scene_id) != scenes.end()) {
        scene_id++;
    }
    return scene_id;
}

const std::map<int, std::shared_ptr<LightScene>>& SceneManager::get_scenes() const {
    return scenes;
}

std::optional<int> SceneManager::get_current_scene() const {
    return current_scene;
}

bool SceneManager::get_is_transitioning() const {
    return is_transitioning;
}

float SceneManager::get_fade_in_time() const {
    return fade_in_time;
}

float SceneManager::get_fade_out_time() const {
    return fade_out_time;
}

std::optional<int> SceneManager::get_next_scene_idx() const {
    return next_scene_idx;
}

std::optional<int> SceneManager::get_next_effect_idx() const {
    return next_effect_idx;
}

std::optional<std::string> SceneManager::get_next_palette_idx() const {
    return next_palette_idx;
}

float SceneManager::get_transition_opacity() const {
    return transition_opacity;
}

float SceneManager::get_transition_start_time() const {
    return transition_start_time;
}
