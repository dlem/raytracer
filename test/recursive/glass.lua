require 'cbox'

root = gr.node('root')

glass = gr.material({0.0, 0.0, 0.0}, {0.7, 1, 1}, 20)
glass:set_ri(1.5)

s = 100

function msphere(x, y, z)
  sp = gr.sphere('')
  sp:translate(x, y, z)
  sp:scale(s, s, s)
  sp:set_material(glass)
  root:add_child(sp)
end

msphere(0, -400, 0)
msphere(-200, -400, -200)
msphere(200, -400, -200)
msphere(0, -400, -200)

cbox.cbox(root, 1024, 1024, 'glass.png')
