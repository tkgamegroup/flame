local trigger_rigid = entity:find_child("trigger_score_area"):get_component_n("cRigid")
make_obj(trigger_rigid, "cRigid")
trigger_rigid:add_trigger_listener_s(get_slot(
	function(type)
		if type == enums["flame::physics::TouchType"]["Found"] then
			print("score!")
		end
	end
))
