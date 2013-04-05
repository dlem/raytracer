require 'misc'
require 'cbox'
require 'cup'

--cbox.mb:set_texture(gr.texture('whitecheck'))

liq = misc.liquid({0, 0.8, 0.9})

root = gr.node('root')

cb = gr.cube('cb')
cb:set_material(misc.texture('whitecheck'))
cb:translate(-0.6, -0.8, -2)
cb:scale(0.5, 0.5, 0.5)
root:add_child(cb)

c = cup.cup(0.8, false, liq, true)
c:translate(0, -1.5, 0)
--c:rotate('x', 30)
root:add_child(c)

cbox.cbox(root)
