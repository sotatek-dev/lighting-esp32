#include "models/light_effect.hpp"
#include "config.hpp"
#include "utils/color_utils.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <nlohmann/json.hpp>
#include "models/light_scene.hpp"

using json = nlohmann::json;

LightEffect::LightEffect(int effect_ID, int led_count, int fps)
    : effect_ID_(effect_ID)
    , segments_()  // self.segments: Dict[int, LightSegment] = {}
    , led_count_(led_count)
    , fps_(fps)
    , time_step_(1.0f / fps)  // self.time_step = 1.0 / fps
    , time_(0.0f)  // self.time = 0.0
    , current_palette_("A")  // self.current_palette = "A"
{
    /**
     * Initialize a LightEffect instance.
     * 
     * Args:
     *     effect_ID: Unique identifier for this effect
     *     led_count: Total number of LEDs
     *     fps: Frame rate for animation updates
     */
}

void LightEffect::set_palette(const std::string& palette_id) {
    /**
     * Set the current palette for this effect.
     * Updates all segments to use the new palette.
     * 
     * Args:
     *     palette_id: ID of the palette to use
     */
    
    std::cout << "Effect " << effect_ID_ << " setting palette to: " << palette_id << std::endl;
    
    current_palette_ = palette_id;
    
    for (auto& [segment_id, segment] : segments_) {
        if (scene_ && segment->get_scene() == nullptr) {
            // If the segment is not part of a scene, set the scene to the current one
            segment->set_scene(scene_);
        }
        // In C++, we know LightSegment always has calculate_rgb method
        segment->calculate_rgb(palette_id);
        std::cout << "Updated segment " << segment_id << " colors with palette " << palette_id << std::endl;
    }
}

void LightEffect::add_segment(int segment_ID, std::shared_ptr<LightSegment> segment) {
    /**
     * Add a segment of light to the effect.
     * 
     * Args:
     *     segment_ID: Unique identifier for the segment
     *     segment: LightSegment instance to add
     */
    
    segments_[segment_ID] = segment;
    
    segment->calculate_rgb(current_palette_);
}

void LightEffect::remove_segment(int segment_ID) {
    /**
     * Remove a segment from the effect.
     * 
     * Args:
     *     segment_ID: ID of the segment to remove
     */
    
    if (segments_.find(segment_ID) != segments_.end()) {
        segments_.erase(segment_ID);
    }
}

void LightEffect::update_segment_param(int segment_ID, const std::string& param_name, const blockc_any_type& value) {
    /**
     * Update a parameter of a specific LightSegment.
     * 
     * Args:
     *     segment_ID: ID of the segment to update
     *     param_name: Name of the parameter to update
     *     value: New value for the parameter
     */
    
    if (segments_.find(segment_ID) != segments_.end()) {
        segments_[segment_ID]->update_param(param_name, value);
    }
}

void LightEffect::update_all() {
    /**
     * Update all segments based on the frame rate.
     * Process movement and time-based effects for each frame.
     */
    
    time_ += time_step_;
    
    for (auto& [segment_id, segment] : segments_) {
        segment->update_param("time", time_);
        
        segment->update_position(fps_);
    }
}

std::vector<std::vector<int>> LightEffect::get_led_output() {
    /**
     * Get the final color values for all LEDs, accounting for overlapping segments.
     * 
     * Returns:
     *     List of RGB color values for each LED [r, g, b]
     */
    
    std::vector<std::vector<int>> led_colors(led_count_, std::vector<int>{0, 0, 0});
    
    std::vector<float> led_transparency(led_count_, 0.0f);
    
    auto palette = get_palette(current_palette_);

    if (scene_ && scene_->get_palettes().find(current_palette_) != scene_->get_palettes().end()) {
        palette = scene_->get_palettes().at(current_palette_);
    }

    std::vector<std::pair<int, std::shared_ptr<LightSegment>>> sorted_segments(segments_.begin(), segments_.end());
    std::sort(sorted_segments.begin(), sorted_segments.end(), 
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [segment_id, segment] : sorted_segments) {
        auto segment_light_data = segment->get_light_data(palette);

        for (const auto& [led_idx, light_data] : segment_light_data) {
            const auto& segment_color = light_data.first;
            float segment_transparency = light_data.second;
            
            if (0 <= led_idx && led_idx < led_count_) {
                
                auto current_led_color = led_colors[led_idx];
                
                float current_led_transparency = led_transparency[led_idx];

                float final_transparency = segment_transparency + current_led_transparency * (1.0f - segment_transparency);
                
                final_transparency = std::max(0.0f, std::min(1.0f, final_transparency));

                std::vector<int> final_color = {0, 0, 0};
                
                if (final_transparency > 1e-6f) {
                    for (int i = 0; i < 3; ++i) {
                        final_color[i] = static_cast<int>(
                            (segment_color[i] * segment_transparency + 
                             current_led_color[i] * current_led_transparency * (1.0f - segment_transparency)) / final_transparency
                        );
                    }
                    for (int& c : final_color) {
                        c = std::max(0, std::min(255, c));
                    }
                } else {
                    final_color = {0, 0, 0};
                }

                led_colors[led_idx] = final_color;
                
                led_transparency[led_idx] = final_transparency;
            }
        }
    }
    
    for (int i = 0; i < static_cast<int>(led_colors.size()); ++i) {
        for (int j = 0; j < std::min(3, static_cast<int>(led_colors[i].size())); ++j) {
            led_colors[i][j] = std::max(0, std::min(255, led_colors[i][j]));
        }
    }
    
    return led_colors;
}

