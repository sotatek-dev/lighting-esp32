#include "models/light_segment.hpp"
#include <algorithm>
#include <numeric>
#include "config.hpp"
#include <iostream>
#include <cmath>
#include "models/light_scene.hpp"

LightSegment::LightSegment(int segment_ID, 
                          const std::vector<int>& color,
                          const std::vector<float>& transparency,
                          const std::vector<int>& length,
                          float move_speed,
                          const std::vector<int>& move_range,
                          int initial_position,
                          bool is_edge_reflect,
                          const std::vector<int>& dimmer_time,
                          float dimmer_time_ratio)
    : segment_ID_(segment_ID)
    , color_(color)
    , transparency_(transparency)
    , length_(length)
    , move_speed_(move_speed)
    , move_range_(move_range)
    , initial_position_(initial_position)
    , current_position_(initial_position)
    , is_edge_reflect_(is_edge_reflect)
    , dimmer_time_(dimmer_time)
    , dimmer_time_ratio_(dimmer_time_ratio)
    , time_(0.0)
    , direction_(move_speed >= 0 ? 1 : -1)
    , gradient_(false)
    , fade_(false)
    , gradient_colors_({0, -1, -1})
{
    if (!move_range_.empty() && move_range_.size() >= 2) {
        move_range_ = {
            std::min(move_range_[0], move_range_[1]),
            std::max(move_range_[0], move_range_[1])
        };
    }

    rgb_color_ = calculate_rgb();

    total_length_ = std::accumulate(length_.begin(), length_.end(), 0);
}

void LightSegment::update_param(const std::string& param_name, const blockc_any_type& value) {
    if (param_name == "color") {
        color_ = std::get<std::vector<int>>(value);
        rgb_color_ = calculate_rgb();
    }
    else if (param_name == "gradient_colors") {
        gradient_colors_ = std::get<std::vector<int>>(value);
        if (!gradient_colors_.empty() && gradient_colors_[0] == 1) {
            gradient_ = true;
        }
    }
    else if (param_name == "gradient") {
        gradient_ = std::get<bool>(value);
        if (gradient_ && !gradient_colors_.empty() && gradient_colors_[0] == 0) {
            gradient_colors_[0] = 1;
        }
    }
    else if (param_name == "move_range") {
        auto new_range = std::get<std::vector<int>>(value);
        if (!new_range.empty() && new_range.size() >= 2) {
            move_range_ = {
                std::min(new_range[0], new_range[1]),
                std::max(new_range[0], new_range[1])
            };
            
            // Position clamping logic - ADD THIS BACK
            if (current_position_ < move_range_[0]) {
                current_position_ = static_cast<float>(move_range_[0]);
            }
            else if (current_position_ > move_range_[1]) {
                current_position_ = static_cast<float>(move_range_[1]);
            }
        }
        else {
            move_range_ = new_range;
        }
    }
    else if (param_name == "move_speed") {
        int old_direction = direction_;
        move_speed_ = std::get<float>(value);
        direction_ = (move_speed_ >= 0) ? 1 : -1;
        
        if (old_direction != direction_) {
            // Direction change logging (optional)
        }
    }
    else if (param_name == "time") {
        time_ = std::get<float>(value);
    }
    // ADD ALL MISSING CASES:
    else if (param_name == "transparency") {
        transparency_ = std::get<std::vector<float>>(value);
    }
    else if (param_name == "length") {
        length_ = std::get<std::vector<int>>(value);
        total_length_ = std::accumulate(length_.begin(), length_.end(), 0);
    }
    else if (param_name == "initial_position") {
        initial_position_ = std::get<int>(value);
    }
    else if (param_name == "is_edge_reflect") {
        is_edge_reflect_ = std::get<bool>(value);
    }
    else if (param_name == "dimmer_time") {
        dimmer_time_ = std::get<std::vector<int>>(value);
    }
    else if (param_name == "dimmer_time_ratio") {
        dimmer_time_ratio_ = std::get<float>(value);
    }
    else if (param_name == "fade") {
        fade_ = std::get<bool>(value);
    }
    // If parameter not recognized, silently ignore (like Python's setattr for unknown params)
}

