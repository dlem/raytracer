module(..., package.seeall)

script_name = debug.getinfo(3).short_src

pang_name, _n = script_name:gsub('.lua', '.png')
if _n <= 0 then png_name = nil end

function cone(name)
  name = name or ''
  _c = gr.cone(name)
  _n = gr.node(name)
  _n:add_child(_c)
  return _c
end

function cylinder(name)
  name = name or ''
  _c = gr.cylinder(name)
  _n = gr.node(name)
  _n:add_child(_c)
  return _c
end

function glass()
  _g = gr.material({0, 0, 0}, {0.95, 0.95, 0.95}, 20)
  _g:set_ri(1.5)
  return _g
end

function red()
  return gr.material({1, 0, 0}, {0, 0, 0}, 20)
end

function green()
  return gr.material({0, 1, 0}, {0, 0, 0}, 20)
end

function blue()
  return gr.material({0, 0, 1}, {0, 0, 0}, 20)
end

function white()
  return gr.material({1, 1, 1}, {0, 0, 0}, 20)
end

function mirror()
  return gr.material({0, 0, 0}, {0.95, 0.95, 0.95})
end

function black()
  return gr.material({0, 0, 0}, {0, 0, 0})
end
