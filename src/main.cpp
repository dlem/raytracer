/**
 * A4 main function.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#include <iostream>
#include <vector>
#include "scene_lua.hpp"
#include "cmdopts.hpp"

using namespace std;

extern CmdOpts g_the_opts;

int main(int argc, const char** argv)
{
  const int trailing = g_the_opts.init(argc, argv);

  auto run = [](const std::string &filename)
  {
    if(!run_lua(filename))
    {
      std::cerr << "Could not open " << filename << std::endl;
      exit(1);
    }
  };

  if(trailing >= argc)
    run("scene.lua");
  else
    for(int i = trailing; i < argc; i++)
      run(argv[i]);
}
