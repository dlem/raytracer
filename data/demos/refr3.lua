require 'misc'
require 'cbox'

gr.option('--hires')

w = 0.05

cbox.mb = misc.texture('redcheck', nil, 'cubefront')
--cbox.mlo = misc.texture('whitecheck', nil, 'cubetop')

s_aq = {x=2.25, y=1.0, z=2.25}
p_aq = {x=0, y=-2.2, z=0}

fullness = 0.8

aq = gr.cube('aq')
aq:set_material(misc.glass())
aq:translate(p_aq.x, p_aq.y, p_aq.z)
aq:scale(s_aq.x, s_aq.y, s_aq.z)

inside = gr.cube('inside')
inside:set_material(gr.air())
inside:translate(p_aq.x, p_aq.y + w/2, p_aq.z)
inside:scale(s_aq.x - w, s_aq.y - w/2, s_aq.z - w)

water = gr.cube('water')
mwater = misc.liquid({0.7, 0.85, 0.95})
mwater:set_bumpmap(misc.bumpmap('sinewaves', 'cubetop'))
water:set_material(mwater)
water:translate(p_aq.x, p_aq.y + w/2 + (fullness - 1) * (s_aq.y + w/2), p_aq.z)
water:scale(s_aq.x - w, fullness * (s_aq.y - w/2), s_aq.z - w)

aq = gr.union(aq, inside)
aq = gr.union(aq, water)

cbox.cbox(aq)
