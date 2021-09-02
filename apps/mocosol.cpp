#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

// #include <mokp/algorithms/bazgan2009.hpp>
#include <moco/algorithms/bb.hpp>
// #include <mokp/algorithms/econstraint.hpp>
// #include <mokp/algorithms/figueira2013.hpp>
// #include <moco/algorithms/nemull.hpp>
#include <moco/problems/knapsack.hpp>

// #include "../include/mokp/algorithms/nemull.hpp"
// #include <mokp/algorithms/pls.hpp>

template <typename P, typename T>
auto run_algorithm(P const& problem, std::string const& algo,
                   T const& timeout) {
  // if (algo == "bazgan2009") {
  //   return bazgan2009(problem, timeout);
  // }
  // if (algo == "figueira2013") {
  //   return figueira2013(problem, timeout);
  // }
  // if (algo == "nemull") {
  //   return nemull(problem, timeout);
  // }
  if (algo == "bb") {
    return bb(problem, timeout);
  }
  // if (algo == "bb_dfs") {
  //   return bb_dfs(problem, timeout); // TODO generalize
  // }
  // if (algo == "bb_hcpq") {
  //   return bb_hcpq(problem, timeout); // TODO generalize
  // }
  // if (algo == "pls") {
  //   return pls(problem, timeout, SelectorAlternate());
  // }
  // if (algo == "pls_mid") {
  //   return pls(problem, timeout, SelectorMid());
  // }
  // if (algo == "pls_ohi") {
  //   return pls(problem, timeout, SelectorOHI());
  // }
  // if (algo == "pls_rnd") {
  //   std::random_device rd;
  //   std::mt19937 gen(rd());
  //   return pls(problem, timeout, SelectorRnd<std::mt19937>(std::move(gen)));
  // }
  // if (algo == "econstraint") {
  //   return econstraint(problem, timeout); // TODO make anytime version
  // }

  std::cerr << "Unrecognized algorithm... aborting!\n";
  exit(1);
}

int main(int argc, char** argv) {
  // Set algorithm (e.g. nemull, pls, econtraint...)
  // Set algorithm parameters (e.g. nemull order, pls neighborhood)
  //
  // Set quality measurements (e.g. hypervolume)
  // Set quality measurements parameters (e.g. reference point)
  //
  // Set time measurements (e.g. cputime, iterations, solutions evaluated)
  // Set time parameters (e.g. cputime precision)
  //
  // Set stopping conditions (eg. cputime/iterations timeout, quality achieved)
  //
  // Set input file
  // Set measurements file
  // Set objective points file (this includes constraints)
  // Set decision vectors file

  if (argc != 5) {
    std::cerr << "Missing arguments.\n";
    std::cerr << "USAGE:\n";
    std::cerr << "  " << argv[0] << " algorithm timeout infile outdir\n";
    return 1;
  }

  std::string algo(argv[1]);
  std::chrono::duration<double> timeout(std::stod(argv[2]));
  std::filesystem::path fin(argv[3]);
  std::filesystem::path dout(argv[4]);

  if (!std::filesystem::is_regular_file(fin)) {
    std::cerr << "Could not find file: " << fin << std::endl;
    return 0;
  }

  std::filesystem::create_directories(dout);

  auto problem = moco::problems::Knapsack<int_fast64_t, 2, 1>::from_stream(
      std::ifstream(fin));

  auto result = run_algorithm(problem, algo, timeout);

  std::cout << result << std::endl;

  return 0;
}