void LightSegment::update_position(int fps) {
    // Calculate time step
    float dt = 1.0f / fps;
    time_ += dt;
    
    // Calculate movement delta and new position
    float delta = move_speed_ * dt;
    float new_position = current_position_ + delta;
    
    // Calculate total segment length (Python: total_length = sum(self.length))
    int total_length = std::accumulate(length_.begin(), length_.end(), 0);
    
    if (is_edge_reflect_) {
        // Handle edge reflection mode
        if (new_position < move_range_[0]) {
            // Reflect off left edge
            float excess = move_range_[0] - new_position;
            new_position = move_range_[0] + excess;
            direction_ = 1;
            move_speed_ = std::abs(move_speed_);
        }
        else if (new_position + total_length - 1 > move_range_[1]) {
            // Reflect off right edge
            float excess = new_position + total_length - 1 - move_range_[1];
            new_position = move_range_[1] - total_length + 1 - excess;
            direction_ = -1;
            move_speed_ = -std::abs(move_speed_);
        }
        
        // Clamp position to valid range
        new_position = std::max(static_cast<float>(move_range_[0]), 
                              std::min(new_position, 
                                     static_cast<float>(move_range_[1] - total_length + 1)));
    }
    else {
        // Handle wrap-around mode
        int range_width = move_range_[1] - move_range_[0] + 1;
        
        if (new_position < move_range_[0]) {
            // Wrap from left to right
            float overshoot = move_range_[0] - new_position;
            new_position = move_range_[1] - overshoot + 1;
        }
        else if (new_position + total_length - 1 > move_range_[1]) {
            // Wrap from right to left
            float overshoot = new_position + total_length - 1 - move_range_[1];
            new_position = move_range_[0] + overshoot - 1;
        }
        
        // Clamp position to valid range
        new_position = std::max(static_cast<float>(move_range_[0]), 
                              std::min(new_position, 
                                     static_cast<float>(move_range_[1] - total_length + 1)));
    }
    
    // Update current position
    current_position_ = new_position;
}

std::vector<std::vector<int>> LightSegment::calculate_rgb(const std::string& palette_name) {
    /**
     * Calculate RGB color values from color palette indices.
     * 
     * Args:
     *     palette_name: Name of the palette to use
     *         
     * Returns:
     *     List of RGB values corresponding to each color index in format [[r0, g0, b0], ..., [r3, g3, b3]]
     */
    
    // Import get_palette function equivalent from config
    auto palette = get_palette(palette_name);

    if (scene_ && scene_->get_palettes().find(palette_name) != scene_->get_palettes().end()) {
        // If scene is set, use its palette
        palette = scene_->get_palettes().at(palette_name);
    }
    
    std::vector<std::vector<int>> rgb_values;
    
    // Process each color index in the segment's color list
    for (size_t i = 0; i < color_.size(); ++i) {
        try {
            int color_idx = color_[i];
            if (color_idx >= 0 && color_idx < static_cast<int>(palette.size())) {
                // Valid color index - copy the RGB values
                rgb_values.push_back(palette[color_idx]);
            } else {
                // Invalid color index - use red as fallback
                rgb_values.push_back({255, 0, 0});
            }
            
            // Debug logging using std::cout
                         
        } catch (const std::exception& e) {
            // Error logging using std::cerr
            std::cerr << "Error getting color " << color_[i] 
                     << " from palette: " << e.what() << std::endl;
            rgb_values.push_back({255, 0, 0});  // Use red as fallback on error
        }
    }
    
    // Pad with copies of the last color until we have 4 colors
    while (rgb_values.size() < 4) {
        if (!rgb_values.empty()) {
            rgb_values.push_back(rgb_values.back());
        } else {
            rgb_values.push_back({255, 0, 0});
        }
    }
    
    return rgb_values;
}

float LightSegment::apply_dimming() {
    // Check for conditions where dimming should not be applied (return full brightness)
    if (!fade_ || dimmer_time_.size() < 5 || dimmer_time_[4] <= 0) {
        return 1.0f;
    }

    // Use the dimmer_time_ratio
    float ratio = dimmer_time_ratio_;

    // Calculate the effective cycle time based on the ratio (convert to int like Python)
    int cycle_time = static_cast<int>(dimmer_time_[4] * ratio);

    // If cycle_time is non-positive after scaling, return full brightness
    if (cycle_time <= 0) {  // ← This will now be reached when ratio <= 0
        return 1.0f;
    }

    // Calculate the current time within the cycle (in milliseconds)
    // Use integer modulo like Python: int((self.time * 1000) % cycle_time)
    int current_time_ms = static_cast<int>(time_ * 1000.0f) % cycle_time;

    // Scale the timing parameters from dimmer_time using the ratio (convert to int like Python)
    int fade_in_start = static_cast<int>(dimmer_time_[0] * ratio);
    int fade_in_end = static_cast<int>(dimmer_time_[1] * ratio);
    int fade_out_start = static_cast<int>(dimmer_time_[2] * ratio);
    int fade_out_end = static_cast<int>(dimmer_time_[3] * ratio);

    // Apply the dimming logic based on the current time within the cycle
    if (current_time_ms < fade_in_start) {
        // Before fade-in starts, brightness is 0
        return 0.0f;
    } else if (current_time_ms < fade_in_end) {
        // During fade-in
        float duration = static_cast<float>(fade_in_end - fade_in_start);
        // Use max(1, ...) like Python to avoid division by zero
        float progress = (current_time_ms - fade_in_start) / std::max(1.0f, duration);
        return progress;
    } else if (current_time_ms < fade_out_start) {
        // Full brightness period between fade-in and fade-out
        return 1.0f;
    } else if (current_time_ms < fade_out_end) {
        // During fade-out
        float duration = static_cast<float>(fade_out_end - fade_out_start);
        // Use max(1, ...) like Python to avoid division by zero
        float progress = (current_time_ms - fade_out_start) / std::max(1.0f, duration);
        return 1.0f - progress; // Brightness decreases
    } else {
        // After fade-out ends, brightness is 0 until the next cycle
        return 0.0f;
    }
}

