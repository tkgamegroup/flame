SKILL_SLOTS_COUNT = 6

SKILL_LIST = {
	{
		type = "ACTIVE",
		name = "fire_ball",
		display_name = "Fire Ball",
		data = {
			cast_mana = 80,
			cool_down = 180,
			target_type = "ENEMY",
			distance = 10,
			logic = function(caster, target)
				local e = create_entity("fire_ball_projectile")
				e.set_name("projectile_"..tostring(math.random(1, 10000)))
				make_projectile(e, target).node.set_pos(caster.pos)
			end
		}
	}
}
