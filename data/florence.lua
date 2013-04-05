module('florence', package.seeall)

require 'misc'
require 'cbox'

liq = gr.material({0, 0, 0}, {0.4, 0.95, 0.45}, 10)
liq:set_ri(1.33)

cutoff = 0.1
r = 0.8
rinner = 0.2
h = 1.1

function mk_cylsphere(mat, small, mat_liq, fullness)
  trans = -r
  if(small) then rinner = rinner * 0.9 end
  if(small) then r = r * 0.8 end
  cyl = gr.cylinder('flor_cyl')
  cyl:set_material(mat)
  cyl:scale(rinner, h, rinner)
  sph = gr.sphere('flor_sph')
  sph:set_material(mat)
  sph:translate(0, trans, 0)
  sph:scale(r, r, r)
  c = gr.cube('flor_c')
  c:translate(0, 2*trans - r + cutoff, 0)
  c:set_material(mat)
  sph = gr.difference(sph, c)

  rv = gr.union(sph, cyl)

  if not (mat_liq == nil) then
    liq = gr.sphere('flor_liq')
    liq:set_material(mat_liq)
    liq:translate(0, trans, 0)
    liq:scale(r, r, r)
    cprime = gr.cube('cprime')
    cprime:set_material(mat_liq)
    cprime:translate(0, 2*trans - r + cutoff, 0)
    liq = gr.difference(liq, cprime)
    c2 = gr.cube('flor_c2')
    c2:translate(0, 1, 0)
    c2:scale(1, 1 + (1 - fullness) * 2 * r, 1)
    c2:set_material(mat_liq)
    liq = gr.difference(liq, c2)
    rv = gr.union(rv, liq)
  end

  return rv
end

function florence(mliq, fullness)
  fl = mk_cylsphere(misc.glass(), false, nil, fullness)
  fl = gr.union(fl, mk_cylsphere(gr.air(), true, mliq, fullness))
  fl:translate(0, 2 * r, 0)
  n = gr.node('florence')
  n:add_child(fl)
  return n
end

if debug.getinfo(2) == nil then
  root = gr.node('root')
  fl = florence(liq, 0.5)
  fl:translate(0, -1, 0)
  root:add_child(fl)
  cbox.cbox(root)
end
