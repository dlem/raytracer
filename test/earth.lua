
white_light = gr.light({100, 0, 100}, {0.9, 0.9, 0.9}, {1, 0, 0})

root = gr.node('root')

mat = gr.material({1, 1, 1}, {0.5, 0.5, 0.5}, 10)
mat:set_texture('earthmap1k.png')
--mat:set_bumpmap('earthbump1k.png')

earth = gr.sphere('earth')
earth:scale(100, 100, 100)
earth:set_material(mat)
root:add_child(earth)


gr.render(root, 'earth_nobump.png', 1024, 1024,
          {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
          {0.3, 0.3, 0.3}, {white_light})

