#ifndef LIGHT_SCENE_HPP
#define LIGHT_SCENE_HPP

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <optional>
#include "models/light_effect.hpp"
#include "models/light_segment.hpp"
#include "models/blockc_types.hpp"

/**
 * LightScene manages multiple LightEffect instances and shares color palettes among them.
 * 
 * Note: This class is an extension to the base specification which only defines LightSegment
 * and LightEffect. It provides higher-level management for multiple effects and color palettes.
 */
class LightScene {
public:
    /**
     * Initialize a LightScene instance.
     * 
     * @param scene_ID Unique identifier for this scene
     */
    LightScene(int scene_ID);

    /**
     * Add a LightEffect to the scene.
     * 
     * @param effect_ID Unique identifier for the effect
     * @param effect LightEffect instance to add
     */
    void add_effect(int effect_ID, std::shared_ptr<LightEffect> effect);

    /**
     * Remove a LightEffect from the scene.
     * 
     * @param effect_ID ID of the effect to remove
     */
    void remove_effect(int effect_ID);

    /**
     * Change the current color palette for all effects.
     * 
     * @param palette_id ID of the palette to use
     */
    void set_palette(const std::string& palette_id);

    /**
     * Update a specific palette's colors.
     * 
     * @param palette_id ID of the palette to update
     * @param colors New color values
     */
    void update_palette(const std::string& palette_id, const std::vector<std::vector<int>>& colors);

    /**
     * Update all palettes at once.
     * 
     * @param new_palettes Dictionary of palette_id -> color list
     */
    void update_all_palettes(const std::map<std::string, std::vector<std::vector<int>>>& new_palettes);

    /**
     * Switch to a different LightEffect.
     * 
     * @param effect_ID ID of the effect to switch to
     */
    void switch_effect(int effect_ID);

    /**
     * Update the current LightEffect.
     * Delegates to the active effect's update_all method.
     */
    void update();

    /**
     * Get the LED output from the current effect.
     * 
     * @return List of RGB color values for each LED
     */
    std::vector<std::vector<int>> get_led_output();

    /**
     * Set transition parameters for effect or palette transitions.
     * 
     * @param next_effect_idx Next effect ID (optional)
     * @param next_palette_idx Next palette ID (optional)  
     * @param fade_in_time Fade in time
     * @param fade_out_time Fade out time
     */
    void set_transition_params(std::optional<int> next_effect_idx = std::nullopt,
                              std::optional<std::string> next_palette_idx = std::nullopt,
                              float fade_in_time = 0.0f,
                              float fade_out_time = 0.0f);

    /**
     * Save the complete scene configuration to a JSON file.
     * 
     * @param file_path Path to save the JSON file
     */
    void save_to_json(const std::string& file_path);

    /**
     * Load a scene configuration from a JSON file.
     * 
     * @param file_path Path to the JSON file
     * @return A new LightScene instance with the loaded configuration
     */
    static std::shared_ptr<LightScene> load_from_json(const std::string& file_path);

    /**
     * Save only color palettes to a JSON file.
     * 
     * @param file_path Path to save the JSON file
     */
    void save_palettes_to_json(const std::string& file_path);

    /**
     * Load color palettes from a JSON file.
     * 
     * @param file_path Path to the JSON file
     */
    void load_palettes_from_json(const std::string& file_path);

    /**
     * Load effects from a JSON file.
     * 
     * @param file_path Path to the JSON file
     */
    void load_effects_from_json(const std::string& file_path);

    std::optional<int> get_current_effect_id() const;
    void set_current_effect_id(std::optional<int> effect_ID);
    const std::map<int, std::shared_ptr<LightEffect>>& get_effects() const;
    const std::shared_ptr<LightEffect> get_light_effects(int effect_id) const;
    const std::map<std::string, std::vector<std::vector<int>>>& get_palettes() const;
    void set_palettes(const std::map<std::string, std::vector<std::vector<int>>>& new_palettes);
    int get_scene_id() const;
    std::string get_current_palette() const;
    void set_current_palette(const std::string& palette_id);
    void set_palette_transition(bool state);
    void set_effect_transition(bool state);
    void set_scene_ID(int scene_ID);
    void set_effects(const std::map<int, std::shared_ptr<LightEffect>>& new_effects);
private:
    int scene_ID;
    std::map<int, std::shared_ptr<LightEffect>> effects;
    std::optional<int> current_effect_ID;
    std::map<std::string, std::vector<std::vector<int>>> palettes;
    std::string current_palette;
    std::optional<int> next_effect_idx;
    std::optional<std::string> next_palette_idx;
    float fade_in_time;
    float fade_out_time;
    float transition_start_time;
    bool effect_transition_active;
    bool palette_transition_active;
};

#endif // LIGHT_SCENE_HPP