require 'misc'
require 'cbox_gi'

root = gr.node('root')

cbox_gi.ml = gr.material({0.6, 1, 0.95}, {0, 0, 0}, 30)

c = gr.cube('cube')
c:set_material(gr.material({0.6, 1, 0.5}, {0, 0, 0}, 30))
c:translate(-1.5, -2, 0)
c:scale(0.5, 1, 0.5)
root:add_child(c)

c = gr.cylinder('cylinder', gr.material({0.6, 0.5, 1}, {0, 0, 0}, 30))
c:translate(1.5, -2, -1)
c:scale(0.7, 1, 0.7)
root:add_child(c)

gr.option('--gi --gi-num-photons 8000000 --gi-num-neighbours 600')

cbox_gi.cbox(root)
