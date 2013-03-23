require 'misc'
require 'cbox'
require 'wikithing'

s = 0.3 

root = gr.node('')

function mk(x, y, z)
  n = gr.node('')
  n:translate(x, y, z)
  n:scale(s, s, s)
  n:add_child(wikithing.wikithing())
  root:add_child(n)
end

d = 3
for x = 0, 5 do
  for y = 0, 1 do
    for z = 0, 1 do
      mk(-d/2 + d/6 * x, -d/2 + d * y, -d/2 + d * z)
    end
  end
end

cbox.cbox(root, 100, 100)
