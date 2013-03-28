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

tube = mk_cylsphere(glass, r, h)
inner = mk_cylsphere(gr.air(), r - w, h - w)
inner:translate(0, w, 0)
it = gr.union(tube, inner)
liq = mk_cylsphere(liq, r - w, (h - w) * fullness)
liq:translate(0, w - (h - w) * (1 - fullness), 0)
it = gr.union(it, liq)

function ttube()
  n = gr.node('ttube')
  n:add_child(it)
  return n
end

if(debug.getinfo(2) == nil) then
  root = gr.node('root')
  if(false) then
  sph = gr.sphere('sph')
  sph:set_material(gr.material({0.2, 0.05, 0.3}, {0.4, 0.4, 0.4}, 20))
  sph:translate(-0.3, -1, -2)
  root:add_child(sph)
  end
  tt = ttube()
  tt:translate(0, -1, 0)
  root:add_child(tt)
  cbox.cbox(root, 256, 256)
end
