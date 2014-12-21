module('plane', package.seeall)

require 'misc'
require 'base'

s = 500

function plane(scene)
  scene = scene or gr.node('')

  root = gr.node('real_root')

  floor = gr.cube('floor')
  floor:set_material(gr.material({0.5, 0.5, 0.5}, {0, 0, 0}, 20))
  floor:translate(0, -2.8, 0)
  floor:scale(s, 0.01, s)
  root:add_child(floor)

  base.render(root)
end

if debug.getinfo(2) == nil then
  plane()
end
