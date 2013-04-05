module('ring', package.seeall)

require 'misc'
require 'cbox'

mring = gr.material({0, 0, 0}, {232/255, 194/255, 90/255}, 30)
mring:set_reflective()

function ring()
  w = 0.03
  s = {x=0.6, y=0.1, z=0.6}

  co = gr.cylinder('co', mring)
  ci = gr.cylinder('ci', mring)
  co:scale(s.x, s.y, s.z)
  ci:scale(s.x - w, s.y * 1.5, s.z - w)

  rv = gr.difference(co, ci)
  return rv
end

if debug.getinfo(2) == nil then
  r = ring()
  r:translate(0, 0, -2.5)
  r:rotate('x', 90)
  r:scale(4, 6, 4)
  gr.option('--caustic-num-photons 8000000 --caustic-num-neighbours 100')
  cbox.cbox(r)
end