effect_dict LightEffect::to_dict() {
    /**
     * Convert the effect to a dictionary representation for serialization.
     * 
     * Returns:
     *     Dictionary containing effect properties
     */
    
    effect_dict result;
    
    // Add simple values (blockc_any_type)
    result.emplace("effect_ID", blockc_any_type{effect_ID_});
    result.emplace("led_count", blockc_any_type{led_count_});
    result.emplace("fps", blockc_any_type{fps_});
    result.emplace("time", blockc_any_type{0.0f});
    result.emplace("current_palette", blockc_any_type{current_palette_});
    
    // Create segments dictionary (type2)
    nested_dict segments_dict;
    for (const auto& [segment_id, segment] : segments_) {
        segments_dict[std::to_string(segment_id)] = segment->to_dict();
    }
    
    // Add segments as nested dictionary (type2)
    result.emplace("segments", segments_dict);
    
    return result;
}

std::shared_ptr<LightEffect> LightEffect::from_dict(const effect_dict& effect_data) {
    /**
     * Create an effect from a dictionary representation (deserialization).
     * 
     * Args:
     *     data: Dictionary containing effect properties
     *     
     * Returns:
     *     A new LightEffect instance
     */
    auto effect = std::make_shared<LightEffect>(
        std::get<int>(std::get<blockc_any_type>(effect_data.at("effect_ID"))),
        std::get<int>(std::get<blockc_any_type>(effect_data.at("led_count"))),
        std::get<int>(std::get<blockc_any_type>(effect_data.at("fps")))
    );
    
    auto current_palette_it = effect_data.find("current_palette");
    if (current_palette_it != effect_data.end()) {
        const auto& palette_variant = current_palette_it->second;
        if (std::holds_alternative<blockc_any_type>(palette_variant)) {
            const auto& palette_any = std::get<blockc_any_type>(palette_variant);
            if (std::holds_alternative<std::string>(palette_any)) {
                std::string palette_id = std::get<std::string>(palette_any);
                effect->current_palette_ = palette_id;
            }
        }
    }
    
    auto segments_it = effect_data.find("segments");
    if (segments_it != effect_data.end()) {
        const auto& segments_nested_dict = std::get<nested_dict>(segments_it->second);
        
        for (const auto& [segment_id_str, segment_data] : segments_nested_dict) {
            auto segment = LightSegment::from_dict(segment_data);
            
            effect->add_segment(std::stoi(segment_id_str), segment);
        }
    }
    
    return effect;
}

void LightEffect::save_to_json(const std::string& file_path) {
    /**
     * Save the effect configuration to a JSON file.
     * 
     * Args:
     *     file_path: Path to save the JSON file
     */
    
    auto data = to_dict();
    
    // Convert effect_dict to JSON
    json j;
    
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
    
    // Convert effect_dict to JSON
    for (const auto& [key, value] : data) {
        if (std::holds_alternative<blockc_any_type>(value)) {
            // Simple value
            j[key] = convert_any_type_to_json(std::get<blockc_any_type>(value));
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
            j[key] = segments_json;
        }
    }
    
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + file_path);
    }
    
    file << j.dump(4);  // indent=4 like Python
    file.close();
}

std::shared_ptr<LightEffect> LightEffect::load_from_json(const std::string& file_path) {
    /**
     * Load an effect configuration from a JSON file.
     * 
     * Args:
     *     file_path: Path to the JSON file
     *     
     * Returns:
     *     A new LightEffect instance with the loaded configuration
     */
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + file_path);
    }
    
    json j;
    file >> j;
    file.close();
    
    // Convert JSON back to effect_dict
    effect_dict data;
    
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
        return 0; // Default case
    };
    
    // Convert JSON to effect_dict structure
    for (const auto& [key, j_value] : j.items()) {
        if (key == "segments") {
            // Handle nested segments dictionary
            nested_dict segments_dict;
            for (const auto& [seg_key, seg_j_value] : j_value.items()) {
                any_type_dict seg_data;
                for (const auto& [param_key, param_j_value] : seg_j_value.items()) {
                    seg_data[param_key] = convert_json_to_any_type(param_j_value);
                }
                segments_dict[seg_key] = seg_data;
            }
            data[key] = segments_dict;
        } else {
            // Simple value
            data[key] = convert_json_to_any_type(j_value);
        }
    }
    
    // return cls.from_dict(data)
    return from_dict(data);
}

void LightEffect::set_current_palette(const std::string& palette) {
    current_palette_ = palette;
}

const std::map<int, std::shared_ptr<LightSegment>>& LightEffect::get_segments() const {
    return segments_;
}

int LightEffect::get_fps() const {
    return fps_;
}

int LightEffect::get_effect_id() const {
    return effect_ID_;
}

int LightEffect::get_led_count() const {
    return led_count_;
}

void LightEffect::set_scene(std::shared_ptr<LightScene> scene) {
    scene_ = scene;
}

void LightEffect::set_fps(int fps) {
    if (fps > 0) {
        fps_ = fps;
        time_step_ = 1.0f / fps;  // Update time step based on new FPS
    } else {
        throw std::invalid_argument("FPS must be greater than 0");
    }
}

void LightEffect::set_time(float time) {
    time_ = time;
}


std::string LightEffect::get_current_palette() {
    return current_palette_;
}
