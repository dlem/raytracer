module(..., package.seeall)

require 'os'

script_name = string.sub(debug.getinfo(3).source, 2)
script_name = string.match(script_name, ".*[/\\]([^/\\]*)") or script_name

png_name, _n = script_name:gsub('.lua', '.png')
if _n <= 0 then png_name = nil end

function glass()
  _g = gr.material({0, 0, 0}, {0.95, 0.95, 0.95}, 20)
  _g:set_ri(1.5)
  return _g
end

function halfhalf(c)
  clr = {c[1]/2,c[2]/2,c[3]/2}
  return gr.material(clr, clr, 10)
end

function red()
  return gr.material({1, 0, 0}, {0, 0, 0}, 10)
end

function green()
  return gr.material({0, 1, 0}, {0, 0, 0}, 10)
end

function blue()
  return gr.material({0, 0, 1}, {0, 0, 0}, 10)
end

function pink()
  return gr.material({0.9, 0.2, 0.1}, {0, 0, 0}, 10)
end

function white()
  return gr.material({1, 1, 1}, {0, 0, 0}, 10)
end

function mirror()
  m = gr.material({0, 0, 0}, {0.95, 0.95, 0.95}, 30)
  m:set_reflective()
  return m
end

function black()
  return gr.material({0, 0, 0}, {0.3, 0.3, 0.3}, 20)
end

function liquid(spec)
  liq = gr.material({0, 0, 0}, spec, 30)
  liq:set_ri(1.33)
  return liq
end

function solid(diff)
  sol = gr.material(diff, {0, 0, 0}, 30)
  return sol
end

function asset(name)
  asset_path = os.getenv('ASSET_PATH')
  if asset_path == nil then
    asset_path = 'assets'
  end
  return (os.getenv('ASSET_PATH') or 'assets') .. '/' .. name
end

function bumpmap(sbm, rm)
  bm = gr.bumpmap(sbm)
  if rm ~= nil then bm:remap(rm) end
  return bm
end

function texture(s, sbm, rm)
  m = gr.material({0, 0, 0}, {0, 0, 0}, 30)

  if s ~= nil then
    tex = gr.texture(s)
    if rm ~= nil then tex:remapt(rm) end
    m:set_texture(tex)
  end

  if sbm ~= nil then m:set_bumpmap(bumpmap(sbm, rm)) end

  return m
end
