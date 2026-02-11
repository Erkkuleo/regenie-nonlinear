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

double invsinCalculator(double value, double period, double phaseOffset) {
    double calcValue = std::asin((std::asin(sqrt(value)) / period) * 2 * M_PI + phaseOffset);
    // take a look at the input values. They mater for the result as for sin we use radians and asine is degreeses
    // maybe a new mode for these 
    //arcsine(sqrt(x)) and log(p/(1-p)) which are the proportion variance stabilization transforms
    //That should check before running that the values are ALMOST ALL between 0 and 1
    //99% between 0 and 1, and 99.99% between -0.01 and 1.01
    return calcValue;
}

double invcosCalculator(double value, double period, double phaseOffset) {
    double calcValue = std::acos((value / period) * 2 * M_PI + phaseOffset);
    return calcValue;
}

double calculateNonlinear(std::string function, double value, double period, double phaseOffset, bool nonlinear_in_degrees) {
    if (nonlinear_in_degrees) {
        if (function == "sinor") {
            return (sinorCalculator(value, period, phaseOffset)*(180/(M_PI)));
            // throw a warning running in degreeses doesnt change the value
        } else if (function == "cosinor") {
            return (cosinorCalculator(value, period, phaseOffset)*(180/(M_PI)));

        } else if (function == "invsin") {
            return (invsinCalculator(value, period, phaseOffset)*(180/(M_PI)));

        } else if (function == "invcos") {
            return (invcosCalculator(value, period, phaseOffset)*(180/(M_PI)));
        }  else if (function == "tan") {
            return (std::tan(value)*(180/(M_PI)));
        } else {
            std::cerr << "Unknown function, doing simple cos: " << function << std::endl;
            return std::cos(value);
        }
    } else {
        if (function == "sinor") {
            return sinorCalculator(value, period, phaseOffset);
        } else if (function == "cosinor") {
            return cosinorCalculator(value, period, phaseOffset);
        } else if (function == "invsin") {
            return (invsinCalculator(value, period, phaseOffset));
        } else if (function == "invcos") {
            return (invcosCalculator(value, period, phaseOffset));
        } else if (function == "tan") {
            return std::tan(value);
        } else {
            std::cerr << "Unknown function, doing simple cos: " << function << std::endl;
            return std::cos(value);
        }
    } 

}


