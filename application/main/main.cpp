#include <core/cli.h>

int main(const int argc, const char** argv) {
  core::CommandLineInterface cli;

  return cli.Run(argc, argv);
}
