require 'misc'
require 'cbox'

cbox.mb = misc.texture('../test/fog.png', '../test/fog_bm.png', 'cubefront')
--cbox.mt = misc.texture('../test/bwgrid.png', '../test/bwgrid_bm.png', 'cubebot')
--cbox.mlo = gr.material({0, 0.8, 0.95}, {0, 0, 0}, 20)
--cbox.mlo:set_bumpmap(misc.bumpmap('sinewaves', 'cubetop'))

sph = gr.sphere('sph')
sph:set_material(misc.texture('../test/bwgrid.png', '../test/bwgrid_bm.png'))
sph:translate(-1.5, 0, 1)
sph:scale(0.75, 0.75, 0.75)

cyl = misc.cylinder('cyl')
mcyl = gr.material({0, 0.8, 0.95}, {0, 0, 0}, 20)
mcyl:set_bumpmap(misc.bumpmap('sinewaves', 'cyltop'))
cyl:set_material(mcyl)
cyl:translate(1.6, -0.5, 0.6)
cyl:rotate('x', 50)
cyl:rotate('z', 25)
cyl:scale(0.5, 0.3, 0.5)

cone = misc.cone('cone')
cone:set_material(misc.texture('../test/cloth.png', '../test/cloth_bm.png'))
cone:translate(0,-1, -0.5)
cone:scale(2, 3, 1)

root = gr.node('root')
root:add_child(sph)
root:add_child(cyl)
root:add_child(cone)

cbox.cbox(root)
