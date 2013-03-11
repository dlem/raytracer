/**
 * A4 main function.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#include <iostream>
#include "scene_lua.hpp"
#include "cmdopts.hpp"

int main(int argc, char** argv)
{
  parse_args(argc, argv);

  auto run = [](const std::string &filename)
  {
    if(!run_lua(filename))
    {
      std::cerr << "Could not open " << filename << std::endl;
      exit(1);
    }
  };

  if (optind >= argc)
    run("scene.lua");
  else
    for(int i = optind; i < argc; i++)
      run(argv[i]);
}
