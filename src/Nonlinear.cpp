#include "Nonlinear.hpp"
#include <iostream>
#include <cmath>

double cosinorCalculator(double value, double period, double phaseOffset) {
    double calcValue = std::cos((value / period) * 2 * M_PI + phaseOffset);
    return calcValue;
}

double sinorCalculator(double value, double period, double phaseOffset) {
    double calcValue = std::sin((value / period) * 2 * M_PI + phaseOffset);
    return calcValue;
}


int get_nonlinear_expansion_size(const std::string& nl_function) {
    if (nl_function == "cosinor") return 4;  // tod_sin, tod_cos, toy_sin, toy_cos
    if (nl_function == "sinor")   return 2;  // tod_sin, toy_sin
    if (nl_function == "tod_cosinor" || nl_function == "toy_cosinor") return 1;
    if (nl_function == "sincos") return 2;  // sin + cos pair
    // default: single-column transform
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

    if (nl_function == "sincos") {
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
        names = {"tod_sin", "tod_cos", "toy_sin", "toy_cos"};
        return;
    }
    if (nl_function == "sinor") {
        names = {"tod_sin", "toy_sin"};
        return;
    }
    if (nl_function == "tod_cosinor") { names = {"tod_cos"}; return; }
    if (nl_function == "toy_cosinor") { names = {"toy_cos"}; return; }
    if (nl_function == "sincos") {
        names.resize(2);
        names[0] = base_name + "_sin";
        names[1] = base_name + "_cos";
    } else {
        names.resize(1);
        names[0] = base_name + "_" + nl_function;
    }
}

Eigen::MatrixXd get_cosinor_basis(
    const Eigen::Ref<const Eigen::ArrayXd>& tod,
    const Eigen::Ref<const Eigen::ArrayXd>& toy,
    const std::string& nl_function) {

    const double TWO_PI = 2.0 * M_PI;
    int n = tod.size();
    Eigen::ArrayXd angle_tod = (tod / 24.0)  * TWO_PI;
    Eigen::ArrayXd angle_toy = (toy / 365.0) * TWO_PI;

    if (nl_function == "cosinor") {
        Eigen::MatrixXd basis(n, 4);
        basis.col(0) = angle_tod.sin().matrix();  // tod_sin
        basis.col(1) = angle_tod.cos().matrix();  // tod_cos
        basis.col(2) = angle_toy.sin().matrix();  // toy_sin
        basis.col(3) = angle_toy.cos().matrix();  // toy_cos
        return basis;
    } else {  // sinor
        Eigen::MatrixXd basis(n, 2);
        basis.col(0) = angle_tod.sin().matrix();  // tod_sin
        basis.col(1) = angle_toy.sin().matrix();  // toy_sin
        return basis;
    }
}

double calculateNonlinear(std::string function, double value, double period, double phaseOffset, bool nonlinear_in_degrees) {
    double result;
    if      (function == "sin")         result = sinorCalculator(value, period, phaseOffset);
    else if (function == "cos")         result = cosinorCalculator(value, period, phaseOffset);
    else if (function == "tod_cosinor") result = cosinorCalculator(value, 24.0, phaseOffset);
    else if (function == "toy_cosinor") result = cosinorCalculator(value, 365.0, phaseOffset);
    else if (function == "tan")         result = std::tan(value);
    else {
        std::cerr << "Unknown nonlinear function: " << function << std::endl;
        result = std::cos(value);
    }
    return nonlinear_in_degrees ? result * (180.0 / M_PI) : result;
}


