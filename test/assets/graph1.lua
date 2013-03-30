module('graph1', package.seeall)

require 'misc'
require 'base'

function graph1()
  t = gr.texture('../graph1.png')
  t:remapt('cubetop')
  mat = gr.material({1, 1, 1}, {0, 0, 0}, 10)
  mat:set_texture(t)
  c = gr.cube('graph1')
  c:translate(0, 0.01, 0)
  c:scale(8.5 * 0.1, 0.001, 11 * 0.1)
  c:set_material(mat)
  n = gr.node('')
  n:add_child(c)
  return n
end

if debug.getinfo(2) == nil then
  n = graph1()
  n:translate(0, -2, 0)
  n:rotate('x', 75)
  base.render(n, 512, 512)
end
