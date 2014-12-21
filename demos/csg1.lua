require 'misc'
require 'cbox'
require 'wikithing'

gr.option('--hires')

wt = wikithing.wikithing()

root = gr.node('root')

wt:translate(0, -1, 0)
wt:rotate('x', 45)
wt:rotate('y', 45)

s_tube = {x=-2, y=-1, z=0, rx=45, rz=-20}

c1 = gr.cylinder('c1', misc.red())
c1:translate(s_tube.x, s_tube.y, s_tube.z)
c1:rotate('x', s_tube.rx)
c1:rotate('z', s_tube.rz)
c1:scale(0.5, 1, 0.5)

c2 = gr.cylinder('c2', misc.green())
c2:translate(s_tube.x, s_tube.y, s_tube.z)
c2:rotate('x', s_tube.rx)
c2:rotate('z', s_tube.rz)
c2:scale(0.4, 1.2, 0.4)

tube = gr.difference(c1, c2)

sp1 = gr.sphere('sp1')
sp1:set_material(misc.halfhalf({0, 0.8, 1}))
sp1:translate(2, -1, 0)
sp1:scale(0.75, 0.75, 0.75)

w = 0.3
sp2 = gr.sphere('sp2')
sp2:set_material(misc.halfhalf({1, 0, 1}))
sp2:translate(2, -1 + w/2, 0)
sp2:scale(0.7, 0.75, 0.7)

n = gr.difference(sp1, sp2)
n:rotate('x', 45)

base.ambient = {0.4, 0.4, 0.4}
base.lights = {gr.light({0, 1.2, 2}, {0.9, 0.9, 0.9}, {0, 0, 0.15})}

root:add_child(n)

root:add_child(tube)

root:add_child(wt)

cbox.cbox(root)
