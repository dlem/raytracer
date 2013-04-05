module('grad', package.seeall)

require 'misc'
require 'cbox'
require 'math'

r = 0.5
rinner = 0.45
h = 2.5
bot = 0.1

function grad(liq, fullness, bubbles)
  bubbles = bubbles or false
  cyl = gr.cylinder('grad_cyl', misc.glass())
  cyl:scale(r, h, r)
  cinner = gr.cylinder('grad_cinner', gr.air())
  cinner:set_material(gr.air())
  cinner:translate(0, bot / 2, 0)
  cinner:scale(rinner, h - bot / 2, rinner)
  rv = gr.union(cyl, cinner)
  
  if fullness > 0 then
    s = h - bot/2

    cliq = gr.cylinder('grad_liq', liq)
    cliq:scale(rinner, s * fullness, rinner)

    if bubbles then
      function bubble(x, y, z, n)
        b = gr.sphere('b')
        b:set_material(gr.air())
        b:translate(x, y, z)
        b:scale(0.1, 0.1, 0.1)
        return gr.union(n, b)
      end
      j = 0.2
      dl = 0.5
      limit = s * fullness * 2 - 0.3
      l = -limit
      while l < limit do
        cliq = bubble(2 * math.random() * j - j, l,
                      2 * math.random() * j - j, cliq)
        l = l + 0.2 + dl * (-1 + 2 * math.random());
      end
    end
    cliq:translate(0, bot / 2 - (1 - fullness) * s, 0)

    rv = gr.union(rv, cliq)
  end
  rv:translate(0, h, 0)
  return rv
end

if debug.getinfo(2) == nil then
  root = gr.node('root')
  liq = gr.material({0, 0, 0}, {1, 0.5, 1}, 20)
  liq:set_ri(1.33)
  g = grad(liq, 0.8, true)
  g:translate(0, -3.5, 0)
  g:scale(0.5, 0.5, 0.5)
  root:add_child(g)
  cbox.cbox(root)
end
