require 'misc'
require 'base'
require 'cbox'
require 'ring'

gr.option('--hires')

root = gr.node('root')

s_sph = 0.9

metal = gr.material({0, 0, 0}, {1, 1, 1}, 30)

gsph = gr.sphere('gsph')
gsph:set_material(misc.glass())
gsph:translate(1.5, -1.8, 0)
gsph:scale(s_sph, s_sph, s_sph)
root:add_child(gsph)

msph = gr.sphere('msph')
msph:set_material(misc.mirror())
msph:translate(-1.5, -1.8, 0)
msph:scale(s_sph, s_sph, s_sph)
root:add_child(msph)

r = ring.ring()
r:translate(0, 0, -2.5)
r:rotate('x', 90)
r:scale(4, 6, 4)
root:add_child(r)

gr.option('--caustic-num-photons 24000000 --caustic-num-neighbours 300')

base.ambient = {0.1, 0.1, 0.1}

cbox.cbox(root)
