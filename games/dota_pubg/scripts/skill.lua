SKILL_SLOTS_COUNT = 6

SKILL_LIST = {}

local s = {
	type = "ACTIVE",
	name = "fire_ball",
	display_name = "Fire Ball",
	cost_mana = 800,
	cool_down = 360,
	target_type = "ENEMY",
	distance = 10,
	description = "Shot a fire ball to an enemy unit, deal 30 damage",
	logic = function(caster, target)
		add_projectile("fire_ball", target, caster.pos + vec3(0, caster.height * 0.5, 0), 0.2, function()
			target.receive_damage(caster, math.floor(0.5 + (caster.MAG_DMG * 10 + 300) * (math.random() * 0.2 + 0.9)))
		end)
	end
}
SKILL_LIST[s.name] = s

local s = {
	type = "ACTIVE",
	name = "trample",
	display_name = "Trample",
	cost_mana = 1500,
	cool_down = 600,
	target_type = "NONE",
	description = "Damaging nearby enemy units by 15 damage",
	logic = function(caster, target)

	end
}
SKILL_LIST[s.name] = s
