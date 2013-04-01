module(..., package.seeall)

if(true) then
  require "macho_cow"
  cow = macho_cow
else
  require "simple_cow"
  cow = simple_cow
end

require "tree"
require "ghost"

function scale(n, s)
  n:scale(s.x, s.y, s.z)  
end

function sphere(n, p, m, x, y, z, s)
  rv = gr.sphere(n)
  p:add_child(rv)
  rv:set_material(m)
  rv:translate(x, y, z)
  scale(rv, s)
  return rv
end

function cube(n, p, m, x, y, z, s)
  rv = gr.cube(n)
  p:add_child(rv)
  rv:set_material(m)
  rv:translate(x, y, z)
  scale(rv, s)
  return rv
end

function zsphere(n, p, m, s) return sphere(n, p, m, 0, 0, 0, s) end
function zcube(n, p, m, s) return cube(n, p, m, 0, 0, 0, s) end

function cs(n, p, x, y, z)
  rv = gr.node(n)
  p:add_child(rv)
  rv:translate(x, y, z)
  return rv
end

root = gr.node('root')
dirt = gr.material({148/255, 96/255, 31/255}, {0, 0, 0}, 10)
grass = gr.material({44/255, 242/255, 5/255}, {0, 0, 0}, 10)
water = gr.material({0/255, 72/255, 217/255}, {0.5, 0.5, 0.5}, 10)

s_mesa = {x=1200, y=150, z=2000}
s_mesatop = {x=s_mesa.x, y=5, z=s_mesa.z}
s_ground = {x=s_mesa.x, y=5, z=s_mesa.z}
s_tree = {x=1, y=1, z=1}
s_ghost = {x=1, y=1, z=1}

cs_mesa = cs('cs_mesa', root, -s_mesa.x/2, 0, 0)
cs_mesatop = cs('cs_mesatop', cs_mesa, 0, s_mesa.y + s_mesatop.y, 0)
cs_top = cs('cs_top', root, -200, 155, 0)

mesa = cube('mesa', cs_mesa, dirt, -s_mesa.x/2, 0, -s_mesa.z/2, s_mesa)
mesatop = cube('mesatop', cs_mesatop, grass, -s_mesatop.x/2, -s_mesatop.y, -s_mesatop.z/2, s_mesatop)

cs_floor = cs('cs_floor', root, s_ground.x/2, 0, 0)
cs_water = cs('cs_water', cs_floor, 0, -24, 0)
ground = cube('ground', cs_floor, water, -s_ground.x/2, -s_ground.y, -s_ground.z/2, s_ground)

cs_bot = cs('cs_bot', root, 200, -24, 0)

tree.get_tree(cs_top, 100, 0, 200, s_tree)
tree.get_tree(cs_top, 100, 0, 300, s_tree)
tree.get_tree(cs_top, -100, 0, 200, s_tree)
tree.get_tree(cs_top, -100, 0, -200, s_tree)
tree.get_tree(cs_top, -200, 0, -300, s_tree)
tree.get_tree(cs_top, -400, 0, -300, s_tree)
tree.get_tree(cs_top, -800, 0, -600, s_tree)
tree.get_tree(cs_top, -800, 0, -300, s_tree)
tree.get_tree(cs_top, -600, 0, 200, s_tree)
tree.get_tree(cs_top, -600, 0, 400, s_tree)

g1 = ghost.ghost()
g1:translate(-200, 0, 0)
g1:scale(10 * s_ghost.x, 10 * s_ghost.y, 10 * s_ghost.z)
g1:rotate('y', 90)
cs_top:add_child(g1)

root:translate(100, -100, 0)
root:rotate('y', -90)
root:rotate('x', 40)
root:rotate('z', -20)

c1 = cow.get_cow(cs_top, 70, 0, -70, 1)
c1:translate(0, 3, 0)
c1:rotate('z', 0)
c3 = cow.get_cow(cs_top, 30, 0, 100, 1)
c3:translate(0, 6, 0)
c3:rotate('y', -20)
c3:rotate('z', -15)
c4 = cow.get_cow(cs_top, 160, 0, -150, 1)
c4:translate(0, 6, 0)
c4:rotate('y', 20)
c4:rotate('z', 15)
c5 = cow.get_cow(cs_top, 160, 0, 100, 1)
c5:translate(0, 5, 0)
c5:rotate('y', -15)
c5:rotate('z', 10)
c6 = cow.get_cow(cs_top, 200, 0, 0, 1)
c6:translate(0, 6, 0)
c6:rotate('z', 15)
c7 = cow.get_cow(cs_top, 250, -20, -100, 1)
c7:rotate('z', -45)
c8 = cow.get_cow(cs_top, 300, -60, 100, 1)
c8:rotate('z', -150)
c9 = cow.get_cow(cs_bot, 0, 0, 0, 1)
c10 = cow.get_cow(cs_bot, 60, 0, -100, 1)
c11 = cow.get_cow(cs_bot, 100, 0, 100, 1)

gr.option('--rays')
gr.option('--unit-distance 2000')

function render(name)
  white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {0.75, 0, 0.5})
  --orange_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

  gr.render(root, name, 1024, 1024,
            {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
            {0.3, 0.3, 0.3}, {white_light})
end
