#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "InterpolationExceptions.h"
#include "InterpolationTask.h"
#include "ScopeLogger.h"

namespace
{
void level3(const InterpolationTask& task)
{
    ScopeLogger log("level3");
    // x = 100 is outside the node range -> std::out_of_range
    std::cout << task.interpolate(100.0) << '\n';
}

void level2(const InterpolationTask& task)
{
    ScopeLogger log("level2");
    try
    {
        level3(task);
    }
    catch (const std::exception& ex)
    {
        std::cout << "level2: caught \"" << ex.what() << "\", rethrowing\n";
        throw;
    }
}

void level1(const InterpolationTask& task)
{
    ScopeLogger log("level1");
    level2(task);
}

template <typename F>
void scenario(const char* title, F action)
{
    std::cout << "\n=== " << title << " ===\n";
    try
    {
        action();
    }
    catch (const EmptyNodeSetException& ex)
    {
        std::cout << "EmptyNodeSetException: " << ex.what() << '\n';
    }
    catch (const InvalidNodeException& ex)
    {
        std::cout << "InvalidNodeException: " << ex.what() << " (parameter=" << ex.parameter()
                  << ", index=" << ex.index() << ")\n";
    }
    catch (const DuplicateAbscissaException& ex)
    {
        std::cout << "DuplicateAbscissaException: " << ex.what() << " (x=" << ex.x() << ")\n";
    }
    catch (const NodeCountException& ex)
    {
        std::cout << "NodeCountException: " << ex.what() << " (count=" << ex.count() << ")\n";
    }
    catch (const InterpolationException& ex)
    {
        std::cout << "InterpolationException: " << ex.what() << '\n';
    }
    catch (const std::out_of_range& ex)
    {
        std::cout << "std::out_of_range: " << ex.what() << '\n';
    }
    catch (const std::invalid_argument& ex)
    {
        std::cout << "std::invalid_argument: " << ex.what() << '\n';
    }
}
}

int main()
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    std::cout << "##### Part 1: error scenarios #####\n";
    InterpolationTask task({0.0, 1.0, 2.0}, {0.0, 1.0, 8.0});
    std::cout << "lagrange(1.5) = " << task.lagrange(1.5) << ", interpolate(1.5) = "
              << task.interpolate(1.5) << '\n';

    scenario("1. empty node set", [] { InterpolationTask t({}, {}); });
    scenario("2. duplicate abscissas", [] { InterpolationTask t({1.0, 2.0, 1.0}, {1.0, 2.0, 3.0}); });
    scenario("3. NaN in nodes", [&] { InterpolationTask t({0.0, nan}, {1.0, 2.0}); });
    scenario("4. Inf in ordinates", [&] { InterpolationTask t({0.0, 1.0}, {1.0, inf}); });
    scenario("5. point outside the node range", [&] { task.interpolate(5.0); });
    scenario("6. wrong node count", [] {
        InterpolationTask t(std::vector<double>(InterpolationTask::MAX_NODES + 1, 0.0),
                            std::vector<double>(InterpolationTask::MAX_NODES + 1, 0.0));
    });
    scenario("7. different sizes of xs and ys", [] { InterpolationTask t({0.0, 1.0}, {1.0}); });

    std::cout << "\n##### Part 2: strong guarantee #####\n";
    scenario("replaceNodes with duplicates", [&] { task.replaceNodes({1.0, 1.0}, {1.0, 2.0}); });
    std::cout << "nodes after failed replaceNodes: " << task.size() << " (unchanged)\n";
    task.replaceNodes({0.0, 1.0, 4.0, 9.0}, {0.0, 1.0, 16.0, 81.0});
    std::cout << "nodes after successful replaceNodes: " << task.size() << '\n';
    scenario("addNode with existing abscissa", [&] { task.addNode(4.0, 0.0); });
    std::cout << "nodes after failed addNode: " << task.size() << " (unchanged)\n";

    std::cout << "\n##### Part 3: stack unwinding #####\n";
    try
    {
        ScopeLogger log("main");
        level1(task);
        std::cout << "this line is never reached\n";
    }
    catch (const InterpolationException& ex)
    {
        std::cout << "main: InterpolationException: " << ex.what() << '\n';
    }
    catch (const std::out_of_range& ex)
    {
        std::cout << "main: std::out_of_range: " << ex.what() << '\n';
    }
    catch (const std::exception& ex)
    {
        std::cout << "main: std::exception: " << ex.what() << '\n';
    }
    return 0;
}
