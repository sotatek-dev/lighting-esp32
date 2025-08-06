#include "tempo_classifier.hpp"

TempoClassifier::TempoClassifier() : tempo_class("mid") {}

std::string TempoClassifier::calc_feature(double tempo) const {
    if (tempo < 60) {
        return "very_slow";  // Changed from 0 to "very_slow"
    } else if (tempo < 80) {
        return "slow";  // Changed from 1 to "slow"
    } else if (tempo < 100) {
        return "mid_slow";  // Changed from 2 to "mid_slow"
    } else if (tempo < 120) {
        return "mid";  // Changed from 3 to "mid"
    } else if (tempo < 140) {
        return "mid_fast";  // Changed from 4 to "mid_fast"
    } else {
        return "fast";  // Changed from 5 to "fast"
    }
}

void TempoClassifier::update_tempo_class(double tempo) {
    tempo_class = calc_feature(tempo);
}

std::string TempoClassifier::get_class() const {
    return tempo_class;
}