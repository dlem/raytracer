require 'misc'
require 'cbox'
require 'plane'

root = gr.node('root')

cyl = gr.cylinder('cyl', misc.texture('redcheck'));
root:add_child(cyl)

cone = gr.cone('cone', misc.texture('bluecheck'));
root:add_child(cone)

cube = gr.cube('cube')
cube:set_material(misc.texture('greencheck'));
root:add_child(cube)

sphere = gr.sphere('sphere')
sphere:set_material(misc.texture('whitecheck'));
root:add_child(sphere)

cyl:translate(1, 1, -1)
cone:translate(-1, -1, 1)
cube:translate(1, -1, 1)
sphere:translate(-1, 1, -1)

cyl:rotate('x', -25)
cone:rotate('x', -45)
cube:rotate('x', 45)
cube:rotate('y', 45)
sphere:rotate('x', 45)

cyl:scale(0.5, 1, 0.5)
cone:scale(0.5, 1.5, 0.5)
sphere:scale(0.75, 0.75, 0.75)
cube:scale(0.5, 0.5, 0.5)

base.lights = {gr.light({0, 0, 1.8}, {0.9, 0.9, 0.9}, {0, 0, 0.15})}

gr.option('--hires')
cbox.cbox(root)
