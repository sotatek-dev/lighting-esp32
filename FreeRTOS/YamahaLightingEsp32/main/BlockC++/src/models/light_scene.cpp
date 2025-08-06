#include "models/light_scene.hpp"
#include "config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

LightScene::LightScene(int scene_ID)
    : scene_ID(scene_ID)                                         // self.scene_ID = scene_ID
    , effects()                                                  // self.effects: Dict[int, LightEffect] = {}
    , current_effect_ID(std::nullopt)                           // self.current_effect_ID = None
    , palettes(DEFAULT_COLOR_PALETTES)                          // self.palettes = DEFAULT_COLOR_PALETTES.copy()
    , current_palette("A")                                      // self.current_palette = "A"
    , next_effect_idx(std::nullopt)                             // self.next_effect_idx = None
    , next_palette_idx(std::nullopt)                            // self.next_palette_idx = None
    , fade_in_time(0.0f)                                        // self.fade_in_time = 0.0
    , fade_out_time(0.0f)                                       // self.fade_out_time = 0.0
    , transition_start_time(0.0f)                               // self.transition_start_time = 0.0
    , effect_transition_active(false)                           // self.effect_transition_active = False
    , palette_transition_active(false)                          // self.palette_transition_active = False
{
    /**
     * Initialize a LightScene instance.
     * 
     * Args:
     *     scene_ID: Unique identifier for this scene
     */
}

void LightScene::add_effect(int effect_ID, std::shared_ptr<LightEffect> effect) {
    /**
     * Add a LightEffect to the scene.
     * 
     * Args:
     *     effect_ID: Unique identifier for the effect
     *     effect: LightEffect instance to add
     */
    
    effects[effect_ID] = effect;
    
    effect->set_current_palette(current_palette);
    
    if (!current_effect_ID.has_value()) {
        current_effect_ID = effect_ID;
    }
}

void LightScene::remove_effect(int effect_ID) {
    /**
     * Remove a LightEffect from the scene.
     * 
     * Args:
     *     effect_ID: ID of the effect to remove
     */
    
    if (effects.find(effect_ID) != effects.end()) {
        effects.erase(effect_ID);

        if (current_effect_ID.has_value() && current_effect_ID.value() == effect_ID) {
            if (!effects.empty()) {
                current_effect_ID = effects.begin()->first;
            } else {
                current_effect_ID = std::nullopt;
            }
        }
    }
}

void LightScene::set_palette(const std::string& palette_id) {
    /**
     * Change the current color palette for all effects.
     * 
     * Args:
     *     palette_id: ID of the palette to use
     */
    
    if (palettes.find(palette_id) != palettes.end()) {
        current_palette = palette_id;
        
        std::cout << "Setting scene " << scene_ID << " palette to " << palette_id << std::endl;

        for (auto& [effect_id, effect] : effects) {
            effect->set_palette(palette_id);
            
            for (auto& [segment_id, segment] : effect->get_segments()) {
                segment->calculate_rgb(palette_id);
                std::cout << "Recalculated RGB for segment " << segment_id 
                         << " in effect " << effect_id << std::endl;
            }
        }
    }
}

void LightScene::update_palette(const std::string& palette_id, const std::vector<std::vector<int>>& colors) {
    /**
     * Update a specific palette's colors.
     * 
     * Args:
     *     palette_id: ID of the palette to update
     *     colors: New color values
     */
    
    if (palettes.find(palette_id) != palettes.end()) {
        palettes[palette_id] = colors;  // C++ assignment does deep copy for vector

        if (palette_id == current_palette) {
            set_palette(palette_id);
        }
    }
}

void LightScene::update_all_palettes(const std::map<std::string, std::vector<std::vector<int>>>& new_palettes) {
    /**
     * Update all palettes at once.
     * 
     * Args:
     *     new_palettes: Dictionary of palette_id -> color list
     */
    
    palettes = new_palettes;  // C++ assignment does deep copy for map
    
    if (palettes.find(current_palette) != palettes.end()) {
        set_palette(current_palette);
    }
    else if (!palettes.empty()) {
        current_palette = palettes.begin()->first;
        set_palette(current_palette);
    }
}

