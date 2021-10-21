# MOOUtils - Multi-Objective Optimization Utilities

MOOUtils is a C++20 library containing several utilities to help
implement multi-objective optimization algorithms. Currently, it is made
up of the following headers:

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
  
The documentation can be found at:

Note: This library is still under active development and breaking
changes may occur between minor versions.

## Dependencies

- [Catch2](https://github.com/catchorg/Catch2) for tests.
- [Google Benchmark](https://github.com/google/benchmark) for benchmarks.
- [Doxygen](https://www.doxygen.nl/index.html) for generating documentation.

## Installation

To install the library you need to first configure the project:

```sh
cmake -S . -B build
```

Then, run:

```sh
cmake --install build --prefix "installdir"
```

This will install the library in the `installdir` folder.

## Setup for development

To configure an enviroment for development you should set the
`MOOUTILS_DEVELOPMENT_BUILD` flag to `ON`, for example:

```bash
cmake -S . -B build -DMOOUTILS_DEVELOPMENT_BUILD=ON
```

This will turn on building the tests and benchmarks.
