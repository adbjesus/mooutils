# MOOUtils - Multi-Objective Optimization Utilities

MOOUtils is a C++20 library containing several utilities to help
implement multi-objective optimization algorithms. Currently, it
consists of the following headers:

- [mooutils/concepts.hpp](mooutils/include/mooutils/concepts.hpp) -
  contains the C++20 concepts considered throughout the library in
  particular to identify what constitutes a decision, objective, or
  constraint vector, how a solution class can claim to have such
  vectors, and what defines a solution set.
- [mooutils/indicators.hpp](mooutils/include/mooutils/indicators.hpp) -
  contains functions for common multi-objective quality indicators and
  incremental structures for those indicators.
- [mooutils/orders.hpp](mooutils/include/mooutils/orders.hpp) - contains
  common orders such as Pareto dominance and lexicographical order to
  compare solutions.
- [mooutils/queues.hpp](mooutils/include/mooutils/queues.hpp) - contains
  different types of commonly considered queues to keep solutions.
- [mooutils/sets.hpp](mooutils/include/mooutils/sets.hpp) - contains
  different types of non-dominated (minimal) sets to keep solutions.
- [mooutils/solution.hpp](mooutils/include/mooutils/solution.hpp) -
  contains functions related to a solution, e.g., to get a particular
  vector from a solution, as well as, base classes that can be easily
  used to design a solution.
  
Note: This library is still under active development and breaking
changes may occur between minor versions. Nonetheless, the code is
tested and all algorithms and data structures are expected to be
correct.

## Dependencies

- [Catch2](https://github.com/catchorg/Catch2) for tests.
- [Doxygen](https://www.doxygen.nl/index.html) for generating documentation.

## Installation

To install the library you need to first configure the project:

```sh
cmake -B build
```

Then, run:

```sh
cmake --install build --prefix "installdir"
```

This will install the library in the `installdir` folder.

## Setup for development

To configure an enviroment for development you can use the following
cmake configuration:

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DMOOUTILS_ENABLE_TESTS=ON \
  -DMOOUTILS_ENABLE_BENCHMARKS=ON \
  -DMOOUTILS_ENABLE_WARNINGS=ON
```

This will turn on building the tests, benchmarks, and warnings.

## Using with CMAKE

This library is prepared for easy inclusion with cmake. In particular,
after installing the library, it should be possible to include it in a
cmake project by adding:

```CMake
find_package(mooutils)
```

Alternatively, it can be fetched from this repo with FetchContent:

```CMake
include(FetchContent)
FetchContent_Declare(
  mooutils
  GIT_REPOSITORY https://github.com/adbjesus/mooutils.git
)
FetchContent_MakeAvailable(mooutils)
add_library(mooutils::mooutils ALIAS mooutils)
```

Then, simply link it to the target:

```CMake
target_link_libraries(sometarget mooutils::mooutils)
```

## Using with nix flakes

This library is prepared to be used with nix flakes. As such, if you
are using nix flakes to manage the dependencies of your project you
can include the url to this repository in your inputs, and add it as a
dependency.

