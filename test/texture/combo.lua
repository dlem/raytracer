white_light = gr.light({100, 0, 100}, {0.9, 0.9, 0.9}, {1, 0, 0})

root = gr.node('root')

mat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
mat:set_texture('../companion_cube_tex.png')
mat:set_bumpmap('../companion_cube_bm.png')

ccube = gr.cube('ccube')
ccube:scale(100, 100, 100)
ccube:set_material(mat)
ccube:translate(-0.5, -0.5, -0.5)

for ry = 0, 4 do
  n = gr.node('')
  root:add_child(n)
  n:add_child(ccube)
  n:translate(-300 + 150 * ry, 100, 0)
  n:rotate('y', ry * 90)
end

earthmat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
earthmat:set_texture('../earthmap1k.png')
earthmat:set_bumpmap('../earthbump1k.png')

earth = gr.sphere('earth')
earth:scale(50, 50, 50)
earth:set_material(earthmat)

for ry = 0, 4 do
  n = gr.node('')
  root:add_child(n)
  n:add_child(earth)
  n:translate(-300 + 150 * ry, -100, 0)
  n:rotate('y', ry * 90)
end


gr.render(root, 'combo.png', 1024, 1024,
          {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.3, 0.3, 0.3}, {white_light})

