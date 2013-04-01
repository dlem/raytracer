module('holder', package.seeall)

require 'misc'
require 'cbox'
require 'ttube'

holder_mat = gr.material({0.2, 0.2, 0.2}, {0, 0, 0}, 30)

sh = 0.2

tl = 4
th = 2 
tw = 0.7

phole = 0.85

ssides = 0.2

function holder()
  root = gr.node('holder')

  cs_top = gr.node('cs_top')
  cs_top:translate(0, th - sh/2, 0)
  root:add_child(cs_top)

  topr = gr.cube('topr')
  topr:set_material(holder_mat)
  topr:scale(tl/2, sh/2, tw/2)

  cs_bot = gr.node('cs_bot')
  cs_bot:translate(0, sh/2, 0)
  root:add_child(cs_bot)

  botr = gr.cube('botr')
  botr:set_material(holder_mat)
  botr:scale(tl/2, sh/2, tw/2)

  left = misc.cylinder('left')
  root:add_child(left)
  left:set_material(holder_mat)
  left:translate(-tl/2 + ssides, th/2, 0)
  left:scale(ssides, th/2, ssides)

  right = misc.cylinder('right')
  root:add_child(right)
  right:set_material(holder_mat)
  right:translate(tl/2 - ssides, th/2, 0)
  right:scale(ssides, th/2, ssides)

  function add_socket(cx, tube, liq, fullness)
    fullness = fullness or 0

    s = gr.sphere('s')
    s:set_material(holder_mat)
    s:translate(-tl/2 + cx * tl, 0, 0)
    r = phole * tw/2
    s:scale(r, r, r)
    topr = gr.difference(topr, s)

    if tube then
      flip = fullness == 0
      tt = ttube.ttube(liq, fullness, flip)
      tt:translate(-tl/2 + cx * tl, sh, 0)
      tt:scale(0.8, 1, 0.8)
      cs_bot:add_child(tt)
    end
  end
  
  add_socket(0.2, true)
  add_socket(0.4, true, misc.liquid({1, 0.3, 0.5}), 0.9)
  add_socket(0.6, true, misc.liquid({0.3, 0.8, 0.95}), 0.7)
  add_socket(0.8, true, misc.liquid({0.1, 0.97, 0.1}), 0.5)

  cs_top:add_child(topr)
  cs_bot:add_child(botr)

  root:translate(0, th/2, 0)
  n = gr.node('')
  n:add_child(root)
  return n
end

if debug.getinfo(2) == nil then
  h = holder()
  h:translate(0, -3, 0)
  h:rotate('x', 10)
  cbox.cbox(h)
end
