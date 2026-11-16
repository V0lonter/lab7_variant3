#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

// Base class of all exceptions of the library.
class InterpolationException : public std::runtime_error
{
public:
    explicit InterpolationException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

// The set of nodes is empty.
class EmptyNodeSetException : public InterpolationException
{
public:
    EmptyNodeSetException()
        : InterpolationException("Node set is empty")
    {
    }
};

// A coordinate of a node is NaN or infinite.
class InvalidNodeException : public InterpolationException
{
private:
    std::string parameter_;
    std::size_t index_;
    double value_;

public:
    InvalidNodeException(const std::string& parameter, std::size_t index, double value)
        : InterpolationException("Invalid node value: " + parameter + "["
                                 + std::to_string(index) + "] is not finite"),
          parameter_(parameter),
          index_(index),
          value_(value)
    {
    }

    const std::string& parameter() const noexcept { return parameter_; }
    std::size_t index() const noexcept { return index_; }
    double value() const noexcept { return value_; }
};

// Two nodes have the same abscissa.
class DuplicateAbscissaException : public InterpolationException
{
private:
    double x_;

public:
    explicit DuplicateAbscissaException(double x)
        : InterpolationException("Duplicate abscissa: " + std::to_string(x)),
          x_(x)
    {
    }

    double x() const noexcept { return x_; }
};

// The number of nodes is outside the allowed range.
class NodeCountException : public InterpolationException
{
private:
    std::size_t count_;
    std::size_t minCount_;
    std::size_t maxCount_;

public:
    NodeCountException(std::size_t count, std::size_t minCount, std::size_t maxCount)
        : InterpolationException("Invalid node count " + std::to_string(count)
                                 + " (allowed " + std::to_string(minCount) + ".."
                                 + std::to_string(maxCount) + ")"),
          count_(count),
          minCount_(minCount),
          maxCount_(maxCount)
    {
    }

    std::size_t count() const noexcept { return count_; }
    std::size_t minCount() const noexcept { return minCount_; }
    std::size_t maxCount() const noexcept { return maxCount_; }
};
