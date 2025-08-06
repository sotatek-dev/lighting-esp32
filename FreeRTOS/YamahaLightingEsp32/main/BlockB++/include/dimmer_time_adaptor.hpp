#ifndef DIMMER_TIME_ADAPTOR_HPP
#define DIMMER_TIME_ADAPTOR_HPP

class DimmerTimeAdaptor {
public:
    // Constructor initializes with default period percentage
    DimmerTimeAdaptor();
    
    // Calculate appropriate dimmer time percentage based on tempo
    // Returns a float representing period percentage
    float calc_feature(double tempo) const;
    
    // Update the internal period percentage based on tempo
    void update_dimmer_time(double tempo);
    
    // Get the current period percentage rounded to integer
    int get_period_percentage() const;

private:
    float current_period_percentage;
};

#endif // DIMMER_TIME_ADAPTOR_HPP