std::map<int, std::pair<std::vector<int>, float>> LightSegment::get_light_data(
    const std::vector<std::vector<int>>& palette) {
    /**
     * Calculate the light data (color and transparency) for each LED covered by this segment.
     * 
     * Args:
     *     palette: The current color palette (list of RGB colors) being used by the effect.
     * 
     * Returns:
     *     A map of LED index to (RGB color, transparency) pairs.
     */
    
    std::map<int, std::pair<std::vector<int>, float>> light_data;
    float brightness = apply_dimming();

    // Prepare segment colors (take first 4, pad with last value if needed)
    std::vector<int> segment_colors(color_.begin(), 
                                  color_.begin() + std::min(static_cast<size_t>(4), color_.size()));
    while (segment_colors.size() < 4) {
        segment_colors.push_back(segment_colors.empty() ? 0 : segment_colors.back());
    }
    
    // Prepare segment transparencies (take first 4, pad with last value if needed)
    std::vector<float> segment_transparencies(transparency_.begin(),
                                            transparency_.begin() + std::min(static_cast<size_t>(4), transparency_.size()));
    while (segment_transparencies.size() < 4) {
        segment_transparencies.push_back(segment_transparencies.empty() ? 1.0f : segment_transparencies.back());
    }

    // Prepare segment lengths (take first 3, pad with last value if needed)
    std::vector<int> segment_lengths(length_.begin(),
                                   length_.begin() + std::min(static_cast<size_t>(3), length_.size()));
    while (segment_lengths.size() < 3) {
        segment_lengths.push_back(segment_lengths.empty() ? 0 : segment_lengths.back());
    }
    
    // Calculate total segment length
    int total_segment_length = std::accumulate(segment_lengths.begin(), segment_lengths.end(), 0);
    if (total_segment_length <= 0) {
        return light_data;  // Return empty map
    }

    // Prepare base RGB colors from palette indices
    std::vector<std::vector<int>> base_rgb;
    for (int idx : segment_colors) {
        if (idx >= 0 && idx < static_cast<int>(palette.size())) {
            base_rgb.push_back(palette[idx]);
        } else {
            base_rgb.push_back({255, 0, 0});  // Fallback to red
        }
    }

    // Calculate LED range - use double precision like Python
    int start_led = static_cast<int>(std::floor(current_position_));
    int end_led = static_cast<int>(std::floor(current_position_ + total_segment_length - 1e-9)); // Changed from 1e-9f to 1e-9

    // Process each LED in range
    for (int led_idx = start_led; led_idx <= end_led; ++led_idx) {
        float relative_pos = led_idx - current_position_;
        
        // Clamp relative position to valid range - use double precision
        relative_pos = std::max(0.0f, 
                              std::min(relative_pos, 
                                     static_cast<float>(total_segment_length) - 1e-9f)); // Keep this as 1e-9f since it's for float comparison

        std::vector<int> interpolated_color = {0, 0, 0};
        float interpolated_transparency = 1.0f;
        
        // Determine which segment section the LED is in and calculate interpolation
        std::vector<int> c1, c2;
        float tr1, tr2;
        float t;
        
        if (relative_pos < segment_lengths[0]) {
            c1 = base_rgb[0];
            c2 = base_rgb[1];
            tr1 = segment_transparencies[0];
            tr2 = segment_transparencies[1];
            t = segment_lengths[0] > 0 ? relative_pos / segment_lengths[0] : 0.0f;
        }
        else if (relative_pos < segment_lengths[0] + segment_lengths[1]) {
            c1 = base_rgb[1];
            c2 = base_rgb[2];
            tr1 = segment_transparencies[1];
            tr2 = segment_transparencies[2];
            t = segment_lengths[1] > 0 ? 
                (relative_pos - segment_lengths[0]) / segment_lengths[1] : 0.0f;
        }
        else {
            c1 = base_rgb[2];
            c2 = base_rgb[3];
            tr1 = segment_transparencies[2];
            tr2 = segment_transparencies[3];
            t = segment_lengths[2] > 0 ? 
                (relative_pos - segment_lengths[0] - segment_lengths[1]) / segment_lengths[2] : 0.0f;
        }

        // Clamp interpolation factor
        t = std::max(0.0f, std::min(1.0f, t));

        // Calculate final color and transparency
        interpolated_color = color_utils::interpolate_colors(c1, c2, t);
        interpolated_transparency = tr1 + (tr2 - tr1) * t;
        
        // Apply brightness and store result
        std::vector<int> final_color = color_utils::apply_brightness(interpolated_color, brightness);
        light_data[led_idx] = std::make_pair(final_color, interpolated_transparency);
    }
    
    return light_data;
}

