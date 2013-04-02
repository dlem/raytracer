require 'misc'
require 'cbox'
require 'base'
require 'cup'

base.ambient = {0.1, 0.1, 0.1}

c = cup.cup(0.7, false, misc.liquid({0.4, 0.9, 0.96}))

c:translate(0, -2, 0)

gr.option('--caustic-num-photons 8000000 --caustic-num-neighbours 100')

cbox.cbox(c)
