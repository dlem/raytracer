module("cbox", package.seeall)

require 'misc'
require 'base'

function cbox(scene, w, h, name)
  scene = scene or gr.node('')

  root = gr.node('real_root')

  s = 3.2
  c = 6
  function mkwall(x, y, z, m)
    wall = gr.cube('cwall')
    wall:set_material(m)
    wall:translate(x, y, z)
    wall:scale(s, s, s)
    box:add_child(wall)
  end
  mr = gr.material({0.9, 0, 0}, {0, 0, 0}, 1)
  mg = gr.material({0, 0.9, 0}, {0, 0, 0}, 1)
  mw = gr.material({0.9, 0.9, 0.9}, {0, 0, 0}, 1)

  box = gr.node('cbox')

  d = 0
  mkwall(-c, 0, 0, mr)
  mkwall(c, 0, 0, mg)
  mkwall(0, -c, 0, mw)
  mkwall(0, c, 0, mw)
  mkwall(0, 0, -c, mw)

  root:add_child(scene)
  root:add_child(box)

  base.render(root, w, h, name)
end

if(debug.getinfo(2) == nil) then
  cbox()
end
