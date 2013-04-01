require 'misc'
require 'base'
require 'cbox'

mirror = gr.material({0.2, 0.2, 0.2}, {0.7, 0.7, 0.7}, 20)
rip = gr.material({0.2, 0.2, 0.2}, {0.7, 0.7, 0.7}, 20)
rip:set_bumpmap(misc.bumpmap('sinewaves', 'cubetop'))
cbox.mlo = rip

root = gr.node('root')

sph = gr.sphere('sph')
sph:set_material(mirror)
sph:translate(2, -1, 0)
sph:scale(0.75, 0.75, 0.75)
root:add_child(sph)

cone = misc.cone('cone')
cone:set_material(mirror)
cone:translate(-2, -1, 0)
cone:scale(0.5, 1, 0.5)
root:add_child(cone)

sph2 = gr.sphere('sph2')
sph2:set_material(misc.texture('redcheck'))
sph2:translate(0, -1, 0)
sph2:scale(0.4, 0.4, 0.4)
root:add_child(sph2)

sph3 = gr.sphere('sph3')
sph3:set_material(misc.texture('bluecheck'))
sph3:translate(2, 1.3, -1)
sph3:scale(0.4, 0.4, 0.4)
root:add_child(sph3)

sph4 = gr.sphere('sph4')
sph4:set_material(misc.texture('greencheck'))
sph4:translate(-2, 1.3, -1)
sph4:scale(0.4, 0.4, 0.4)
root:add_child(sph4)

cyl = misc.cylinder('cyl')
cyl:set_material(mirror)
cyl:translate(0, 0.75, -2)
cyl:scale(0.5, 0.5, 1)
root:add_child(cyl)

base.lights = {gr.light({0, 1.8, 1.8}, {0.9, 0.9, 0.9}, {0, 0, 0.15})}

cbox.cbox(root)
