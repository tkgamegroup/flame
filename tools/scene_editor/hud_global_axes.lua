local element = entity.find_component("cElement")
local proj = flame_perspective_matrix(45, 1, 1, 100)
element.add_drawer(function(canvas)
local p0 = element.get_point(0)
local p2 = element.get_point(2)
local sz = p2 - p0
p0 = p0 + sz * 0.5
local view = canvas.get_view_matrix()

local a0 = flame_mat4_transform(view, vec4(0, 0, 0, 0))
a0.z = a0.z - 5
a0 = flame_mat4_transform(proj, a0)
a0.x = a0.x / a0.w
a0.y = a0.y / a0.w
local a0_2 = vec2(a0.x, a0.y)

local ax = flame_mat4_transform(view, vec4(1, 0, 0, 0))
ax.z = ax.z - 5
ax = flame_mat4_transform(proj, ax)
ax.x = ax.x / ax.w
ax.y = ax.y / ax.w
canvas.begin_path()
canvas.move_to(a0_2 * sz + p0)
canvas.line_to(vec2(ax.x, ax.y) * sz + p0)
canvas.stroke(vec4(255, 0, 0, 255), 2, false)

local ay = flame_mat4_transform(view, vec4(0, 1, 0, 0))
ay.z = ay.z - 5
ay = flame_mat4_transform(proj, ay)
ay.x = ay.x / ay.w
ay.y = ay.y / ay.w
canvas.begin_path()
canvas.move_to(a0_2 * sz + p0)
canvas.line_to(vec2(ay.x, ay.y) * sz + p0)
canvas.stroke(vec4(0, 255, 0, 255), 2, false)

local az = flame_mat4_transform(view, vec4(0, 0, 1, 0))
az.z = az.z - 5
az = flame_mat4_transform(proj, az)
az.x = az.x / az.w
az.y = az.y / az.w
canvas.begin_path()
canvas.move_to(a0_2 * sz + p0)
canvas.line_to(vec2(az.x, az.y) * sz + p0)
canvas.stroke(vec4(0, 0, 255, 255), 2, false)
end, true)
