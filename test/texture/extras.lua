cone_mat = gr.material({0/255, 72/255, 217/255}, {0.5, 0.5, 0.5}, 10)
cyl_mat = gr.material({44/255, 242/255, 5/255}, {0, 0, 0}, 10)
cyl_mat:set_texture("../barrel.png")

root = gr.node('root')
cone = gr.cone('cone')
cone:scale(50, 100, 50)
cone:set_material(cone_mat)
cyl = gr.cylinder('cyl')
cyl:scale(100, 100, 100)
cyl:set_material(cyl_mat)


function mkcyl(x, y, z)
  n = gr.node('')
  n:add_child(cyl)
  n:translate(x, y, z)
  return n
end

function mkcone(x, y, z)
  n = gr.node('')
  n:add_child(cone)
  n:translate(x, y, z)
  return n
end

for i = 0, 4 do
  c = mkcyl(-200 + 100 * i, 100, 0)
  c:rotate('x', 90 * i)
  root:add_child(c)
  c = mkcone(-200 + 100 * i, -150, 0)
  c:rotate('x', 90 * i)
  root:add_child(c)
end

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})

gr.render(root, 'combo.png', 256, 256,
          {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.3, 0.3, 0.3}, {white_light})

