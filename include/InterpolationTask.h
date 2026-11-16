#pragma once

#include <cstddef>
#include <vector>

// Set of interpolation nodes with controlled invariants.
//
// Class invariants (hold for every accessible object):
//   1. xs.size() == ys.size()
//   2. 1 <= size() <= MAX_NODES
//   3. all coordinates are finite numbers
//   4. abscissas are strictly increasing (sorted, no duplicates)
class InterpolationTask
{
public:
    static constexpr std::size_t MAX_NODES = 1000;

    // Throws: EmptyNodeSetException, NodeCountException, InvalidNodeException,
    //         DuplicateAbscissaException, std::invalid_argument (different sizes).
    InterpolationTask(std::vector<double> xs, std::vector<double> ys);

    std::size_t size() const noexcept;
    const std::vector<double>& xs() const noexcept;
    const std::vector<double>& ys() const noexcept;
    double minX() const noexcept;
    double maxX() const noexcept;

    // const operations, do not change the state
    double lagrange(double x) const;
    double interpolate(double x) const;
    double error(double x, double trueValue) const;

    // Replaces the whole set of nodes. Safety guarantee: STRONG.
    // The new set is validated and sorted in temporary vectors first, the state is
    // changed only by noexcept swap. On any exception the object is unchanged.
    void replaceNodes(std::vector<double> xs, std::vector<double> ys);

    // Adds one node. Safety guarantee: STRONG (copy-and-swap).
    void addNode(double x, double y);

private:
    std::vector<double> xs_;
    std::vector<double> ys_;
};
