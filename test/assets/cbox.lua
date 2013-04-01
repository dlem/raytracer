module("cbox", package.seeall)

require 'misc'
require 'base'

ml = gr.material({0.8, 0, 0.9}, {0, 0, 0}, 1)
mr = gr.material({0, 0.9, 0.0}, {0, 0, 0}, 1)
mb = gr.material({0.95, 0.95, 0.95}, {0, 0, 0}, 1)
mt = gr.material({0.95, 0.95, 0.95}, {0, 0, 0}, 1)
mlo = gr.material({0.95, 0.95, 0.95}, {0, 0, 0}, 1)

diffmat = nil


function cbox(scene, w, h, name)
  scene = scene or gr.node('')

  root = gr.node('real_root')

  s = 3.2
  c = 6
  function mkwall(x, y, z, m, diffmat)
    wall = gr.cube('cwall')
    wall:set_material(m)
    wall:translate(x, y, z)
    wall:scale(s, s, s)

    if diffmat ~= nil then
      wall = gr.difference(wall, diffmat)
    end

    box:add_child(wall)
  end

  box = gr.node('cbox')

  mkwall(-c, 0, 0, ml)
  mkwall(c, 0, 0, mr)
  mkwall(0, -c, 0, mlo)
  mkwall(0, c, 0, mt)
  mkwall(0, 0, -c, mb, diffmat)

  root:add_child(scene)
  root:add_child(box)

  base.render(root, w, h, name)
end

if(debug.getinfo(2) == nil) then
  cbox()
end
