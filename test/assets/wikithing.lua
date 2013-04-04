module("wikithing", package.seeall)

require 'misc'
require 'cbox'

spec = {0.0, 0.0, 0.0}
shin = 20
green = gr.material({0, 0.8, 0}, {0.3, 0.2, 0.3}, shin)
blue = gr.material({0, 0, 0.9}, spec, shin)
pink = gr.material({0.9, 0.2, 0.1}, spec, shin)

tube = gr.cylinder('wt_tube')
tube:scale(0.5, 1, 0.5)
cube = gr.cube('wt_cube')
cube:scale(0.75, 0.75, 0.75)
sphere = gr.sphere('wt_sphere')

tube:set_material(green)
cube:set_material(pink)
sphere:set_material(blue)

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
