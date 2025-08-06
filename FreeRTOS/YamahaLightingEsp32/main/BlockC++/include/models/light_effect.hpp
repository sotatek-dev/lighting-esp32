#ifndef LIGHT_EFFECT_HPP
#define LIGHT_EFFECT_HPP

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <variant>
#include <optional>
#include "models/light_segment.hpp"
#include "models/blockc_types.hpp"

class LightScene;

/**
 * LightEffect manages multiple LightSegment instances to create a complete lighting effect.
 * This class follows the specification for managing LED tape light segments.
 */
class LightEffect {
public:
    /**
     * Initialize a LightEffect instance.
     * 
     * @param effect_ID Unique identifier for this effect
     * @param led_count Total number of LEDs
     * @param fps Frame rate for animation updates
     */
    LightEffect(int effect_ID, int led_count, int fps);

    /**
     * Set the current palette for this effect.
     * Updates all segments to use the new palette.
     * 
     * @param palette_id ID of the palette to use
     */
    void set_palette(const std::string& palette_id);

    /**
     * Add a segment of light to the effect.
     * 
     * @param segment_ID Unique identifier for the segment
     * @param segment LightSegment instance to add
     */
    void add_segment(int segment_ID, std::shared_ptr<LightSegment> segment);

    /**
     * Remove a segment from the effect.
     * 
     * @param segment_ID ID of the segment to remove
     */
    void remove_segment(int segment_ID);

    /**
     * Update a parameter of a specific LightSegment.
     * 
     * @param segment_ID ID of the segment to update
     * @param param_name Name of the parameter to update
     * @param value New value for the parameter
     */
    void update_segment_param(int segment_ID, const std::string& param_name, const blockc_any_type& value);

    /**
     * Update all segments based on the frame rate.
     * Process movement and time-based effects for each frame.
     */
    void update_all();

    /**
     * Get the final color values for all LEDs, accounting for overlapping segments.
     * 
     * @return List of RGB color values for each LED [r, g, b]
     */
    std::vector<std::vector<int>> get_led_output();

    /**
     * Convert the effect to a dictionary representation for serialization.
     * 
     * @return Dictionary containing effect properties with nested structure
     */
    effect_dict to_dict();

    /**
     * Create an effect from a dictionary representation (deserialization).
     * 
     * @param data Dictionary containing effect properties
     * @return A new LightEffect instance
     */
    static std::shared_ptr<LightEffect> from_dict(const effect_dict& data);

    /**
     * Save the effect configuration to a JSON file.
     * 
     * @param file_path Path to save the JSON file
     */
    void save_to_json(const std::string& file_path);

    /**
     * Load an effect configuration from a JSON file.
     * 
     * @param file_path Path to the JSON file
     * @return A new LightEffect instance with the loaded configuration
     */
    static std::shared_ptr<LightEffect> load_from_json(const std::string& file_path);

    void set_current_palette(const std::string& palette_id);

    const std::map<int, std::shared_ptr<LightSegment>>& get_segments() const;

    // std::shared_ptr<LightScene>& get_scene();
    std::string get_current_palette();
    int get_fps() const;
    int get_effect_id() const;
    int get_led_count() const;
    void set_scene(std::shared_ptr<LightScene> scene);
    void set_fps(int fps);
    void set_time(float time);

private:
    int effect_ID_;
    std::map<int, std::shared_ptr<LightSegment>> segments_;
    int led_count_;
    int fps_;
    float time_step_;
    float time_;
    std::string current_palette_;
    std::shared_ptr<LightScene> scene_; // Reference to the parent scene
};

#endif // LIGHT_EFFECT_HPP