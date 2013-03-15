module(..., package.seeall)

function cbox(scene, w, h, name)
  scene = scene or gr.node('')
  w = w or 256
  h = h or 256
  name = name or 'cbox.png'

  root = gr.node('real_root')

  s = 1000
  c = 500
  function mkwall(x, y, z, m)
    wall = gr.cube('cwall')
    wall:set_material(m)
    wall:translate(x, y, z)
    wall:scale(s, s, s)
    box:add_child(wall)
  end
  mr = gr.material({1, 0, 0}, {0, 0, 0}, 1)
  mg = gr.material({0, 1, 0}, {0, 0, 0}, 1)
  mw = gr.material({1, 1, 1}, {0, 0, 0}, 1)

  box = gr.node('cbox')

  d = s/2
  mkwall(-s - c, -d, -d, mr)
  mkwall(c, -d, -d, mg)
  mkwall(-d, -s - c, -d, mw)
  mkwall(-d, c, -d, mw)
  mkwall(-d, -d, -s - c, mw)

  root:add_child(scene)
  root:add_child(box)

  light = gr.light({0, 300, 0}, {0.9, 0.9, 0.9}, {1, 0, 0})

  gr.render(root, name, w, h, {0, 0, 1550}, {0, 0, -1}, {0, 1, 0}, 50,
            {0.3, 0.3, 0.3}, {light})
end
