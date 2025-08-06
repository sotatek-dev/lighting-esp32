#ifndef TYPE_DEFINED_HPP
#define TYPE_DEFINED_HPP

#include <variant>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include "config.hpp"

// BlockC input variant type - for data coming from BlockB
using blockc_input_type = std::variant<
    double,           // Match BlockB's double precision
    bool, 
    std::string, 
    std::vector<double>,  // Match BlockB's vector<double>
    int, 
    std::vector<int>
>;

// BlockC output variant type - updated to include all types used in the code
using blockc_output_type = std::variant<
    int,
    float,
    bool,
    std::string,
    std::vector<float>,
    std::vector<int>,
    std::vector<uint8_t>,
    std::map<std::string, std::vector<int>>
>;

// Data input structure for BlockC (from BlockB output)
typedef struct
{
    std::string address;
    blockc_input_type data;
} BlockCInputData;

// Data output structure for BlockC (to hardware/serial)
typedef struct
{
    std::string address;
    blockc_output_type payload;
} BlockCOutputData;

// Legacy aliases for backward compatibility
using blockc_any_type = blockc_output_type;
using any_type_dict = std::map<std::string, blockc_any_type>;                              // type1
using nested_dict = std::map<std::string, any_type_dict>;                           // type2
using effect_dict = std::map<std::string, std::variant<blockc_any_type, nested_dict>>;     // type3

#endif // TYPE_DEFINED_HPP