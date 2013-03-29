require 'misc'
require 'cbox'

mat = gr.material({0, 0, 0}, {0.7, 0.85, 0.95}, 20)
mat:set_ri(1.33)

bar = gr.material({0, 0, 0}, {0, 0, 0}, 20)
tex = gr.texture('testtex')
tex:remapt('cyltop')
bar:set_texture(tex)

water = gr.cube('water')
water:set_material(mat)
water:translate(0, -2, 0);
water:scale(0.5, 0.5, 0.5)
water:rotate('x', 90)

ripple = gr.bumpmap('sinewaves')
ripple:remap('cubetop')
mat:set_bumpmap(ripple)

m2 = gr.material({0, 0, 0}, {0.9, 0.90, 0.95}, 20)
m2:set_ri(1.33)
r2 = gr.bumpmap('sinewaves')
r2:remap('cyltop')
m2:set_bumpmap(r2)
cyl = misc.cylinder('')
cyl:set_material(m2)
cyl:translate(0, -1, 0)
cyl:rotate('y', -30)
cyl:rotate('x', 90)
root = gr.node('')
--root:add_child(water)
root:add_child(cyl)

cbox.cbox(root)
