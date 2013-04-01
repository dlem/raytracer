require 'misc'
require 'florence'
require 'ttube'
require 'cup'
require 'math'

liq = misc.liquid({0.4, 0.8, 0.95})

root = gr.node('root')

for i = 1, 15 do
  fl = cup.cup(0.5, false, liq)
  l = 1.8
  x = -l + math.random() * 2 * l
  y = -l + math.random() * 2 * l
  z = -l + math.random() * 2 * l
  fl:translate(0, -l, 0)
  fl:translate(x, y, z)
  fl:scale(0.25, 0.25, 0.25)
  root:add_child(fl)
end

cbox.cbox(root)
