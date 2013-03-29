module('lab', package.seeall)

require 'misc'
require 'base'
require 'cup'
require 'erlenmeyer'
require 'florence'

mbench = gr.material({0.3, 0.3, 0.3}, {0.3, 0.3, 0.3}, 30)
mwall = misc.white()
mceil = mwall
mfloor = mwall

s = 3.2
c = 6

function mkwall(x, y, z, m)
  wall = gr.cube('lwall')
  wall:set_material(m)
  wall:translate(x, y, z)
  wall:scale(100, 100, s)
  return wall
end

function lab(scene, name)
  name = name or misc.png_name or 'lab_noname.png'
  scene = scene or gr.node('')
  root = gr.node('real_root')
  root:add_child(scene)

  nlab = gr.node('nlab')
  root:add_child(nlab)
  --nlab:add_child(mkwall(-c, 0, 0, mfloor))
  --nlab:add_child(mkwall(c, 0, 0, mceil))
  --nlab:add_child(mkwall(0, -c, 0, mwall))
  --nlab:add_child(mkwall(0, c, 0, mwall))
  nlab:add_child(mkwall(0, 0, -c, mwall))
  
  s_bench = {x=100, y=s, z=100}
  cs_bench = gr.node('cs_bench')
  cs_bench:translate(0, -0.75 * 0.5 * c, -0.25 * 0.5 * c)
  nlab:add_child(cs_bench)

  bench = gr.cube('bench')
  bench:translate(0, -s_bench.y, 0)
  bench:scale(s_bench.x, s_bench.y, s_bench.z)
  bench:set_material(mbench)
  cs_bench:add_child(bench)

  cup1 = cup.cup()
  cup1:translate(1, 0, 2)
  cup1:scale(1.2, 1, 1.2)
  cs_bench:add_child(cup1)

  fl1 = florence.florence()
  fl1:translate(0, 0.4, 3)
  fl1:scale(1.7, 1.7, 1.7)
  cs_bench:add_child(fl1)

  if(false) then
  s_e1 = {x=3, y=3, z=3}
  e1 = erlenmeyer.erlenmeyer()
  e1:translate(-1, 0.5, 3)
  e1:scale(3, 3, 3)
  cs_bench:add_child(e1)
end

  w = 256
  h = 256

  gr.option("--unit-distance 1")
  light = gr.light({4, 3, 4}, {0.9, 0.9, 0.9}, {0, 0, 0.03})
  light:set_radius(0.05)
  gr.set_miss_colour({0, 0, 0})

  if(false) then
    gr.render(root, name, w, h, {0, 0, 8}, {0, 0, -1}, {0, 1, 0}, 50,
              {0.4, 0.4, 0.4}, {light})
  else
    gr.render(root, name, w, h, {0, 8, 8}, {0, -1/1.41, -1/1.41}, {0, 1/1.41, -1/1.41}, 50,
                                {0.4, 0.4, 0.4}, {light})
  end
end

if debug.getinfo(2) == nil then
  lab()
end
