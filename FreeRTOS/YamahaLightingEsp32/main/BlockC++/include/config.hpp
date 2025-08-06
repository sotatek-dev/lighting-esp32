#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>

// System constants
constexpr int DEFAULT_FPS = 60;
const std::vector<int> DEFAULT_LED_SEP_COUNT = {205, 0};
const int DEFAULT_LED_COUNT = DEFAULT_LED_SEP_COUNT[0] + DEFAULT_LED_SEP_COUNT[1];
constexpr int MAX_SEGMENTS = 30;

// LED Binary output configuration
const std::string LED_BINARY_OSC_ADDRESS = "/light/serial";

// Color palettes
using ColorPalette = std::vector<std::vector<int>>;
using PaletteMap = std::map<std::string, ColorPalette>;

extern const PaletteMap DEFAULT_COLOR_PALETTES;

// Default segment parameters
const std::vector<float> DEFAULT_TRANSPARENCY = {1.0f, 1.0f, 1.0f, 1.0f};
const std::vector<int> DEFAULT_LENGTH = {1, 0, 0};
constexpr float DEFAULT_MOVE_SPEED = 0.0f;
const std::vector<int> DEFAULT_MOVE_RANGE = {0, DEFAULT_LED_COUNT - 1};
constexpr int DEFAULT_INITIAL_POSITION = 0;
constexpr bool DEFAULT_IS_EDGE_REFLECT = true;
const std::vector<int> DEFAULT_DIMMER_TIME = {0, 100, 200, 100, 0};
constexpr float DEFAULT_DIMMER_TIME_RATIO = 1.0f;

// Runtime configuration functions
void initialize_palette_cache();
void update_palette_cache(const std::string& palette_id, const ColorPalette& colors);
ColorPalette get_palette(const std::string& palette_id);

#endif // CONFIG_HPP