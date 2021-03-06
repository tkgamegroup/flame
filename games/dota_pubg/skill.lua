SKILL_SLOTS_COUNT = 6

SKILL_LIST = {
	{
		type = "ACTIVE",
		name = "fire_ball",
		display_name = "Fire Ball",
		cost_mana = 200,
		cool_down = 0,
		target_type = "ENEMY",
		distance = 10,
		logic = function(caster, target)
			local e = create_entity("fire_ball_projectile")
			e.set_name("projectile_"..tostring(math.random(1, 10000)))
			make_projectile(e, target, caster.pos + vec3(0, caster.height * 0.5, 0), 0.2, function()
				target.receive_damage(caster,  math.floor(0.5 + (caster.MAG_DMG * 10 + 200) * (math.random() * 0.2 + 0.9)))
			end)
			projectile_root.add_child(e)
		end
	}
}
