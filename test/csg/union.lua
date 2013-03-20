require 'cbox'

root = gr.node('root')

red = gr.material({0.1, 0, 0}, {0.0, 0.0, 0.0}, 20)
red:set_ri(1.5)

s1 = gr.sphere('s1')
s1:set_material(red)
s1:translate(-100, 0, 0)
s1:scale(100, 100, 100)

s2 = gr.sphere('s2')
s2:set_material(red)
s2:translate(100, 0, 0)
s2:scale(100, 100, 100)

root:add_child(gr.union(s1, s2))
--root:add_child(s1)
--root:add_child(s2)

cbox.cbox(root, 256, 256, 'union.png')

