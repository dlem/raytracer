module('ttube', package.seeall)

require 'misc'
require 'cbox'

glass = gr.material({0, 0, 0}, {0.95, 0.95, 0.95}, 10)
glass:set_ri(1.5)
liq = gr.material({0, 0, 0}, {0.4, 0.95, 0.45}, 10)
liq:set_ri(1.33)

w = 0.05
r = 0.3
h = 1.3
fullness = 0.5

function mk_cylsphere(mat, r, h)
  cyl = gr.cylinder('ttube_cyl')
  cyl:scale(r, h, r)
  cyl:set_material(mat)
  sph = gr.sphere('ttube_sph')
  sph:translate(0, -h, 0)
  sph:scale(r, r, r)
  sph:set_material(mat)
  return gr.union(sph, cyl)
end

function ttube(liq, fullness, flip)
  tube = mk_cylsphere(glass, r, h)
  inner = mk_cylsphere(gr.air(), r - w, h - w)
  tt = gr.union(tube, inner)
  if fullness > 0 then
    liq = mk_cylsphere(liq, r - w, (h - w) * fullness)
    liq:translate(0, w - (h - w) * (1 - fullness), 0)
    tt = gr.union(tt, liq)
  end
  tt:translate(0, h + r, 0)
  if flip then
    tt:translate(0, -r, 0)
    tt:rotate('x', 180)
  end
  n = gr.node('ttube')
  n:add_child(tt)
  return n
end

if debug.getinfo(2) == nil then
  root = gr.node('root')
  tt = ttube(misc.liquid({0.4, 0.95, 0.45}), 0.7)
  tt:translate(0, -2.5, 0)
  root:add_child(tt)
  cbox.cbox(root)
end
