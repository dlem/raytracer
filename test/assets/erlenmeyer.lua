module('erlenmeyer', package.seeall)

require 'misc'
require 'cbox'

glass = gr.material({0, 0, 0}, {0.95, 0.95, 0.95}, 10)
glass:set_ri(1.5)
dbg = gr.material({1, 0, 0}, {0, 0, 0}, 20)
dbg = glass
liq = gr.material({0, 0, 0}, {250/255, 123/255, 100/255}, 10)
liq:set_ri(1.33)

lo = 0.1
mid = 0.8
top = 0.1
w = 0.7
wtop = 0.2
fullness = 0.1

function mk_em(small, mat, liq)
  c = misc.cone()
  rfactor = 1
  bfactor = 1
  if small then rfactor = 0.7 end
  if small then bfactor = 0.5 end
  c:translate(0, mid/2, 0)
  c:scale(rfactor * w/2, mid, rfactor * w/2)
  c:set_material(mat)
  cyl_mat = mat
  if liq then cyl_mat = gr.air() end
  cyl = misc.cylinder()
  cyl:set_material(cyl_mat)
  cyl:translate(0, -lo + (lo + mid + top)/2, 0)
  cyl:scale(rfactor * wtop/2, (lo + mid + top)/2, rfactor * wtop/2)
  rv = gr.union(c, cyl)
  s = gr.sphere('s')
  s:set_material(mat)
  s:scale(rfactor * w/2, bfactor * lo, rfactor * w/2)
  rv = gr.union(rv, s)
  if liq and false then
    noliq = misc.cone()
    noliq:translate(0, mid/2 + fullness * mid, 0)
    x = (1 - fullness) * rfactor * w/2
    noliq:scale(x, (1 - fullness) * mid, x)
    noliq:set_material(gr.air())
    rv = gr.union(rv, noliq)
  end
  return rv
end

em = gr.union(mk_em(false, glass), mk_em(true, gr.air(), false))

function erlenmeyer()
  n = gr.node('erlenmeyer')
  n:add_child(em)
  n:translate(0, lo, 0)
  return n
end

if debug.getinfo(2) == nil then
  e = erlenmeyer()
  e:translate(0, -2, 0)
  e:scale(2, 2, 2)
  cbox.cbox(e)
end
