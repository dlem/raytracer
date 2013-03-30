module('cup', package.seeall)

-- 1M photons and 100 neighbours seems to work.

require 'misc'
require 'cbox'


r = 0.7
r_inner = 0.6
bot = 0.1

function cup(fullness, ripple, mat_liq, straw)
  straw = straw or false

  n = gr.node('cup')

  cyl = gr.cylinder('cup_outer')
  cyl:set_material(misc.glass())
  ncyl = gr.node('ncyl')
  ncyl:add_child(cyl)
  ncyl:scale(r, 1, r)

  inner = gr.cylinder('cup_inner')
  inner:set_material(gr.air())
  ninner = gr.node('ninner')
  ninner:add_child(inner)
  ninner:translate(0, bot * 0.5, 0)
  ninner:scale(r_inner, 1 - bot * 0.5, r_inner)

  liq = gr.cylinder('cup_liq')
  liq:set_material(mat_liq)
  nliq = gr.node('nliq')
  nliq:add_child(liq)
  nliq:translate(0, bot * 0.5 - (1 - fullness), 0)
  nliq:scale(r_inner, (1 - bot * 0.5) * fullness, r_inner)

  if(ripple) then
    bm = gr.bumpmap('sinewaves')
    bm:remap('cyltop')
    mat_liq:set_bumpmap(bm)
  end

  if(straw) then
    straw = misc.cylinder('straw')
    straw:set_material(gr.material({0.3, 0.3, 0.3}, {0, 0, 0}, 30))
    straw:translate(0, 0.3, 0)
    straw:rotate('z', -25)
    straw:scale(0.03, 0.8, 0.03)
    n:add_child(straw)
  end

  _cup = gr.union(ncyl, ninner)
  _cup = gr.union(_cup, nliq)

  n:add_child(_cup)
  n:translate(0, 1.01, 0)
  return n
end

if(debug.getinfo(2) == nil) then
  gr.option("--caustic-num-photons 1000000")
  gr.option("--caustic-num-neighbours 100")

  root = gr.node('root')
  c = cup(0.6, true, misc.liquid({0.5, 0.95, 0.95}), true)
  c:translate(0, -1.5, 0)
  c:rotate('x', 45)
  root:add_child(c)
  cbox.cbox(root, 256, 256)
end
