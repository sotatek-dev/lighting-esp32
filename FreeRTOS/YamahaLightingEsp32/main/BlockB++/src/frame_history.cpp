#include "frame_history.hpp"

FrameHistory::FrameHistory(size_t maxlen) : maxlen(maxlen) {}

void FrameHistory::append(const std::string &key, const blockb_any_type &value) {
    if (data.find(key) == data.end()) {
        data[key] = std::deque<blockb_any_type>();
    }
    
    data[key].push_back(value);
    
    if (data[key].size() > maxlen) {
        data[key].pop_front();
    }
}

std::vector<blockb_any_type> FrameHistory::get(const std::string &key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return {};  // Return empty vector
    }
    // Create and return a copy of the data
    return std::vector<blockb_any_type>(it->second.begin(), it->second.end());
}

size_t FrameHistory::size() const {
    if (!data.empty()) {
        auto it = data.begin();
        return it->second.size();
    }
    return 0;
}

std::vector<std::string> FrameHistory::keys() const {
    std::vector<std::string> result;
    for (const auto &pair : data) {
        result.push_back(pair.first);
    }
    return result;
}