require 'misc'
require 'cbox'
require 'base'
require 'plane'

root = gr.node('root')

cyl = gr.cylinder('cyl', misc.texture(misc.asset('fog.png')));
root:add_child(cyl)

cone = gr.cone('cone', misc.texture(misc.asset('noise.png')));
root:add_child(cone)

cube = gr.cube('cube')
cube:set_material(misc.texture(misc.asset('lava.png')));
root:add_child(cube)

sphere = gr.sphere('sphere')
sphere:set_material(misc.texture(misc.asset('clouds1.png')));
root:add_child(sphere)

cyl:translate(1.5, 0, 1.5)
cone:translate(0, -1.5, 1)
cube:translate(-1.5, 0, 1)
sphere:translate(0, 1.2, -1.2)

cyl:rotate('x', -25)
cube:rotate('x', 45)
cube:rotate('y', 45)
sphere:rotate('x', 0)

cyl:scale(0.5, 1, 0.5)
cone:scale(0.5, 1.5, 0.5)
sphere:scale(0.75, 0.75, 0.75)
cube:scale(0.5, 0.5, 0.5)

base.lights = {gr.light({0, 0, 2.5}, {0.9, 0.9, 0.9}, {0, 0, 0.15})}

cbox.cbox(root)
