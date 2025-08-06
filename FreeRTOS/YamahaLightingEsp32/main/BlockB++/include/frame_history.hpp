#ifndef FRAME_HISTORY_HPP
#define FRAME_HISTORY_HPP

#include <deque>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include "blockb_types.hpp"
class FrameHistory {
public:
    explicit FrameHistory(size_t maxlen = 200);
    
    void append(const std::string &key, const blockb_any_type &value);
    std::vector<blockb_any_type> get(const std::string &key) const;
    size_t size() const;
    std::vector<std::string> keys() const;

private:
    size_t maxlen;
    std::map<std::string, std::deque<blockb_any_type>> data;
};

#endif // FRAME_HISTORY_HPP