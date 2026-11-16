#pragma once

#include <vector>

// Algorithms of the library. Nodes (xs, ys) must have equal size.
// Throws: std::invalid_argument for inconsistent arguments or NaN query point,
//         DuplicateAbscissaException if two abscissas coincide.

// Lagrange polynomial value at x (extrapolation is allowed).
double lagrange(const std::vector<double>& xs, const std::vector<double>& ys, double x);

// Value of the straight line through (x0, y0) and (x1, y1) at x.
double linearInterpolation(double x0, double y0, double x1, double y1, double x);

// Piecewise linear interpolation, xs must be sorted in ascending order.
// Throws std::out_of_range if x is outside [xs.front(), xs.back()].
double interpolate(const std::vector<double>& xs, const std::vector<double>& ys, double x);

// |trueValue - lagrange(x)|
double interpolationError(const std::vector<double>& xs, const std::vector<double>& ys,
                          double x, double trueValue);
