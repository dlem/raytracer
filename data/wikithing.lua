module("wikithing", package.seeall)

require 'misc'
require 'cbox'

tube = gr.cylinder('wt_tube', misc.green())
tube:scale(0.5, 1, 0.5)
cube = gr.cube('wt_cube')
cube:scale(0.75, 0.75, 0.75)
sphere = gr.sphere('wt_sphere')

cube:set_material(misc.pink())
sphere:set_material(misc.blue())

n1 = gr.intersection(cube, sphere)
n2 = gr.node('wt_n2')
n2:add_child(tube)
n2:rotate('x', 90)
n3 = gr.union(tube, n2)
n4 = gr.node('wt_n4')
n4:add_child(tube)
n4:rotate('z', 90)
n5 = gr.union(n3, n4)
wt = gr.difference(n1, n5)

i = 0

function wikithing()
  n = gr.node('wt' .. i)
  i = i + 1
  n:add_child(wt)
  return n
end

if(debug.getinfo(2) == nil) then
  if(true) then
  wt = wikithing()
  wt:translate(0, -0.8, 0)
  wt:rotate('x', 45)
  wt:rotate('y', 45)
else
  s = gr.sphere('')
  s:set_material(blue)
  s:translate(-1, 0, 0)
  c = gr.cube('')
  c:set_material(green)
  wt = gr.difference(s, c)
end
  cbox.cbox(wt)
end
