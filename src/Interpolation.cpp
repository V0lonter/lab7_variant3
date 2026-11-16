#include "Interpolation.h"

#include <cmath>
#include <stdexcept>

#include "InterpolationExceptions.h"

namespace
{
void checkNodes(const std::vector<double>& xs, const std::vector<double>& ys)
{
    if (xs.size() != ys.size())
    {
        throw std::invalid_argument("xs and ys must have the same size");
    }
    if (xs.empty())
    {
        throw std::invalid_argument("at least one node is required");
    }
}
}

double lagrange(const std::vector<double>& xs, const std::vector<double>& ys, double x)
{
    checkNodes(xs, ys);
    if (std::isnan(x))
    {
        throw std::invalid_argument("query point must not be NaN");
    }
    const std::size_t n = xs.size();
    double result = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        double term = ys[i];
        for (std::size_t j = 0; j < n; ++j)
        {
            if (j == i)
            {
                continue;
            }
            double denom = xs[i] - xs[j];
            if (denom == 0.0)
            {
                throw DuplicateAbscissaException(xs[i]);
            }
            term *= (x - xs[j]) / denom;
        }
        result += term;
    }
    return result;
}

double linearInterpolation(double x0, double y0, double x1, double y1, double x)
{
    if (x1 == x0)
    {
        throw DuplicateAbscissaException(x0);
    }
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

double interpolate(const std::vector<double>& xs, const std::vector<double>& ys, double x)
{
    checkNodes(xs, ys);
    if (std::isnan(x))
    {
        throw std::invalid_argument("query point must not be NaN");
    }
    if (x < xs.front() || x > xs.back())
    {
        throw std::out_of_range("interpolation point is outside the node range");
    }
    const std::size_t n = xs.size();
    if (n == 1)
    {
        return ys[0];
    }
    std::size_t i = 0;
    while (i < n - 2 && x > xs[i + 1])
    {
        ++i;
    }
    return linearInterpolation(xs[i], ys[i], xs[i + 1], ys[i + 1], x);
}

double interpolationError(const std::vector<double>& xs, const std::vector<double>& ys,
                          double x, double trueValue)
{
    return std::fabs(trueValue - lagrange(xs, ys, x));
}
