SKILL_SLOTS_COUNT = 6

SKILL_LIST = {
	{
		type = "ACTIVE",
		name = "fire_ball",
		display_name = "Fire Ball",
		cost_mana = 800,
		cool_down = 360,
		target_type = "ENEMY",
		distance = 10,
		logic = function(caster, target)
			add_projectile("fire_ball_projectile", caster.pos + vec3(0, caster.height * 0.5, 0), 0.2, function()
				target.receive_damage(caster, target, math.floor(0.5 + (caster.MAG_DMG * 10 + 300) * (math.random() * 0.2 + 0.9)))
			end)
		end
	},
	{
		type = "ACTIVE",
		name = "trample",
		display_name = "Trample",
		cost_mana = 1500,
		cool_down = 600,
		target_type = "NONE",
		logic = function(caster, target)

		end
	}
}
