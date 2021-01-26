local trigger_rigid = entity.find_child("trigger_score_area").find_component("cRigid")
trigger_rigid.add_trigger_listener_s(function(type)
	if type == enums["flame::physics::TouchType"]["Found"] then
		print("score!")
	end
end)
