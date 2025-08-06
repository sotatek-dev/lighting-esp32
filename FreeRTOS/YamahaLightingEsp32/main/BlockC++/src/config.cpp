#include "config.hpp"

// Static storage for the runtime cache
static PaletteMap RUNTIME_PALETTE_CACHE;

// Define the default color palettes
PaletteMap create_default_palettes() {
    return PaletteMap{
        {"A", {
            {255, 0, 0},    // Red
            {0, 255, 0},    // Green
            {0, 0, 255},    // Blue
            {255, 255, 0},  // Yellow
            {0, 255, 255},  // Cyan
            {255, 0, 255}   // Magenta
        }},
        {"B", {
            {255, 128, 0},  // Orange
            {128, 0, 255},  // Purple
            {0, 128, 255},  // Sky Blue
            {255, 0, 128},  // Pink
            {128, 255, 0},  // Lime
            {255, 255, 255} // White
        }},
        {"C", {
            {128, 0, 0},    // Dark Red
            {0, 128, 0},    // Dark Green
            {0, 0, 128},    // Dark Blue
            {128, 128, 0},  // Olive
            {0, 128, 128},  // Teal
            {128, 0, 128}   // Purple
        }},
        {"D", {
            {255, 200, 200},  // Light Pink
            {200, 255, 200},  // Light Green
            {200, 200, 255},  // Light Blue
            {255, 255, 200},  // Light Yellow
            {200, 255, 255},  // Light Cyan
            {255, 200, 255}   // Light Magenta
        }},
        {"E", {
            {100, 100, 100},  // Dark Gray
            {150, 150, 150},  // Medium Gray
            {200, 200, 200},  // Light Gray
            {255, 100, 50},   // Coral
            {50, 100, 255},   // Royal Blue
            {150, 255, 150}   // Light Green
        }}
    };
}

// Define a static variable to hold the palettes
static const PaletteMap DEFAULT_PALETTES = create_default_palettes();

// Define the extern variable to reference the static variable
const PaletteMap DEFAULT_COLOR_PALETTES = DEFAULT_PALETTES;

void initialize_palette_cache() {
    RUNTIME_PALETTE_CACHE = DEFAULT_COLOR_PALETTES;  // Map copy is equivalent to deepcopy
}

void update_palette_cache(const std::string& palette_id, const ColorPalette& colors) {
    RUNTIME_PALETTE_CACHE[palette_id] = colors;  // Vector copy is equivalent to deepcopy
}

ColorPalette get_palette(const std::string& palette_id) {
    if (RUNTIME_PALETTE_CACHE.find(palette_id) != RUNTIME_PALETTE_CACHE.end()) {
        return RUNTIME_PALETTE_CACHE[palette_id];
    }
    else if (DEFAULT_COLOR_PALETTES.find(palette_id) != DEFAULT_COLOR_PALETTES.end()) {
        return DEFAULT_COLOR_PALETTES.at(palette_id);
    }
    return DEFAULT_COLOR_PALETTES.at("A");
}
