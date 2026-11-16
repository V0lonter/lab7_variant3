#include "InterpolationTask.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

#include "Interpolation.h"
#include "InterpolationExceptions.h"

namespace
{
// Validates the nodes and sorts them by abscissa. Works on the caller's local
// copies, so it never touches the state of an existing object.
void normalize(std::vector<double>& xs, std::vector<double>& ys)
{
    if (xs.size() != ys.size())
    {
        throw std::invalid_argument("xs and ys must have the same size");
    }
    if (xs.empty())
    {
        throw EmptyNodeSetException();
    }
    if (xs.size() > InterpolationTask::MAX_NODES)
    {
        throw NodeCountException(xs.size(), 1, InterpolationTask::MAX_NODES);
    }
    for (std::size_t i = 0; i < xs.size(); ++i)
    {
        if (!std::isfinite(xs[i]))
        {
            throw InvalidNodeException("x", i, xs[i]);
        }
        if (!std::isfinite(ys[i]))
        {
            throw InvalidNodeException("y", i, ys[i]);
        }
    }

    std::vector<std::size_t> order(xs.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::sort(order.begin(), order.end(),
              [&xs](std::size_t a, std::size_t b) { return xs[a] < xs[b]; });

    std::vector<double> sortedX(xs.size());
    std::vector<double> sortedY(xs.size());
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        sortedX[i] = xs[order[i]];
        sortedY[i] = ys[order[i]];
        if (i > 0 && sortedX[i] == sortedX[i - 1])
        {
            throw DuplicateAbscissaException(sortedX[i]);
        }
    }
    xs.swap(sortedX);
    ys.swap(sortedY);
}
}

InterpolationTask::InterpolationTask(std::vector<double> xs, std::vector<double> ys)
{
    normalize(xs, ys);
    xs_.swap(xs);
    ys_.swap(ys);
}

std::size_t InterpolationTask::size() const noexcept
{
    return xs_.size();
}

const std::vector<double>& InterpolationTask::xs() const noexcept
{
    return xs_;
}

const std::vector<double>& InterpolationTask::ys() const noexcept
{
    return ys_;
}

double InterpolationTask::minX() const noexcept
{
    return xs_.front();
}

double InterpolationTask::maxX() const noexcept
{
    return xs_.back();
}

double InterpolationTask::lagrange(double x) const
{
    return ::lagrange(xs_, ys_, x);
}

double InterpolationTask::interpolate(double x) const
{
    return ::interpolate(xs_, ys_, x);
}

double InterpolationTask::error(double x, double trueValue) const
{
    return ::interpolationError(xs_, ys_, x, trueValue);
}

// Safety guarantee: strong
void InterpolationTask::replaceNodes(std::vector<double> xs, std::vector<double> ys)
{
    normalize(xs, ys);   // may throw, the object is not modified yet
    xs_.swap(xs);        // noexcept commit
    ys_.swap(ys);
}

// Safety guarantee: strong
void InterpolationTask::addNode(double x, double y)
{
    std::vector<double> newXs = xs_;
    std::vector<double> newYs = ys_;
    newXs.push_back(x);
    newYs.push_back(y);
    normalize(newXs, newYs);
    xs_.swap(newXs);
    ys_.swap(newYs);
}
