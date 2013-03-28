module(..., package.seeall)

script_name = debug.getinfo(3).short_src

png_name, _n = script_name:gsub('.lua', '.png')
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

