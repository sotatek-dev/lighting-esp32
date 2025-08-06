#include "dimmer_time_adaptor.hpp"
#include <cmath>

DimmerTimeAdaptor::DimmerTimeAdaptor() : current_period_percentage(100) {}

float DimmerTimeAdaptor::calc_feature(double tempo) const {
    if (tempo <= 0) {
        return 100;
    }
    if (tempo < 60) {
        return 500;
    }
    else if (tempo < 80) {
        return 300;
    }
    else if (tempo < 100) {
        return 200;
    }
    else if (tempo < 120) {
        return 100;
    }
    else if (tempo < 140) {
        return 60;
    }
    else {
        return 30;
    }
}

void DimmerTimeAdaptor::update_dimmer_time(double tempo) {
    current_period_percentage = calc_feature(tempo);
}

int DimmerTimeAdaptor::get_period_percentage() const {
    return static_cast<int>(std::round(current_period_percentage));
}