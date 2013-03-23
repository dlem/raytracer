module('cup', package.seeall)

require 'misc'
require 'cbox'

glass = gr.material({0, 0, 0}, {1, 1, 1}, 20)
glass:set_ri(1.5)

root = gr.node('root')

r = 0.5
r_inner = 0.3
bot = 0.1
fullness = 0.6
epsilon = 0.00001

cyl = gr.cylinder('cup_outer')
cyl:set_material(glass)
ncyl = gr.node('ncyl')
ncyl:add_child(cyl)
ncyl:scale(r, 1, r)

inner = gr.cylinder('cup_inner')
inner:set_material(gr.air())
ninner = gr.node('ninner')
ninner:add_child(inner)
ninner:translate(0, fullness, 0)
ninner:scale(r_inner, 1, r_inner)

mat_liq = gr.material({0, 0, 0}, {1, 0.4, 0.4}, 20)
mat_liq:set_ri(1.33)
liq = gr.cylinder('cup_liq')
liq:set_material(mat_liq)
nliq = gr.node('nliq')
nliq:add_child(liq)
nliq:translate(0, bot, 0)
nliq:scale(r_inner - epsilon, 1, r_inner - epsilon)

_cup = gr.difference(ncyl, nliq)
_cup = gr.difference(_cup, ninner)

function cup()
  n = gr.node('cup')
  n:add_child(_cup)
  return n
end

if(debug.getinfo(2) == nil) then
  root = gr.node('root')
  c = cup()
  c:translate(0, -1, 0)
  root:add_child(c)
  straw = gr.
  cbox.cbox(root)
end