std::map<std::string, blockc_any_type> LightSegment::to_dict() {
    /**
     * Convert segment properties to a dictionary representation (serialization).
     * 
     * Returns:
     *     A map containing all segment properties with their current values
     */
    return {
        {"segment_ID", segment_ID_},
        {"color", color_},
        {"transparency", transparency_},
        {"length", length_},
        {"move_speed", move_speed_},
        {"move_range", move_range_},
        {"initial_position", initial_position_},
        {"current_position", current_position_},
        {"is_edge_reflect", is_edge_reflect_},
        {"dimmer_time", dimmer_time_},
        {"dimmer_time_ratio", dimmer_time_ratio_},
        {"gradient", gradient_},
        {"fade", fade_},
        {"gradient_colors", gradient_colors_}
    };
}

std::shared_ptr<LightSegment> LightSegment::from_dict(const std::map<std::string, blockc_any_type>& data) {
    /**
     * Create a segment from a dictionary representation (deserialization).
     * 
     * Args:
     *     data: Dictionary containing segment properties
     *     
     * Returns:
     *     A new LightSegment instance
     */
    
    // Create segment with required constructor parameters
    auto segment = std::make_shared<LightSegment>(
        std::get<int>(data.at("segment_ID")),
        std::get<std::vector<int>>(data.at("color")),
        std::get<std::vector<float>>(data.at("transparency")),
        std::get<std::vector<int>>(data.at("length")),
        std::get<float>(data.at("move_speed")),
        std::get<std::vector<int>>(data.at("move_range")),
        std::get<int>(data.at("initial_position")),
        std::get<bool>(data.at("is_edge_reflect")),
        std::get<std::vector<int>>(data.at("dimmer_time")),
        // Try to get dimmer_time_ratio with default 1.0
        data.find("dimmer_time_ratio") != data.end() ? 
            std::get<float>(data.at("dimmer_time_ratio")) : 1.0f
    );
    
    // Set current_position from initial_position
    segment->current_position_ = static_cast<float>(std::get<int>(data.at("initial_position")));
    
    // Optional parameters
    if (data.find("gradient") != data.end()) {
        segment->gradient_ = std::get<bool>(data.at("gradient"));
    }
    
    if (data.find("fade") != data.end()) {
        segment->fade_ = std::get<bool>(data.at("fade"));
    }
    
    if (data.find("gradient_colors") != data.end()) {
        segment->gradient_colors_ = std::get<std::vector<int>>(data.at("gradient_colors"));
    }
    
    return segment;
}

void LightSegment::set_current_position(float position) {
    /**
     * Set the current position of the segment.
     * 
     * Args:
     *     position: New current position value
     */
    current_position_ = position;
}

int LightSegment::get_initial_position() const {
    /**
     * Get the initial position of the segment.
     * 
     * Returns:
     *     Initial position value
     */
    return initial_position_;
}

void LightSegment::set_scene(std::shared_ptr<LightScene> scene) {
    scene_ = scene;
}

std::shared_ptr<LightScene> LightSegment::get_scene() const {
    return scene_;
}

void LightSegment::set_rgb_color(const std::vector<std::vector<int>>& rgb_color) {
    rgb_color_ = rgb_color;
}

void LightSegment::set_fade(bool state) {
    fade_ = state;
}

void LightSegment::set_time(float time) {
    time_ = time;
}

std::vector<int> LightSegment::get_color() const {
    return color_;
}

void LightSegment::set_gradient(bool state) {
    gradient_ = state;
}