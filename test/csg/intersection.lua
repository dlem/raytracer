require 'cbox'

root = gr.node('root')

glass = gr.material({0, 0, 0}, {1, 1, 1}, 20)
glass:set_ri(1.5)

mirror = gr.material({0, 0, 0}, {1, 1, 1}, 20)

red = gr.material({1, 0, 0}, {0, 0, 0}, 20)

mat = glass

s1 = gr.sphere('s1')
s1:set_material(mat)
s1:translate(0, 0, 0)
s1:scale(100, 100, 100)
s1:rotate('y', 90)

s2 = gr.sphere('s2')
s2:set_material(mirror)
s2:translate(100, 0, 0)
s2:scale(100, 100, 100)

root:add_child(gr.intersection(s1, s2))
--root:add_child(s1)
--root:add_child(s2)

n1 = gr.node('')
n1:translate(0, -200, 0)
n1:add_child(s1)

n2 = gr.node('')
n2:translate(0, -200, 0)
n2:add_child(s2)

root:add_child(n1)
root:add_child(n2)

cbox.cbox(root, 256, 256, 'intersection.png')

