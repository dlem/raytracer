module('lab', package.seeall)

require 'misc'
require 'base'
require 'cup'
require 'erlenmeyer'
require 'florence'
require 'grad'
require 'paper'
require 'holder'

mbench = gr.material({0.5, 0.5, 0.5}, {0.3, 0.3, 0.3}, 30)
mwall = gr.material({0.6, 0.6, 0.6}, {0, 0, 0}, 30)
mceil = mwall
mfloor = mwall

s = 3.2
c = 8

function mkwall(x, y, z, m)
  tex = gr.texture('../bwgrid.png')
  bm = gr.bumpmap('../bwgrid.png')
  tex:remapt('cubetop')
  bm:remap('cubetop')
  m:set_bumpmap(bm)
  m:set_texture(tex)
  wall = gr.cube('lwall')
  wall:set_material(m)
  wall:translate(x, y, z)
  wall:scale(10, 10, s)
  wall:rotate('x', 90)
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
  
  s_bench = {x=10, y=s, z=10}
  cs_bench = gr.node('cs_bench')
  cs_bench:translate(0, -0.75 * 0.5 * c, -0.25 * 0.5 * c)
  nlab:add_child(cs_bench)

  bench = gr.cube('bench')
  bench:translate(0, -s_bench.y, 0)
  bench:scale(s_bench.x, s_bench.y, s_bench.z)
  bench:set_material(mbench)
  cs_bench:add_child(bench)

  cup1 = cup.cup(0.6, true, misc.liquid({0.5, 0.95, 0.95}), true)
  cup1:translate(3.1, 0, 3)
  cup1:scale(1.2, 1, 1.2)
  cs_bench:add_child(cup1)

  fl1 = florence.florence(misc.liquid({0.4, 0.95, 0.45}), 0.5)
  fl1:translate(-3.25, 0.2, 2.25)
  fl1:scale(1.7, 1.7, 1.7)
  cs_bench:add_child(fl1)

  fl2 = florence.florence(misc.liquid({1, 0, 0}), 0.5)
  fl2:translate(5.15, 0.2, -2)
  fl2:scale(1.7, 1.7, 1.7)
  --cs_bench:add_child(fl2)

  gliq1 = gr.material({0, 0, 0}, {1, 0.5, 1}, 20)
  gliq1:set_ri(1.33)
  g1 = grad.grad(gliq1, 0.9)
  g1:translate(-4, 0, -2)
  cs_bench:add_child(g1)

  gliq2 = gr.material({0, 0, 0}, {219/255, 91/255, 86/255}, 20)
  bm2 = gr.bumpmap('sinewaves')
  bm2:remap('cyltop')
  gliq2:set_bumpmap(bm2)
  gliq2:set_ri(1.33)
  g2 = grad.grad(gliq2, 0.8, false)
  g2:translate(-2, 0, -2)
  cs_bench:add_child(g2)
  
  gliq3 = gr.material({0, 0, 0}, {0/255, 81/255, 255/255}, 20)
  gliq3:set_ri(1.33)
  g3 = grad.grad(gliq3, 0.6)
  g3:translate(0, 0, -2)
  cs_bench:add_child(g3)

  gr1 = paper.paper('../graph1.png', 8.5, 11)
  gr1:translate(0, 0, 2)
  gr1:scale(1.6, 1, 1.6)
  gr1:rotate('y', -15)
  cs_bench:add_child(gr1)

  p2 = paper.paper(nil, 8.5, 11)
  p2:translate(0, -0.001, 2)
  p2:scale(1.6, 1, 1.6)
  p2:rotate('y', 5)
  cs_bench:add_child(p2)

  pertable = paper.paper('../pertable.png', 14.5, 8.5)
  pertable:translate(6, 3.5, -c + s)
  pertable:scale(3, 3, 1)
  pertable:rotate('x', 90)
  --root:add_child(pertable)

  h1 = holder.holder()
  h1:translate(3, -0.8, -0.4)
  h1:scale(0.8, 0.8, 0.8)
  h1:rotate('y', -30)
  cs_bench:add_child(h1)

  gr.option("--unit-distance 1")
  light = gr.light({1, 2, 2}, {0.9, 0.9, 0.9}, {0, 0, 0.03})
  light:set_radius(0.05)
  gr.set_miss_colour({0, 0, 0})

  if(false) then
    gr.render(root, name, w, h, {0, 0, 8}, {0, 0, -1}, {0, 1, 0}, 50,
              {0.4, 0.4, 0.4}, {light})
  else
    gr.render(root, name, 256, 256, {0, 8, 8}, {0, -1/1.41, -1/1.41}, {0, 1/1.41, -1/1.41}, 50,
                                {0.4, 0.4, 0.4}, {light})
  end
end

if debug.getinfo(2) == nil then
  lab()
end
