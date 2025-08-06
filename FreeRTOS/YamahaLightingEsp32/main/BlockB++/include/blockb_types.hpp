#ifndef BLOCKB_TYPES_HPP
#define BLOCKB_TYPES_HPP

#include <variant>
#include <string>
#include <vector>

// BlockB specific variant type - for music analysis data
using blockb_output_type = std::variant<
    double,
    bool, 
    std::string, 
    std::vector<double>,
    int, 
    std::vector<int>
>;

// Music analysis data structure
typedef struct {
    float allpass_dB;
    float LPF200_dB;
    float BPF500_dB;
    float BPF2000_dB;
    float BPF4000_dB;
    float HPF6000_dB;
    int genreID;
    int surround_score;
    int beat;
    float tempo;
    float tempo_confidence;
} MusicAnalyzedData;

// BlockB output data structure (becomes BlockC input)
typedef struct
{
    std::string address;
    blockb_output_type data;
} BlockBOutputData;

// Legacy aliases for backward compatibility
using blockb_any_type = blockb_output_type;  // Keep for existing code

#endif // BLOCKB_TYPES_HPP