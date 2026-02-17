
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

#endif // NONLINEAR_HPP