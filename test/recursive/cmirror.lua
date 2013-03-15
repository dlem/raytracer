require 'cbox'

root = gr.node('root')

mirror = gr.material({0.3, 0.3, 0.3}, {1, 1, 1}, 20)

s = 100

function msphere(x, y, z)
  sp = gr.cone('')
  sp:translate(x, y, z)
  sp:scale(s, s, s)
  sp:set_material(mirror)
  root:add_child(sp)
end

msphere(0, -400, 0)
msphere(-200, -400, -200)
msphere(200, -400, -200)
msphere(0, -400, -200)

cbox.cbox(root, 1024, 1024, 'cmirror.png')
