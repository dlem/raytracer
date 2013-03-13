module(..., package.seeall)

white = gr.material({1, 1, 1}, {0.3, 0.3, 0.3}, 10)
black = gr.material({0, 0, 0}, {0, 0, 0}, 10)

s_body = {x=10, y=20, z=10}
s_eye = {x=1, y=2, z=1}
s_mouth = {x=2, y=2, z=1}
s_arm = {x=5, y=2, z=2}
s_eb = {x=2, y=0.8, z=1}

cs_body = gr.node('cs_ghost_body')
cs_body:translate(0, s_body.y/2, 0)

body = gr.sphere('ghost_body')
cs_body:add_child(body)
body:set_material(white)
body:translate(0, -s_body.y/2, 0)
body:scale(s_body.x, s_body.y, s_body.z)
arm1 = gr.sphere('ghost_arm1')
arm2 = gr.sphere('ghost_arm2')
cs_body:add_child(arm1)
cs_body:add_child(arm2)
arm1:set_material(white)
arm2:set_material(white)
arm1:translate(s_body.x, 0, 0)
arm2:translate(-s_body.x, 0, 0)
arm1:scale(s_arm.x, s_arm.y, s_arm.z)
arm2:scale(s_arm.x, s_arm.y, s_arm.z)

eye1 = gr.sphere('geye1')
eye2 = gr.sphere('geye2')
mouth = gr.sphere('gmouth')
cs_body:add_child(mouth)
cs_body:add_child(eye1)
cs_body:add_child(eye2)
eye1:set_material(black)
eye2:set_material(black)
mouth:set_material(black)
eye1:translate(s_body.x * 0.25, s_body.y * 0.2, s_body.z * 0.6)
eye2:translate(-s_body.x * 0.25, s_body.y * 0.2, s_body.z * 0.6)
mouth:translate(0, 0, s_body.z * 0.9)
eye1:rotate('x', -20)
eye2:rotate('x', -20)
eye1:scale(s_eye.x, s_eye.y, s_eye.z)
eye2:scale(s_eye.x, s_eye.y, s_eye.z)
mouth:scale(s_mouth.x, s_mouth.y, s_mouth.z)

eb1 = gr.cube('eb1')
eb2 = gr.cube('eb2')
cs_body:add_child(eb1)
cs_body:add_child(eb2)
eb1:set_material(black)
eb2:set_material(black)
eb1:translate(s_body.x * 0.15, s_body.y * 0.35, s_body.z * 0.7)
eb2:translate(-s_body.x * 0.15 - s_eb.x, s_body.y * 0.375, s_body.z * 0.7)
eb1:rotate('z', 20)
eb2:rotate('z', -20)
eb1:scale(s_eb.x, s_eb.y, s_eb.z)
eb2:scale(s_eb.x, s_eb.y, s_eb.z)

i = 0

function get_ghost(p, x, y, z, s)
  rv = gr.node('ghost' .. i)
  i = i + 1
  rv:add_child(cs_body)
  p:add_child(rv)
  rv:translate(x, y, z)
  rv:scale(s.x, s.y, s.z)
  return rv
end
