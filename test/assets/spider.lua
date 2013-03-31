module('spider', package.seeall)

require 'misc'
require 'cbox'

mtorso = misc.black()
mlegs = misc.black()

srleg = 0.005
sfleg = 0.2
sbleg = 0.15
rtorso = 0.04
wtorso = 0.02

function mkleg()
  fleg = gr.cylinder('spider_fleg')
  fleg:set_material(mlegs)
  fleg:translate(0, -sfleg/2, 0)
  fleg:scale(srleg, sfleg/2, srleg)
  n = gr.node('')
  n:add_child(fleg)
  n:translate(0, -sbleg/2, 0)
  n:rotate('x', 90)

  cs_bleg = gr.node('cs_bleg')
  cs_bleg:translate(0, -sbleg/2, 0)
  cs_bleg:add_child(n)

  bleg = gr.cylinder('spider_bleg')
  bleg:set_material(mlegs)
  cs_bleg:add_child(bleg)
  bleg:scale(srleg, sbleg/2, srleg)

  n = gr.node('')
  n:add_child(cs_bleg)
  return n
end

function spider()
  torso = gr.sphere('spider_torso')
  torso:set_material(mtorso)
  torso:scale(rtorso, wtorso, rtorso)

  leg = mkleg()
  leg:translate(0, 0, rtorso)
  leg:rotate('x', -120)

  rv = gr.node('spider')
  rv:add_child(torso)

  function add_leg(angle)
    l = gr.node('sleg')
    l:add_child(leg)
    l:rotate('y', angle)
    rv:add_child(l)
  end

  for i = 0, 3 do
    add_leg(45 + i * 90 / 3)
    add_leg(-45 - i * 90 / 3)
  end

  return rv
end

if debug.getinfo(2) == nil then
  root = gr.node('root')
  s = spider()
  s:translate(0, -1, 0)
  s:scale(2, 2, 2)
  cbox.cbox(s)
end