void LightScene::switch_effect(int effect_ID) {
    /**
     * Switch to a different LightEffect.
     * 
     * Args:
     *     effect_ID: ID of the effect to switch to
     */
    
    if (effects.find(effect_ID) != effects.end()) {
        current_effect_ID = effect_ID;
    }
}

void LightScene::update() {
    /**
     * Update the current LightEffect.
     * Delegates to the active effect's update_all method.
     */

    if (effect_transition_active) {
        if (current_effect_ID.has_value() && effects.find(current_effect_ID.value()) != effects.end()) {
            transition_start_time += 1.0f / effects[current_effect_ID.value()]->get_fps();
        }
        
        if (transition_start_time >= fade_out_time + fade_in_time) {
            if (next_effect_idx.has_value() && effects.find(next_effect_idx.value()) != effects.end()) {
                switch_effect(next_effect_idx.value());
            }
            
            effect_transition_active = false;
            next_effect_idx = std::nullopt;
            transition_start_time = 0.0f;
        }
    }
    
    if (palette_transition_active) {
        if (current_effect_ID.has_value() && effects.find(current_effect_ID.value()) != effects.end()) {
            transition_start_time += 1.0f / effects[current_effect_ID.value()]->get_fps();
        }
        
        if (transition_start_time >= fade_out_time + fade_in_time) {
            if (next_palette_idx.has_value()) {
                set_palette(next_palette_idx.value());
            }
            
            palette_transition_active = false;
            next_palette_idx = std::nullopt;
            transition_start_time = 0.0f;

        }
    }
    
    if (current_effect_ID.has_value() && effects.find(current_effect_ID.value()) != effects.end()) {
        effects[current_effect_ID.value()]->update_all();
    }
}

std::vector<std::vector<int>> LightScene::get_led_output() {
    /**
     * Get the LED output from the current effect.
     * 
     * Returns:
     *     List of RGB color values for each LED
     */
    
    if (current_effect_ID.has_value() && effects.find(current_effect_ID.value()) != effects.end()) {
        return effects[current_effect_ID.value()]->get_led_output();
    }
    return {};
}

void LightScene::set_transition_params(std::optional<int> next_effect_idx,
                                      std::optional<std::string> next_palette_idx,
                                      float fade_in_time,
                                      float fade_out_time) {
    /**
     * Set transition parameters for effect or palette transitions.
     * 
     * Args:
     *     next_effect_idx: Next effect ID (optional)
     *     next_palette_idx: Next palette ID (optional)  
     *     fade_in_time: Fade in time
     *     fade_out_time: Fade out time
     */
    
    this->next_effect_idx = next_effect_idx;
    this->next_palette_idx = next_palette_idx;
    this->fade_in_time = fade_in_time;
    this->fade_out_time = fade_out_time;
    
    transition_start_time = 0.0f;
    effect_transition_active = next_effect_idx.has_value();
    palette_transition_active = next_palette_idx.has_value();
}

void LightScene::save_to_json(const std::string& file_path) {
    /**
     * Save the complete scene configuration to a JSON file.
     * 
     * Args:
     *     file_path: Path to save the JSON file
     */
    
    json data;
    data["scene_ID"] = scene_ID;
    data["current_effect_ID"] = current_effect_ID.has_value() ? json(current_effect_ID.value()) : json();
    data["current_palette"] = current_palette;
    data["palettes"] = palettes;
    data["effects"] = json::object();
    
    // Helper function to convert blockc_any_type to JSON
    auto convert_any_type_to_json = [](const blockc_any_type& value) -> json {
        return std::visit([](const auto& v) -> json {
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
                return json{};  // Default case
            }
        }, value);
    };
    
    for (const auto& [effect_id, effect] : effects) {
        auto effect_data = effect->to_dict();
        
        // Convert effect_dict to JSON
        json effect_json;
        
        for (const auto& [key, value] : effect_data) {
            if (std::holds_alternative<blockc_any_type>(value)) {
                // Simple value
                effect_json[key] = convert_any_type_to_json(std::get<blockc_any_type>(value));
            }
            else if (std::holds_alternative<nested_dict>(value)) {
                // Handle nested segments dictionary
                const auto& segments_map = std::get<nested_dict>(value);
                json segments_json;
                
                for (const auto& [seg_key, seg_value] : segments_map) {
                    json seg_json;
                    for (const auto& [param_key, param_value] : seg_value) {
                        seg_json[param_key] = convert_any_type_to_json(param_value);
                    }
                    segments_json[seg_key] = seg_json;
                }
                effect_json[key] = segments_json;
            }
        }
        
        data["effects"][std::to_string(effect_id)] = effect_json;
    }
    
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + file_path);
    }
    file << data.dump(4);
    file.close();
}

