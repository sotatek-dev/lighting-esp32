#ifndef DATA_DISPATCHER_HPP
#define DATA_DISPATCHER_HPP

#include <regex>
#include <string>
#include <functional>
#include <vector>
#include <utility>
#include <optional>
#include "blockc_types.hpp"

typedef struct {
    std::string regex_name;
    std::smatch matches;
} MatchResult;

class DataDispatcher {
public:
    DataDispatcher();
    std::optional<MatchResult> match_pattern(const BlockCInputData& data);

private:
    std::vector<std::pair<std::regex, std::string>> regex_patterns;
};

#endif  //DATA_DISPATCHER_HPP