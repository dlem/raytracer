module(..., package.seeall)

script_name = debug.getinfo(3).short_src

png_name, _n = script_name:gsub('.lua', '.png')
if _n <= 0 then png_name = nil end
