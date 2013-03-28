require 'misc'
require 'cbox'

mat = gr.material({0, 0, 0}, {0.7, 0.85, 0.95}, 20)
mat:set_ri(1.33)

water = gr.cube('water')
water:set_material(mat)
water:translate(0, -2, 0);
water:scale(0.5, 0.5, 0.5)

ripple = gr.bumpmap('sinewaves')
mat:set_bumpmap(ripple)

cbox.cbox(water)
