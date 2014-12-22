module(..., package.seeall)

require 'readobj'
require 'misc'

hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)

cow_poly = gr.mesh('cow', readobj(misc.asset('cow.obj')))
factor = 2.0/(2.76+3.637)

cow_poly:set_material(hide)

cow_poly:translate(0.0, -1.0, 0.0)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(0.0, 3.637, 0.0)

n = gr.node('macho_cow')
n:add_child(cow_poly)
n:translate(0, 30, 0)
n:scale(30, 30, 30)

cow = n


i = 0

function get_cow(p, x, y, z, s)
  rv = gr.node('macho-cow' .. i)
  i = i + 1
  rv:add_child(cow)
  p:add_child(rv)
  rv:translate(x, y, z)
  rv:scale(s, s, s)
  return rv
end
