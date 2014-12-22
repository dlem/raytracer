module('aa', package.seeall)

require 'misc'
require 'ghost'
require 'base'

gr.option('--lores')

function aa()
  g = ghost.ghost()
  g:translate(0, -1.5, 4)
  g:scale(0.1, 0.1, 0.1)

  gr.set_miss_colour({0, 0, 0})
  gr.option("--unit-distance 1")
  light = gr.light({0, 1.8, 8}, {0.9, 0.9, 0.9}, {0, 0, 0.15})
  light:set_radius(0.05)

  print(misc.png_name)
  gr.render(g, misc.png_name, 100, 100, {0, 0, 8}, {0, 0, -1}, {0, 1, 0}, 50, {0.4, 0.4, 0.4}, {light})
end
