white_light = gr.light({100, 0, 100}, {0.9, 0.9, 0.9}, {1, 0, 0})

root = gr.node('root')

ccmat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
ccmat:set_texture('../companion_cube_tex.png')
ccmat:set_bumpmap('../companion_cube_bm.png')

ccube = gr.cube('ccube')
ccube:scale(100, 100, 100)
ccube:set_material(ccmat)
ccube:translate(-0.5, -0.5, -0.5)

earthmat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
earthmat:set_texture('../earthmap1k.png')
earthmat:set_bumpmap('../earthbump1k.png')

barrelmat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
barrelmat:set_texture('../barrel.png')
barrelmat:set_bumpmap('../barrel_bm.png')

cyl = gr.cylinder('cyl')
cyl:scale(100, 100, 100)
cyl:set_material(barrelmat)

earth = gr.sphere('earth')
earth:scale(50, 50, 50)
earth:set_material(earthmat)

cone = gr.cone('cone')
cone:scale(50, 100, 50)
cone:set_material(earthmat)

for ry = 0, 4 do
  n = gr.node('')
  root:add_child(n)
  n:add_child(ccube)
  n:translate(-300 + 150 * ry, 100, 0)
  n:rotate('y', ry * 90)
  n:rotate('x', ry * 90)
end

for ry = 0, 4 do
  n = gr.node('')
  root:add_child(n)
  n:add_child(earth)
  n:translate(-300 + 150 * ry, -100, 0)
  n:rotate('y', ry * 90)
  n:rotate('x', ry * 90)
end

for ry = 0, 4 do
  n = gr.node('')
  root:add_child(n)
  n:add_child(cone)
  n:translate(-300 + 150 * ry, -300, 0)
  n:rotate('y', ry * 90)
  n:rotate('x', ry * 90)
end

for ry = 0, 4 do
  n = gr.node('')
  root:add_child(n)
  n:add_child(cyl)
  n:translate(-300 + 150 * ry, 300, 0)
  n:rotate('y', ry * 90)
  n:rotate('x', ry * 90)
end

gr.render(root, 'combo.png', 1024, 1024,
          {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.3, 0.3, 0.3}, {white_light})

