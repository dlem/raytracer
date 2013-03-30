module('base', package.seeall)

require 'misc'

function render(scene, w, h, name)
  scene = scene or gr.node('')
  name = name or misc.png_name or 'base_noname.png'
  w = w or 256
  h = h or 256

  light = gr.light({0, 1.8, 0}, {0.9, 0.9, 0.9}, {0, 0, 0.15})
  light:set_radius(0.05)

  gr.option('--unit-distance 1')
  gr.render(scene, name, w, h, {0, 0, 8}, {0, 0, -1}, {0, 1, 0}, 50,
            {0.4, 0.4, 0.4}, {light})
end

if debug.getinfo(2) == nil then
  render()
end
