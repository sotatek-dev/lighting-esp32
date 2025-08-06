#include "data_dispatcher.hpp"

DataDispatcher::DataDispatcher() {
    regex_patterns = {
        // Only keep patterns for the specified callbacks
        {std::regex(R"(/scene/(\d+)/load_effects)"), "scene_load_effects"},
        {std::regex(R"(/scene/(\d+)/change_effect)"), "scene_change_effect"},
        {std::regex(R"(/scene/(\d+)/change_palette)"), "scene_change_palette"},
        {std::regex(R"(/scene/(\d+)/effect/(\d+)/segment/(\d+)/(.+))"), "scene_effect_segment"}
    };
}

std::optional<MatchResult> DataDispatcher::match_pattern(const BlockCInputData& data) {
    std::smatch matches;
    
    for (const auto& [pattern, name] : regex_patterns) {
        if (std::regex_match(data.address, matches, pattern)) {
            return MatchResult{name, matches};
        }
    }
    
    return std::nullopt; // No pattern matched
}
