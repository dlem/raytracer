module('graph1', package.seeall)

require 'misc'
require 'base'
require 'paper'

function graph1()
  return paper.paper('../graph1.png', 8.5, 11)
end

if debug.getinfo(2) == nil then
  n = graph1()
  n:translate(0, -2, 0)
  n:rotate('x', 75)
  base.render(n, 512, 512)
end
