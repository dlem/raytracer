require 'cbox'

root = gr.node('root')

glass = gr.material({0.3, 0.3, 0.3}, {1, 1, 1}, 20)
glass:set_opacity(0, 1.5)

s = 100

function msphere(x, y, z)
  sp = gr.cylinder('')
  sp:translate(x, y, z)
  sp:scale(s, s, s)
  sp:set_material(glass)
  root:add_child(sp)
end

msphere(0, -400, 0)
--msphere(-200, -400, -200)
--msphere(200, -400, -200)
--msphere(0, -400, -200)

cbox.cbox(root, 1024, 1024, 'cylglass.png')
