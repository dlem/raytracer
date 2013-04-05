module('paper', package.seeall)

require 'misc'
require 'base'

function paper(imgname, w, h)
  mat = gr.material({1, 1, 1}, {0, 0, 0}, 10)
  if imgname ~= nil then
    t = gr.texture(imgname)
    t:remapt('cubetop')
    mat:set_texture(t)
  end
  c = gr.cube('paper')
  c:translate(0, 0.01, 0)
  c:scale(w * 0.1, 0.001, h * 0.1)
  c:set_material(mat)
  n = gr.node('')
  n:add_child(c)
  return n
end

if debug.getinfo(2) == nil then
  n = paper('../graph1.png', 8.5, 11)
  n:translate(0, -2, 0)
  n:rotate('x', 75)
  base.render(n, 512, 512)
end
