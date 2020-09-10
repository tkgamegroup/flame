local state = {
	balls = {},
	idx = 1
}

for i=1, 5 do
	local ball = {}
	ball.e = flame_call(nil, find_udt("Entity")["create"])
	ball.e:load("assets/ball")
	ball.node = ball.e:get_component_n("cNode")
	make_obj(ball.node, "cNode")
	ball.node:set_pos({x=i*0.3-1, y=10, z=0})
	ball.rigid = ball.e:get_component_n("cRigid")
	make_obj(ball.rigid, "cRigid")
	root:add_child(ball.e.p)

	state.balls[i] = ball
end

local root_event_receiver = root:get_component_n("cEventReceiver")
make_obj(root_event_receiver, "cEventReceiver")
root_event_receiver:add_key_down_listener_s(get_slot(
	function(k)
		if (k == enums["flame::KeyboardKey"]["F"]) then
			local ball = state.balls[state.idx]
			ball.node:set_pos(camera.pos)
			ball.rigid:add_impulse({x=-camera.dir1.x*5, y=-camera.dir1.y*5, z=-camera.dir1.z*5})
			state.idx = state.idx + 1
			if state.idx == 6 then state.idx = 1 end
		end
	end
))

local trigger_rigid = entity:find_child("trigger_score_area"):get_component_n("cRigid")
make_obj(trigger_rigid, "cRigid")
trigger_rigid:add_trigger_listener_s(get_slot(
	function(type)
		if type == enums["flame::physics::TouchType"]["Found"] then
			print("score!")
		end
	end
))
