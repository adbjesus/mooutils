# moutils

Library and solver for Multi-Objective Combinatorial Optimization (MOUTILS)
problems focusing on anytime algorithms.

Note: This is still in the early phases of development, so expect things
to change between versions.

## Goals

The goal of the library is to provide a set of algorithms and problem
definitions to solve multi-objective combinatorial problems. The aim is
to design the algorithms and problems according to a zero (or low)
overhead framework in mind to allow the user to easily customize the
provided algorithms and problems, and to define new algorithms and new
problems capable of operating with the provided ones.

Furthermore this projects aims to consider anytime algorithms, so there
should be a set of utilities that can communicate with the algorithm to
decide when an algorithm should be stopped. Since these utilities may
introduce overhead they should be opt-in, and if the user chooses not
use them there should be no overhead.

The solver will serve as an interface to solve instances of the problem
defined in the library with the algorithms provided in the library, and
to allow for some anytime capabilities of the library. Inevitably, the
solver will needs to make some choices in which case the user should
instead use the library directly.

Extensive documentation should be provided for the library and the
solver.

## Roadmap to 1.0

The following lists give the roadmap for a `1.0` release in terms of
algorithm frameworks, problems, and anytime capabilities.

### Algorithm Frameworks

- [ ] Dynamic Programming
- [ ] Branch and Bound
- [ ] Scalarization Techniques
- [ ] Pareto Local Search
- [ ] Evolutionary Algorithms

### Problems

- [ ] Knapsack
- [ ] ρMNK-Landscapes
- [ ] Traveling Salesman

### Anytime Capabilities

The anytime capabilities will be implemented in terms of futures.

- [ ] Functions that run in a separate thread than the algorithm and
      that can set the value of a `void` promise. These can be used to,
      for example, implement a time based interruption, or to have a
      process monitor the system to decide whether to interrupt the
      algorithm or not.
- [ ] A class that has a function that may receive a set of (newly
      considered) objective points and returns a future which in turn
      returns a boolean. This future can be checked to see if the
      algorithm should be stopped or not. The algorithm should send the
      points to this function as it finds them.

## Implemented algorithms and problems

In the rows of the following table we see the algorithm frameworks
considered. In the columns the problems considered. Inside, we have the
particular algorithms implemented for each problem. These may be called
from the library or the solver.

|                          | Knapsack | ρMNK-Landscapes | Traveling Salesman |
|--------------------------|----------|-----------------|--------------------|
| Dynamic Programming      |          |                 |                    |
| Branch and Bound         |          |                 |                    |
| Scalarization Techniques |          |                 |                    |
| Pareto Local Search      |          |                 |                    |
| Evolutionary Algorithms  |          |                 |                    |

## Building

This project uses CMake to generate the build system. As such, to generate the
build files you can do for example

```sh
# Debug build
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
# Release build
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
```

Then, to actually build the project you can do

```sh
# Debug
cmake --build build/debug
# Release
cmake --build build/release
```

This will build:

- The library (header only) under the namespace `moutils`
- The solver, an executable named `moutilssol`

There are some options in CMake project definition that you might want to
configure, which should all be self explanatory

| Flag                | Default | Description      |
|---------------------|---------|------------------|
| `MOUTILS_BUILD_SOLVER` | `ON`    | Build the solver |
| `MOUTILS_BUILD_TESTS`  | `ON`    | Build the tests  |
| `MOUTILS_BUILD_DOCS`   | `ON`    | Build the tests  |

## Using the library

The library is documented at TODO.

## Using the solver

To see usage commands use:

```sh
moutilssol --help
```
