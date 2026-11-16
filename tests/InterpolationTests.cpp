#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Interpolation.h"
#include "InterpolationExceptions.h"
#include "InterpolationTask.h"

namespace
{
const double NaN = std::numeric_limits<double>::quiet_NaN();
const double Inf = std::numeric_limits<double>::infinity();

InterpolationTask makeSquares()
{
    return InterpolationTask({0.0, 1.0, 4.0, 9.0}, {0.0, 1.0, 16.0, 81.0});
}

// Checks the class invariants.
void expectInvariants(const InterpolationTask& t)
{
    ASSERT_EQ(t.xs().size(), t.ys().size());
    ASSERT_GE(t.size(), 1u);
    ASSERT_LE(t.size(), InterpolationTask::MAX_NODES);
    for (std::size_t i = 0; i < t.size(); ++i)
    {
        EXPECT_TRUE(std::isfinite(t.xs()[i]));
        EXPECT_TRUE(std::isfinite(t.ys()[i]));
        if (i > 0)
        {
            EXPECT_LT(t.xs()[i - 1], t.xs()[i]);
        }
    }
}
}

// ---------- normal operation ----------

TEST(Construction, SortsNodesByAbscissa)
{
    InterpolationTask t({2.0, 0.0, 1.0}, {8.0, 0.0, 1.0});
    EXPECT_EQ(t.size(), 3u);
    EXPECT_DOUBLE_EQ(t.xs()[0], 0.0);
    EXPECT_DOUBLE_EQ(t.ys()[2], 8.0);
    expectInvariants(t);
}

TEST(Operations, LagrangeAtInnerPoint)
{
    InterpolationTask t({0.0, 1.0, 2.0}, {0.0, 1.0, 4.0});   // y = x^2
    EXPECT_NEAR(t.lagrange(1.5), 2.25, 1e-12);
}

TEST(Operations, LagrangeAtNodeReturnsOrdinate)
{
    InterpolationTask t = makeSquares();
    EXPECT_NEAR(t.lagrange(4.0), 16.0, 1e-12);
}

TEST(Operations, LagrangeExtrapolates)
{
    InterpolationTask t({0.0, 1.0, 2.0}, {0.0, 1.0, 4.0});
    EXPECT_NEAR(t.lagrange(4.0), 16.0, 1e-9);
}

TEST(Operations, LinearInterpolationInsideSegment)
{
    InterpolationTask t({0.0, 1.0, 4.0, 9.0}, {0.0, 1.0, 16.0, 81.0});
    EXPECT_NEAR(t.interpolate(0.5), 0.5, 1e-12);
    EXPECT_NEAR(t.interpolate(2.5), 8.5, 1e-12);
}

TEST(Operations, InterpolationErrorValue)
{
    InterpolationTask t({0.0, 1.0, 2.0}, {0.0, 1.0, 8.0});   // y = x^3 samples
    EXPECT_NEAR(t.lagrange(1.5), 3.75, 1e-12);
    EXPECT_NEAR(t.error(1.5, 3.375), 0.375, 1e-12);
}

TEST(Operations, SingleNodeInterpolation)
{
    InterpolationTask t({3.0}, {7.0});
    EXPECT_NEAR(t.interpolate(3.0), 7.0, 1e-12);
    EXPECT_NEAR(t.lagrange(10.0), 7.0, 1e-12);
}

TEST(Operations, LinearInterpolationFunction)
{
    EXPECT_NEAR(linearInterpolation(0.0, 0.0, 2.0, 4.0, 1.0), 2.0, 1e-12);
}

// ---------- boundary values ----------

TEST(Boundary, InterpolateAtLeftAndRightEnd)
{
    InterpolationTask t = makeSquares();
    EXPECT_NO_THROW(t.interpolate(t.minX()));
    EXPECT_NO_THROW(t.interpolate(t.maxX()));
    EXPECT_NEAR(t.interpolate(t.maxX()), 81.0, 1e-12);
}

TEST(Boundary, ExactlyMaxNodesAllowed)
{
    std::vector<double> xs(InterpolationTask::MAX_NODES), ys(InterpolationTask::MAX_NODES, 1.0);
    for (std::size_t i = 0; i < xs.size(); ++i) xs[i] = static_cast<double>(i);
    EXPECT_NO_THROW(InterpolationTask(xs, ys));
}

// ---------- exceptions ----------

TEST(Errors, EmptyNodeSet)
{
    EXPECT_THROW(InterpolationTask({}, {}), EmptyNodeSetException);
}

TEST(Errors, DuplicateAbscissas)
{
    EXPECT_THROW(InterpolationTask({1.0, 2.0, 1.0}, {1.0, 2.0, 3.0}), DuplicateAbscissaException);
}

TEST(Errors, DuplicateAbscissaCarriesValue)
{
    try
    {
        InterpolationTask t({1.0, 2.0, 2.0}, {1.0, 2.0, 3.0});
        FAIL() << "exception expected";
    }
    catch (const DuplicateAbscissaException& ex)
    {
        EXPECT_DOUBLE_EQ(ex.x(), 2.0);
    }
}

TEST(Errors, NaNAbscissa)
{
    EXPECT_THROW(InterpolationTask({0.0, NaN}, {1.0, 2.0}), InvalidNodeException);
}

