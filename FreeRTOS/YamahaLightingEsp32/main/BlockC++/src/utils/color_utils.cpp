#include "utils/color_utils.hpp"
#include <algorithm>
#include <numeric>

namespace color_utils {

std::vector<int> interpolate_colors(const std::vector<int>& color1,
                                  const std::vector<int>& color2,
                                  float factor) {
    int r = static_cast<int>(color1[0] + (color2[0] - color1[0]) * factor);
    int g = static_cast<int>(color1[1] + (color2[1] - color1[1]) * factor);
    int b = static_cast<int>(color1[2] + (color2[2] - color1[2]) * factor);

    return {
        std::max(0, std::min(255, r)),
        std::max(0, std::min(255, g)),
        std::max(0, std::min(255, b))
    };
}

std::vector<int> apply_transparency(const std::vector<int>& base_color,
                                  const std::vector<int>& overlay_color,
                                  float transparency) {
    return interpolate_colors(base_color, overlay_color, transparency);
}

std::vector<int> blend_colors(const std::vector<std::vector<int>>& colors,
                            const std::vector<float>& weights) {
    if (colors.empty() || weights.empty() || colors.size() != weights.size()) {
        return {0, 0, 0};
    }

    float total_weight = std::accumulate(weights.begin(), weights.end(), 0.0);
    if (total_weight == 0) {
        return {0, 0, 0};
    }

    std::vector<float> normalized_weights;
    normalized_weights.reserve(weights.size());
    for (float w : weights) {
        normalized_weights.push_back(w / total_weight);
    }

    int r = static_cast<int>(std::inner_product(
        colors.begin(), colors.end(),
        normalized_weights.begin(), 0.0,
        std::plus<>(),
        [](const std::vector<int>& c, float w) { return c[0] * w; }
    ));
    int g = static_cast<int>(std::inner_product(
        colors.begin(), colors.end(),
        normalized_weights.begin(), 0.0,
        std::plus<>(),
        [](const std::vector<int>& c, float w) { return c[1] * w; }
    ));
    int b = static_cast<int>(std::inner_product(
        colors.begin(), colors.end(),
        normalized_weights.begin(), 0.0,
        std::plus<>(),
        [](const std::vector<int>& c, float w) { return c[2] * w; }
    ));

    return {
        std::max(0, std::min(255, r)),
        std::max(0, std::min(255, g)),
        std::max(0, std::min(255, b))
    };
}

std::vector<int> apply_brightness(const std::vector<int>& color,
                                float brightness) {
    int r = static_cast<int>(color[0] * brightness);
    int g = static_cast<int>(color[1] * brightness);
    int b = static_cast<int>(color[2] * brightness);
    return {
        std::max(0, std::min(255, r)),
        std::max(0, std::min(255, g)),
        std::max(0, std::min(255, b))
    };
}

std::vector<int> get_color_from_palette(
    const std::map<std::string, std::vector<std::vector<int>>>& palette,
    const std::string& palette_name,
    int color_index) {
    
    auto it = palette.find(palette_name);
    if (it == palette.end()) {
        return {0, 0, 0};
    }

    const auto& palette_colors = it->second;
    if (color_index < 0 || color_index >= static_cast<int>(palette_colors.size())) {
        return {0, 0, 0};
    }

    return palette_colors[color_index];
}

} // namespace color_utils