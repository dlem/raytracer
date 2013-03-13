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
  n:translate(-150 + 100 * ry, 0, 0)
  n:rotate('y', ry * 90)
end

gr.render(root, 'ccube.png', 1024, 1024,
          {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.3, 0.3, 0.3}, {white_light})

