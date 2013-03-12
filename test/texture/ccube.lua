white_light = gr.light({100, 0, 100}, {0.9, 0.9, 0.9}, {1, 0, 0})

root = gr.node('root')

mat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
mat:set_texture('../companion_cube_tex.png')
--mat:set_bumpmap('earthbump1k.png')

ccube = gr.cube('ccube')
ccube:translate(-0.5, -0.5, -0.5)
ccube:scale(100, 100, 100)
ccube:set_material(mat)
root:add_child(ccube)


gr.render(root, 'ccube.png', 256, 256,
          {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.3, 0.3, 0.3}, {white_light})