void LightScene::save_palettes_to_json(const std::string& file_path) {
    /**
     * Save only color palettes to a JSON file.
     * 
     * Args:
     *     file_path: Path to save the JSON file
     */
    
    json data;
    data["palettes"] = palettes;
    data["current_palette"] = current_palette;
    
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + file_path);
    }
    file << data.dump(4);
    file.close();
}

std::shared_ptr<LightScene> LightScene::load_from_json(const std::string& file_path) {
    /**
     * Load a scene configuration from a JSON file.
     * 
     * Args:
     *     file_path: Path to the JSON file
     *     
     * Returns:
     *     A new LightScene instance with the loaded configuration
     */
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + file_path);
    }
    
    json data;
    file >> data;
    file.close();
    
    auto scene = std::make_shared<LightScene>(data["scene_ID"]);
    
    if (data.contains("palettes")) {
        scene->palettes = data["palettes"];
        for (const auto& [palette_id, colors] : data["palettes"].items()) {
            update_palette_cache(palette_id, colors);
        }
    }
    
    if (data.contains("current_palette")) {
        scene->current_palette = data["current_palette"];
    }

    // Helper function to convert JSON back to blockc_any_type
    auto convert_json_to_any_type = [](const json& value) -> blockc_any_type {
        if (value.is_number_integer()) {
            return value.get<int>();
        } else if (value.is_number_float()) {
            return value.get<float>();
        } else if (value.is_boolean()) {
            return value.get<bool>();
        } else if (value.is_string()) {
            return value.get<std::string>();
        } else if (value.is_array()) {
            if (!value.empty() && value[0].is_number_integer()) {
                return value.get<std::vector<int>>();
            } else if (!value.empty() && value[0].is_number_float()) {
                return value.get<std::vector<float>>();
            }
        }
        return int(0);  // Default fallback
    };

    for (const auto& [effect_id_str, effect_data] : data["effects"].items()) {
        int effect_id = std::stoi(effect_id_str);
        
        // Convert JSON back to effect_dict
        effect_dict effect_dict_data;
        
        for (const auto& [key, value] : effect_data.items()) {
            if (key == "segments") {
                // Handle nested segments dictionary
                nested_dict segments_map;
                
                for (const auto& [seg_key, seg_value] : value.items()) {
                    any_type_dict seg_dict;
                    for (const auto& [param_key, param_value] : seg_value.items()) {
                        seg_dict[param_key] = convert_json_to_any_type(param_value);
                    }
                    segments_map[seg_key] = seg_dict;
                }
                effect_dict_data[key] = segments_map;
            } else {
                // Simple value
                effect_dict_data[key] = convert_json_to_any_type(value);
            }
        }
        
        auto effect = LightEffect::from_dict(effect_dict_data);
        for (auto& [segment_id, segment] : effect->get_segments()) {
            segment->set_current_position(static_cast<float>(segment->get_initial_position()));
        }
        
        scene->add_effect(effect_id, effect);
    }
    
    if (data.contains("current_effect_ID") && !data["current_effect_ID"].is_null()) {
        scene->current_effect_ID = data["current_effect_ID"];
    }
    
    scene->set_palette(scene->current_palette);
    
    return scene;
}

void LightScene::load_palettes_from_json(const std::string& file_path) {
    /**
     * Load color palettes from a JSON file.
     * 
     * Args:
     *     file_path: Path to the JSON file
     */
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + file_path);
    }
    
    json data;
    file >> data;
    file.close();
    
    if (data.contains("palettes")) {
        palettes = data["palettes"];
        for (const auto& [palette_id, colors] : data["palettes"].items()) {
            update_palette_cache(palette_id, colors);
        }
    }
    
    std::string target_palette = current_palette;
    if (data.contains("current_palette")) {
        target_palette = data["current_palette"];
    }
    
    set_palette(target_palette);
}

