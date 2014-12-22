require 'misc'
require 'cbox'
require 'paper'

p = paper.paper(misc.asset('graph1.png'), 8.5, 11)
p:translate(0, -1, 1.5)
p:rotate('x', 45)
p:rotate('y', 10)
p:scale(1, 1, 1)

cbox.cbox(p)
