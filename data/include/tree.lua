module(..., package.seeall)

leaves = gr.material({26/255, 140/255, 3/255}, {0, 0, 0}, 10)
bark = gr.material({64/255, 54/255, 1/255}, {0, 0, 0}, 10)

s_campus = {x=38, y=50, z=38}
s_trunk = {x=20, y=70, z=20}

tree = gr.node('tree')
campus = gr.sphere('campus')
campus:set_material(leaves)
campus:translate(0, 0.7 * (s_trunk.y + s_campus.y), 0)
campus:scale(s_campus.x, s_campus.y, s_campus.z)
tree:add_child(campus)
trunk = gr.cube('trunk')
trunk:translate(0, 0, 0)
trunk:set_material(bark)
trunk:scale(s_trunk.x, s_trunk.y, s_trunk.z)
tree:add_child(trunk)

i = 0

function get_tree(p, x, y, z, s)
  rv = gr.node('tree' .. i)
  i = i + 1
  p:add_child(rv)
  rv:add_child(tree)
  rv:translate(x, y, z)
  rv:scale(s.x, s.y, s.z)
  return rv
end
