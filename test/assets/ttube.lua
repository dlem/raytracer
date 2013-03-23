module('ttube', package.seeall)

require 'misc'
require 'cbox'

glass = gr.material({0, 0, 0}, {1, 1, 1}, 10)
glass:set_ri(1.5)

function mk_outline(ends, r)
  cyl = gr.cylinder('ttube_cyl')
  cyl:translate(0, 0.5 * ends, 0)
  cyl:scale(r, 1 - 0.5 * ends, r)
  cyl:set_material(glass)
  sph = gr.sphere('ttube_sph1')
  sph:translate(0, -1 + ends, 0)
  sph:scale(r, ends, r)
  sph:set_material(glass)
  bottom = gr.union(cyl, sph)
  return bottom
end

inner_r = 0.09

r = 0.1
ends = 0.2
epsilon = 0.001
tube = mk_outline(ends, r)

function ttube(liquid_mat, fullness)
  if(liquid_mat == nil or fullness == nil) then
    fullness = 0
  end

  empty_chunk = gr.cylinder('ttube_air')
  empty_chunk:translate(0, ends + 2 * fullness + epsilon, 0)
  empty_chunk:scale(inner_r, 1, inner_r)
  empty_chunk:set_material(gr.air())

  if(fullness > 0) then
    liq_chunk = gr.cylinder('ttube_liquid')
    liq_chunk:translate(0, ends, 0)
    liq_chunk:scale(inner_r - epsilon, 1, inner_r - epsilon)
    liq_chunk:set_material(liquid_mat)
  end

  return gr.difference(gr.difference(tube, empty_chunk), liq_chunk)
end

if(debug.getinfo(2) == nil) then
  root = gr.node('root')
  if(false) then
  ball = gr.sphere('ball')
  ball:set_material(gr.material({0, 0, 0}, {1, 1, 1}, 20))
  ball:translate(-1, -1, -2)
  root:add_child(ball)
  end

  cyl = gr.cylinder('')
  cyl:set_material(gr.material({0, 0, 0}, {0.2, 0.2, 0.9}, 0))
  cyl:translate(0, -1.5, -2)
  cyl:rotate('z', 45)
  cyl:scale(2, 0.1, 0.1)
  root:add_child(cyl)

  liquid_mat = gr.material({0, 0, 0}, {0.2, 0.9, 0.2}, 10)
  liquid_mat:set_ri(1.33)
  tt = ttube(liquid_mat, 0.4)
  tt:translate(0, -1, 0)
  tt:scale(3, 1.5, 3)
  root:add_child(tt)
  cbox.cbox(root)
end
