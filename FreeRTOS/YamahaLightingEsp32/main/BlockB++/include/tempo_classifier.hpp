#ifndef TEMPO_CLASSIFIER_HPP
#define TEMPO_CLASSIFIER_HPP

#include <string>

class TempoClassifier {
public:
    // Constructor initializes with default tempo class
    TempoClassifier();

    // Calculate tempo class based on tempo value
    // Returns a string representation of the tempo class
    std::string calc_feature(double tempo) const;

    // Update the internal tempo class
    void update_tempo_class(double tempo);

    // Get the current tempo class
    std::string get_class() const;

private:
    std::string tempo_class;
};

#endif // TEMPO_CLASSIFIER_HPP