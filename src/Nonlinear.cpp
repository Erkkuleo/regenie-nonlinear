#include "Nonlinear.hpp"
#include <iostream>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <stdexcept>

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

int get_nonlinear_expansion_size(const std::string& nl_function) {
    if (nl_function == "cosinor") return 2;  // sin + cos
    // single-transform functions
    if (nl_function == "sinor" || nl_function == "invsin" ||
        nl_function == "invcos" || nl_function == "tan" ||
        nl_function == "cos") return 1;
    // default: assume single transform
    return 1;
}

Eigen::MatrixXd get_nonlinear_basis(const Eigen::Ref<const Eigen::ArrayXd>& raw_values,
                                     const std::string& nl_function,
                                     double period,
                                     double offset,
                                     bool in_degrees) {
    int n = raw_values.size();
    int ncols = get_nonlinear_expansion_size(nl_function);
    Eigen::MatrixXd basis = Eigen::MatrixXd::Zero(n, ncols);
    double deg_factor = in_degrees ? (180.0 / M_PI) : 1.0;

    if (nl_function == "cosinor") {
        for (int i = 0; i < n; i++) {
            double angle = (raw_values(i) / period) * 2.0 * M_PI + offset;
            basis(i, 0) = std::sin(angle) * deg_factor;
            basis(i, 1) = std::cos(angle) * deg_factor;
        }
    } else {
        // Single-column transforms: delegate to calculateNonlinear
        for (int i = 0; i < n; i++) {
            basis(i, 0) = calculateNonlinear(nl_function, raw_values(i), period, offset, in_degrees);
        }
    }

    return basis;
}

void get_nonlinear_names(std::vector<std::string>& names,
                         const std::string& base_name,
                         const std::string& nl_function) {
    if (nl_function == "cosinor") {
        names.resize(2);
        names[0] = base_name + "_sin";
        names[1] = base_name + "_cos";
    } else {
        names.resize(1);
        names[0] = base_name + "_" + nl_function;
    }
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


