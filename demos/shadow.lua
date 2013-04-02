require 'misc'
require 'cbox'
require 'base'
require 'wikithing'

base.ambient = {0.3, 0.3, 0.3}

l = gr.light({0, 0, 0}, {0.9, 0.9, 0.9}, {0, 0, 0.10})
l:set_radius(0.3)
base.lights = {l}

wt = wikithing.wikithing()
wt:translate(0, 0, 0)
wt:scale(1.6, 1.6, 1.6)
wt:rotate('x', 45)

root = gr.node('root')
root:add_child(wt)

cbox.cbox(root)
