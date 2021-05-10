local element = entity.find_component("cElement")
element.add_drawer(function(canvas)
local p0 = element.get_point(0)
local p2 = element.get_point(2)
local sz = p2 - p0
p0 = p0 + sz * 0.5
canvas.begin_path()
canvas.move_to(p0 - vec2(4, 0))
canvas.line_to(p0 + vec2(4, 0))
canvas.move_to(p0 - vec2(0, 4))
canvas.line_to(p0 + vec2(0, 4))
canvas.stroke(vec4(255, 0, 0, 255), 1, false)
end, true)
