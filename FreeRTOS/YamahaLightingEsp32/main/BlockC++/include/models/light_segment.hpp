#ifndef LIGHT_SEGMENT_HPP
#define LIGHT_SEGMENT_HPP

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <variant>
#include "utils/color_utils.hpp"
#include "models/blockc_types.hpp"

class LightScene;

/**
 * LightSegment represents a segment of light with specific properties like color, position, and movement.
 * This class follows the specification from the LED tape light signal processing system.
 */
class LightSegment {
public:
    /**
     * Initialize a LightSegment instance.
     * 
     * @param segment_ID Unique identifier for this segment
     * @param color List of color indices from the palette (4 elements: left to right)
     * @param transparency Transparency values for each color point (0.0~1.0)
     * @param length Lengths of each segment section (3 elements)
     * @param move_speed Speed of movement in LED particles per second (Positive: right, Negative: left)
     * @param move_range Range of movement [left_edge, right_edge]
     * @param initial_position Initial position of the segment
     * @param is_edge_reflect Whether to reflect at edges (True) or wrap around (False)
     * @param dimmer_time Fade timing parameters [fade_in_start, fade_in_end, fade_out_start, fade_out_end, cycle_length]
     * @param dimmer_time_ratio Ratio to stretch or shrink dimmer_time (default: 1.0)
     */
    LightSegment(int segment_ID, 
                 const std::vector<int>& color,
                 const std::vector<float>& transparency,
                 const std::vector<int>& length,
                 float move_speed,
                 const std::vector<int>& move_range,
                 int initial_position,
                 bool is_edge_reflect,
                 const std::vector<int>& dimmer_time,
                 float dimmer_time_ratio = 1.0);

    /**
     * Update a specific parameter of the segment.
     * 
     * @param param_name Name of the parameter to update
     * @param value New value for the parameter
     */
    void update_param(const std::string& param_name, const blockc_any_type& value);

    /**
     * Update the position of the segment based on move_speed and fps.
     * Based on the move_speed, only specified LED particles are moved in 1 second.
     * 
     * @param fps Frames per second
     */
    void update_position(int fps);

    /**
     * Calculate RGB color values from color palette indices.
     * 
     * @param palette_name Name of the palette to use
     * @return List of RGB values corresponding to each color index in format [[r0, g0, b0], ..., [r3, g3, b3]]
     */
    std::vector<std::vector<int>> calculate_rgb(const std::string& palette_name = "A");

    /**
     * Apply fade effect based on dimmer_time parameters.
     * Implements the fade in/out functionality as specified in the requirements.
     * Uses dimmer_time_ratio to scale the timing values.
     * 
     * @return Brightness level from 0.0 to 1.0
     */
    float apply_dimming();

    /**
     * Calculate the light data (color and transparency) for each LED covered by this segment.
     * 
     * @param palette The current color palette (list of RGB colors) being used by the effect
     * @return A map of LED index to (RGB color, transparency) pairs
     */
    std::map<int, std::pair<std::vector<int>, float>> get_light_data(
        const std::vector<std::vector<int>>& palette);

    /**
     * Convert segment properties to a dictionary representation (serialization).
     * 
     * Returns:
     *     A map containing all segment properties with their current values
     */
    std::map<std::string, blockc_any_type> to_dict();

    /**
     * Create a segment from a dictionary representation (deserialization).
     * 
     * Args:
     *     data: Dictionary containing segment properties
     *     
     * Returns:
     *     A new LightSegment instance
     */
    static std::shared_ptr<LightSegment> from_dict(const std::map<std::string, blockc_any_type>& data);

    /**
     * Set the current position of the segment.
     * 
     * @param position New current position value
     */
    void set_current_position(float position);

    /**
     * Get the initial position of the segment.
     * 
     * @return Initial position value
     */
    int get_initial_position() const;

    void set_scene(std::shared_ptr<LightScene> scene);
    std::shared_ptr<LightScene> get_scene() const;
    void set_rgb_color(const std::vector<std::vector<int>>& rgb_color);
    void set_fade(bool state);
    void set_time(float time);
    void set_gradient(bool state);
    std::vector<int> get_color() const;
    
private:
    int segment_ID_;
    std::vector<int> color_;
    std::vector<float> transparency_;
    std::vector<int> length_;
    float move_speed_;
    std::vector<int> move_range_;
    int initial_position_;
    float current_position_;
    bool is_edge_reflect_;
    std::vector<int> dimmer_time_;
    float dimmer_time_ratio_;
    float time_;
    int direction_;
    bool gradient_;
    bool fade_;
    std::vector<int> gradient_colors_;
    std::vector<std::vector<int>> rgb_color_;
    int total_length_;
    std::shared_ptr<LightScene> scene_;  // Reference to the scene this segment belongs to
};

#endif // LIGHT_SEGMENT_HPP