TEST(Errors, InfiniteOrdinateCarriesDiagnostics)
{
    try
    {
        InterpolationTask t({0.0, 1.0}, {1.0, Inf});
        FAIL() << "exception expected";
    }
    catch (const InvalidNodeException& ex)
    {
        EXPECT_EQ(ex.parameter(), "y");
        EXPECT_EQ(ex.index(), 1u);
    }
}

TEST(Errors, TooManyNodes)
{
    const std::size_t n = InterpolationTask::MAX_NODES + 1;
    EXPECT_THROW(InterpolationTask(std::vector<double>(n, 0.0), std::vector<double>(n, 0.0)),
                 NodeCountException);
}

TEST(Errors, DifferentSizes)
{
    EXPECT_THROW(InterpolationTask({0.0, 1.0}, {1.0}), std::invalid_argument);
}

TEST(Errors, PointOutsideRangeLeft)
{
    InterpolationTask t = makeSquares();
    EXPECT_THROW(t.interpolate(-0.001), std::out_of_range);
}

TEST(Errors, PointOutsideRangeRight)
{
    InterpolationTask t = makeSquares();
    EXPECT_THROW(t.interpolate(9.001), std::out_of_range);
}

TEST(Errors, NaNQueryPoint)
{
    InterpolationTask t = makeSquares();
    EXPECT_THROW(t.lagrange(NaN), std::invalid_argument);
    EXPECT_THROW(t.interpolate(NaN), std::invalid_argument);
}

TEST(Errors, LinearInterpolationEqualAbscissas)
{
    EXPECT_THROW(linearInterpolation(1.0, 0.0, 1.0, 5.0, 1.0), DuplicateAbscissaException);
}

TEST(Hierarchy, CatchableAsBaseClasses)
{
    EXPECT_THROW(InterpolationTask({}, {}), InterpolationException);
    EXPECT_THROW(InterpolationTask({}, {}), std::runtime_error);
    EXPECT_THROW(InterpolationTask({}, {}), std::exception);
}

// ---------- invariants and strong guarantee ----------

TEST(Invariants, HoldAfterSuccessfulOperations)
{
    InterpolationTask t = makeSquares();
    t.addNode(2.0, 4.0);
    expectInvariants(t);
    t.replaceNodes({5.0, 3.0, 1.0}, {25.0, 9.0, 1.0});
    expectInvariants(t);
    EXPECT_EQ(t.size(), 3u);
}

TEST(StrongGuarantee, ReplaceNodesDuplicatesKeepsState)
{
    InterpolationTask t = makeSquares();
    const auto xs = t.xs();
    const auto ys = t.ys();
    EXPECT_THROW(t.replaceNodes({1.0, 1.0}, {1.0, 2.0}), DuplicateAbscissaException);
    EXPECT_EQ(t.xs(), xs);
    EXPECT_EQ(t.ys(), ys);
}

TEST(StrongGuarantee, ReplaceNodesEmptyKeepsState)
{
    InterpolationTask t = makeSquares();
    const auto xs = t.xs();
    EXPECT_THROW(t.replaceNodes({}, {}), EmptyNodeSetException);
    EXPECT_EQ(t.xs(), xs);
}

TEST(StrongGuarantee, ReplaceNodesNaNKeepsState)
{
    InterpolationTask t = makeSquares();
    const auto xs = t.xs();
    const auto ys = t.ys();
    EXPECT_THROW(t.replaceNodes({0.0, 1.0, 2.0}, {1.0, NaN, 3.0}), InvalidNodeException);
    EXPECT_EQ(t.xs(), xs);
    EXPECT_EQ(t.ys(), ys);
}

TEST(StrongGuarantee, ReplaceNodesSizeMismatchKeepsState)
{
    InterpolationTask t = makeSquares();
    const auto xs = t.xs();
    EXPECT_THROW(t.replaceNodes({0.0, 1.0}, {1.0}), std::invalid_argument);
    EXPECT_EQ(t.xs(), xs);
}

TEST(StrongGuarantee, AddNodeDuplicateKeepsState)
{
    InterpolationTask t = makeSquares();
    const auto xs = t.xs();
    const auto ys = t.ys();
    EXPECT_THROW(t.addNode(4.0, 0.0), DuplicateAbscissaException);
    EXPECT_EQ(t.xs(), xs);
    EXPECT_EQ(t.ys(), ys);
}

TEST(StrongGuarantee, AddNodeNaNKeepsState)
{
    InterpolationTask t = makeSquares();
    const auto xs = t.xs();
    EXPECT_THROW(t.addNode(NaN, 1.0), InvalidNodeException);
    EXPECT_EQ(t.xs(), xs);
}

TEST(StrongGuarantee, AddNodeOverLimitKeepsState)
{
    std::vector<double> xs(InterpolationTask::MAX_NODES), ys(InterpolationTask::MAX_NODES, 0.0);
    for (std::size_t i = 0; i < xs.size(); ++i) xs[i] = static_cast<double>(i);
    InterpolationTask t(xs, ys);
    EXPECT_THROW(t.addNode(5000.0, 0.0), NodeCountException);
    EXPECT_EQ(t.size(), InterpolationTask::MAX_NODES);
}
