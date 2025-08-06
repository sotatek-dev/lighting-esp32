#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <optional>
#include "models/light_scene.hpp"
#include "models/light_effect.hpp"
#include "models/light_segment.hpp"

// Forward declaration for OSC handler
class OSCHandler;

/**
 * SceneManager manages multiple LightScene instances and handles transitions between them.
 * 
 * This class provides high-level scene management including:
 * - Scene switching with transition effects
 * - Cross-scene effect and palette transitions
 * - LED output with transition opacity control
 * - Scene persistence (save/load from JSON)
 * - Integration with OSC communication
 */
class SceneManager {
public:
    /**
     * Initialize a SceneManager instance.
     */
    SceneManager();

    /**
     * Add a LightScene to the manager.
     * 
     * @param scene_ID Unique identifier for the scene
     * @param scene LightScene instance to add
     */
    void add_scene(int scene_ID, std::shared_ptr<LightScene> scene);

    /**
     * Remove a LightScene from the manager.
     * 
     * @param scene_ID ID of the scene to remove
     */
    void remove_scene(int scene_ID);

    /**
     * Switch to a different LightScene.
     * 
     * @param scene_ID ID of the scene to switch to
     */
    void switch_scene(int scene_ID);

    /**
     * Set transition parameters for scene, effect, or palette transitions.
     * 
     * @param next_scene_idx Next scene ID (optional)
     * @param next_effect_idx Next effect ID (optional)
     * @param next_palette_idx Next palette ID (optional)
     * @param fade_in_time Fade in time in seconds
     * @param fade_out_time Fade out time in seconds
     */
    void set_transition_params(std::optional<int> next_scene_idx = std::nullopt,
                              std::optional<int> next_effect_idx = std::nullopt,
                              std::optional<std::string> next_palette_idx = std::nullopt,
                              float fade_in_time = 0.0f,
                              float fade_out_time = 0.0f);

    /**
     * Update the current scene and handle transitions.
     * This method should be called regularly in the main loop.
     */
    void update();

    /**
     * Get the LED output from the current scene with transition effects applied.
     * 
     * @return List of RGB color values for each LED, modified by transition opacity
     */
    std::vector<std::vector<int>> get_led_output();

    /**
     * Save all scenes to a JSON file.
     * 
     * @param file_path Path to save the JSON file
     */
    void save_scenes_to_json(const std::string& file_path);

    /**
     * Load scenes from a JSON file.
     * 
     * @param file_path Path to the JSON file
     * @return true if loading succeeded, false otherwise
     */
    bool load_scenes_from_json(const std::string& file_path);

    /**
     * Create a new scene with default effect and segment.
     * 
     * @param scene_ID ID for the new scene (optional, auto-generated if not provided)
     * @return ID of the created scene
     */
    int create_new_scene(std::optional<int> scene_ID = std::nullopt);

    // Getters for scene management attributes
    const std::map<int, std::shared_ptr<LightScene>>& get_scenes() const;
    std::optional<int> get_current_scene() const;
    bool get_is_transitioning() const;
    float get_fade_in_time() const;
    float get_fade_out_time() const;
    std::optional<int> get_next_scene_idx() const;
    std::optional<int> get_next_effect_idx() const;
    std::optional<std::string> get_next_palette_idx() const;
    float get_transition_opacity() const;
    float get_transition_start_time() const;

private:
    // Scene management
    std::map<int, std::shared_ptr<LightScene>> scenes;
    std::optional<int> current_scene;

    // Transition parameters
    std::optional<int> next_scene_idx;
    std::optional<int> next_effect_idx;
    std::optional<std::string> next_palette_idx;
    float fade_in_time;
    float fade_out_time;
    float transition_start_time;
    bool is_transitioning;
    float transition_opacity;

    // OSC integration
    OSCHandler* osc_handler;

    /**
     * Helper method to get the next available scene ID.
     * 
     * @return Next available scene ID
     */
    int get_next_available_scene_id() const;
};

#endif // SCENE_MANAGER_HPP