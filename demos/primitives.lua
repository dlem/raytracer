require 'misc'
require 'base'

mcone = gr.material({0, 0.8, 0.95}, {0, 0, 0}, 20)
mcyl = misc.green()

root = gr.node('root')

function cone(x, y, z, rx)
  c = misc.cone()
  c:set_material(mcone)
  c:translate(x, y, z)
  c:rotate('x', rx)
  c:scale(0.5, 2, 0.5)
  root:add_child(c)
end

function cylinder(x, y, z, rx)
  c = misc.cylinder()
  c:set_material(mcyl)
  c:translate(x, y, z)
  c:rotate('x', rx)
  c:scale(0.5, 1, 0.5)
  root:add_child(c)
end

li = -2
lf = 2
ycy = 2
yco = -2
z = 0
ri = 0
rf = 270
imax = 3
for i = 0, imax do
  x = li + i * (lf - li) / imax;
  rz = ri + i * (rf - ri) / imax;
  cone(x, yco, z, rz)
  cylinder(x, ycy, z, rz)
end

light1 = gr.light({0, -1.8, 4}, {0.9, 0.9, 0.9}, {0, 0, 0.15})
light2 = gr.light({0, 1.8, 4}, {0.9, 0.9, 0.9}, {0, 0, 0.15})

gr.option('--midres')
gr.option('--unit-distance 1')
gr.set_miss_colour({0, 0, 0})
gr.render(root, misc.png_name, 256, 256, {0, 0, 8}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.4, 0.4, 0.4}, {light1, light2})

