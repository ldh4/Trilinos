#include <Kokkos_Core.hpp>

#include <Kokkos_TaskPolicy.hpp>
#include <impl/Kokkos_Serial_TaskPolicy.hpp>

#include "Teuchos_CommandLineProcessor.hpp"

using namespace std;

typedef double value_type;
typedef int    ordinal_type;
typedef int    size_type;

typedef Kokkos::Serial exec_space;

#include "example_symbolic_factor.hpp"

using namespace Example;

int main (int argc, char *argv[]) {

  Teuchos::CommandLineProcessor clp;
  clp.setDocString("This example program demonstrates symbolic factorization algorithm on Kokkos::Serial execution space.\n");

  int fill_level = 0;
  clp.setOption("fill-level", &fill_level, "Fill level for incomplete factorization");

  int league_size = 1;
  clp.setOption("league-size", &league_size, "League size");

  bool verbose = false;
  clp.setOption("enable-verbose", "disable-verbose", &verbose, "Flag for verbose printing");

  string file_input = "test.mtx";
  clp.setOption("file-input", &file_input, "Input file (MatrixMarket SPD matrix)");

  int treecut = 15;
  clp.setOption("treecut", &treecut, "Level to cut tree from bottom");

  int minblksize = 0;
  clp.setOption("minblksize", &treecut, "Minimum block size for internal reordering");

  int seed = 0;
  clp.setOption("seed", &seed, "Seed for random number generator in graph partition");

  bool reorder = false;
  clp.setOption("enable-reorder", "disable-reorder", &reorder, "Flag for reordering a matrix");

  clp.recogniseAllOptions(true);
  clp.throwExceptions(false);

  Teuchos::CommandLineProcessor::EParseCommandLineReturn r_parse= clp.parse( argc, argv );

  if (r_parse == Teuchos::CommandLineProcessor::PARSE_HELP_PRINTED) return 0;
  if (r_parse != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL  ) return -1;

  int r_val = 0;
  {
    Kokkos::initialize();

    r_val = exampleSymbolicFactor
      <value_type,ordinal_type,size_type,exec_space,void>
      (file_input, treecut, minblksize, seed, fill_level, league_size, reorder, verbose);

    Kokkos::finalize();
  }

  return r_val;
}
