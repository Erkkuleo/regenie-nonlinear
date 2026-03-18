
#ifndef NONLINEAR_HPP
#define NONLINEAR_HPP

#include <string>
#include <vector>
#include <Eigen/Dense>

double calculateNonlinear(std::string function, double value, double period, double phaseOffset, bool nonlinear_in_degrees);

// Returns number of basis columns for a given nonlinear function type
int get_nonlinear_expansion_size(const std::string& nl_function);

// Expands a raw covariate column into a multi-column basis matrix
Eigen::MatrixXd get_nonlinear_basis(const Eigen::Ref<const Eigen::ArrayXd>& raw_values,
                                     const std::string& nl_function,
                                     double period,
                                     double offset,
                                     bool in_degrees);

// Generates column names for the nonlinear basis expansion
void get_nonlinear_names(std::vector<std::string>& names,
                         const std::string& base_name,
                         const std::string& nl_function);

/// Build interaction matrix from pre-computed tod and toy arrays.
/// nl_function: "cosinor" -> 4 cols (tod_sin, tod_cos, toy_sin, toy_cos)
///              "sinor"   -> 2 cols (tod_sin, toy_sin)
/// tod: time-of-day in hours [0, 24)
/// toy: time-of-year as day-of-year [0, 365)
Eigen::MatrixXd get_cosinor_basis(
    const Eigen::Ref<const Eigen::ArrayXd>& tod,
    const Eigen::Ref<const Eigen::ArrayXd>& toy,
    const std::string& nl_function);

#endif // NONLINEAR_HPP