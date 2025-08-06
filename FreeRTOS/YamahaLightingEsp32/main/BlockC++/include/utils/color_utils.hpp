#ifndef COLOR_UTILS_HPP
#define COLOR_UTILS_HPP

#include <vector>
#include <map>
#include <string>

/**
 * Utility functions for color manipulation and processing.
 * These functions handle color interpolation, blending, transparency, and brightness adjustments.
 */
namespace color_utils {

    /**
     * Interpolate between two RGB colors.
     * 
     * @param color1 First RGB color [r, g, b]
     * @param color2 Second RGB color [r, g, b]
     * @param factor Interpolation factor (0.0 = color1, 1.0 = color2)
     * @return Interpolated RGB color [r, g, b]
     */
    std::vector<int> interpolate_colors(const std::vector<int>& color1, 
                                      const std::vector<int>& color2, 
                                      float factor);  // Changed from float

    /**
     * Apply a transparent overlay color to a base color.
     * 
     * @param base_color Base RGB color [r, g, b]
     * @param overlay_color Overlay RGB color [r, g, b]
     * @param transparency Transparency of the overlay (0.0 = fully transparent, 1.0 = fully opaque)
     * @return Resulting RGB color [r, g, b]
     */
    std::vector<int> apply_transparency(const std::vector<int>& base_color,
                                      const std::vector<int>& overlay_color,
                                      float transparency);  // Changed from float

    /**
     * Blend multiple colors based on weights.
     * 
     * @param colors List of RGB colors [[r, g, b], ...]
     * @param weights List of weights for each color [w1, w2, ...]
     * @return Blended RGB color [r, g, b]
     */
    std::vector<int> blend_colors(const std::vector<std::vector<int>>& colors,
                                const std::vector<float>& weights);

    /**
     * Apply brightness factor to color.
     * 
     * @param color RGB color [r, g, b]
     * @param brightness Brightness factor (0.0-1.0)
     * @return Resulting RGB color [r, g, b]
     */
    std::vector<int> apply_brightness(const std::vector<int>& color,
                                float brightness);  // Changed from float

    /**
     * Get a color from a palette by name and index.
     * 
     * @param palette Dictionary of color palettes
     * @param palette_name Name of the palette (A, B, C, etc.)
     * @param color_index Index of the color in the palette (0-5)
     * @return RGB color [r, g, b]
     */
    std::vector<int> get_color_from_palette(
        const std::map<std::string, std::vector<std::vector<int>>>& palette,
        const std::string& palette_name,
        int color_index);
}

#endif // COLOR_UTILS_HPP