void LightScene::load_effects_from_json(const std::string& file_path) {
    /**
     * Load effects from a JSON file.
     * 
     * Args:
     *     file_path: Path to the JSON file
     */
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + file_path);
    }
    
    json data;
    file >> data;
    file.close();
    
    // Helper function to convert JSON back to blockc_any_type
    auto convert_json_to_any_type = [](const json& value) -> blockc_any_type {
        if (value.is_number_integer()) {
            return value.get<int>();
        } else if (value.is_number_float()) {
            return value.get<float>();
        } else if (value.is_boolean()) {
            return value.get<bool>();
        } else if (value.is_string()) {
            return value.get<std::string>();
        } else if (value.is_array()) {
            if (!value.empty() && value[0].is_number_integer()) {
                return value.get<std::vector<int>>();
            } else if (!value.empty() && value[0].is_number_float()) {
                return value.get<std::vector<float>>();
            }
        }
        return int(0);  // Default fallback
    };
    
    if (data.contains("effects")) {
        std::map<int, std::shared_ptr<LightEffect>> new_effects;
        
        for (const auto& [effect_id_str, effect_data] : data["effects"].items()) {
            int effect_id = std::stoi(effect_id_str);
            
            // Convert JSON back to effect_dict
            effect_dict effect_dict_data;
            
            for (const auto& [key, value] : effect_data.items()) {
                if (key == "segments") {
                    // Handle nested segments dictionary
                    nested_dict segments_map;
                    
                    for (const auto& [seg_key, seg_value] : value.items()) {
                        any_type_dict seg_dict;
                        for (const auto& [param_key, param_value] : seg_value.items()) {
                            seg_dict[param_key] = convert_json_to_any_type(param_value);
                        }
                        segments_map[seg_key] = seg_dict;
                    }
                    effect_dict_data[key] = segments_map;
                } else {
                    // Simple value
                    effect_dict_data[key] = convert_json_to_any_type(value);
                }
            }
            
            auto effect = LightEffect::from_dict(effect_dict_data);
            for (auto& [segment_id, segment] : effect->get_segments()) {
                segment->set_current_position(static_cast<float>(segment->get_initial_position()));
            }
            
            new_effects[effect_id] = effect;
        }
        
        effects = new_effects;
    }
    
    if (data.contains("current_effect_ID") && !data["current_effect_ID"].is_null()) {
        current_effect_ID = data["current_effect_ID"];
    } else if (!effects.empty()) {
        current_effect_ID = effects.begin()->first;
    } else {
        current_effect_ID = std::nullopt;
    }
}

std::optional<int> LightScene::get_current_effect_id() const {
    return current_effect_ID;
}

const std::map<int, std::shared_ptr<LightEffect>>& LightScene::get_effects() const {
    return effects;
}

void LightScene::set_effects(const std::map<int, std::shared_ptr<LightEffect>>& new_effects) {
    effects = new_effects;
}

const std::shared_ptr<LightEffect> LightScene::get_light_effects(int effect_id) const {
    auto it = effects.find(effect_id);
    if (it != effects.end()) {
        return it->second;
    }
    return nullptr;
}

const std::map<std::string, std::vector<std::vector<int>>>& LightScene::get_palettes() const {
    return palettes;
}

int LightScene::get_scene_id() const {
    return scene_ID;
}

std::string LightScene::get_current_palette() const {
    return current_palette;
}

void LightScene::set_palettes(const std::map<std::string, std::vector<std::vector<int>>>& new_palettes) {
    palettes = new_palettes;
}

void LightScene::set_current_palette(const std::string& palette_id) {
    current_palette = palette_id;
    set_palette(palette_id);
}

void LightScene::set_current_effect_id(std::optional<int> effect_id) {
    current_effect_ID = effect_id;
}

void LightScene::set_palette_transition(bool state) {
    palette_transition_active = state;
}

void LightScene::set_effect_transition(bool state) {
    effect_transition_active = state;
}

void LightScene::set_scene_ID(int scene_ID) {
    this->scene_ID = scene_ID;
}   
