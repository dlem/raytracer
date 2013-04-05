require 'misc'
require 'cbox'
require 'wikithing'

wt = wikithing.wikithing()

s = {x=0.2, y=0.2, z=0.2}

root = gr.node('root')

function make_at(x, y, z)
  n = gr.node('wt')
  n:add_child(wt)
  n:translate(x, y, z)
  n:scale(s.x, s.y, s.z)
  n:rotate('x', math.random() * 360)
  n:rotate('y', math.random() * 360)
  root:add_child(n)
end

for i = 1, 30 do
  make_at(-2 + math.random() * 2 * 2,
          -2 + math.random() * 2 * 1.5,
          -2 + math.random() * 2 * 2)
end

cbox.cbox(root)
