#include "Nonlinear.hpp"
#include <iostream>
#include <unordered_map>
#include <functional>
#include <cmath>

double cosinorCalculator(double value, double period, double phaseOffset) {
    double calcValue = std::cos((value / period) * 2 * M_PI + phaseOffset);
    return calcValue;
}

double sinorCalculator(double value, double period, double phaseOffset) {
    double calcValue = std::sin((value / period) * 2 * M_PI + phaseOffset);
    return calcValue;
}

double calculateNonlinear(std::string function, double value, double period, double phaseOffset, bool nonlinear_in_degrees) {
    if (nonlinear_in_degrees) {
        if (function == "sin") {
            return (sinorCalculator(value, period, phaseOffset)*(180/(M_PI)));
        } else if (function == "cos") {
            return (cosinorCalculator(value, period, phaseOffset)*(180/(M_PI)));
        } else if (function == "tan") {
            return (std::tan(value)*(180/(M_PI)));
        } else {
            std::cerr << "Unknown function, doing simple cos: " << function << std::endl;
            return std::cos(value);
        }
    } else {
        if (function == "sin") {
            return sinorCalculator(value, period, phaseOffset);
        } else if (function == "cos") {
            return cosinorCalculator(value, period, phaseOffset);
        } else if (function == "tan") {
            return std::tan(value);
        } else {
            std::cerr << "Unknown function, doing simple cos: " << function << std::endl;
            return std::cos(value);
        }
    } 

}


