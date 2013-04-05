require 'misc'
require 'cbox'
require 'ghost'

gr.option('--hires')

root = gr.node('root')

cbox.mb = misc.texture('redcheck', nil, 'cubefront')
--cbox.ml = gr.material({0.4, 0.0, 0.4}, {0.4, 0.4, 0.4}, 30)

g = ghost.ghost()
g:translate(0, -2.5, -2)
g:scale(0.03, 0.03, 0.03)
root:add_child(g)

sph = gr.sphere('sph')
msph = gr.material({0, 0, 0}, {1, 1, 1}, 30)
msph:set_ri(1.5)
sph:set_material(msph)
sph:scale(1.2, 1.2, 1.2)
sph:translate(0, -1.25, 1)
root:add_child(sph)

sph2 = gr.sphere('sph2')
sph2:set_material(msph)
sph2:scale(0.8, 0.8, 0.8)
sph2:translate(1.8, 1.8, 0)
root:add_child(sph2)

sph3 = gr.sphere('sph2')
sph3:set_material(msph)
sph3:scale(0.8, 0.8, 0.8)
sph3:translate(-1.8, 1.8, 0)
root:add_child(sph3)

cbox.cbox(root)
