SKILL_SLOTS_COUNT = 6

MAX_AOE_NUM = 16

SKILL_LIST = {}

local s = {
	type = "Active",
	name = "fire_ball",
	display_name = "Fire Ball",
	cost_mana = 800,
	cool_down = 360,
	target_type = "Enemy",
	distance = 10,
	description = "Shot a fire ball to an enemy unit, inflict 30 damage",
	logic = function(caster, target)
		if caster.tag == target.tag then return end
		add_projectile("fire_ball", target, caster.pos + vec3(0, caster.height * 0.5, 0), 0.2, function()
			caster.inflict_damage(target, "Magical", math.floor(0.5 + 300 * (math.random() * 0.2 + 0.9)))
		end)
	end
}
SKILL_LIST[s.name] = s

local s = {
	type = "Active",
	name = "ice_bolt",
	display_name = "Ice Bolt",
	cost_mana = 800,
	cool_down = 360,
	target_type = "Enemy",
	distance = 10,
	description = "Shot a ice bolt to an enemy unit, inflict 24 damage and reduce its move speed by 20% in 5 sec",
	logic = function(caster, target)
		if caster.tag == target.tag then return end
		add_projectile("ice_bolt", target, caster.pos + vec3(0, caster.height * 0.5, 0), 0.2, function()
			caster.inflict_damage(target, "Magical", math.floor(0.5 + 240 * (math.random() * 0.2 + 0.9)))
			target.receive_buff(caster, "ice_bolt")
		end)
	end
}
SKILL_LIST[s.name] = s

local s = {
	type = "Active",
	name = "trample",
	display_name = "Trample",
	cost_mana = 1500,
	cool_down = 600,
	target_type = "None",
	range = 5,
	description = "Damaging nearby enemy units by 15 damage",
	logic = function(caster, target)
		add_particle("trample", caster.pos + vec3(0.0, 0.8, 0.0), 20)

		local arr = flame_malloc(MAX_AOE_NUM * 8)
		local n = obj_root_n.get_within_circle(caster.pos.to_flat(), 5, arr, MAX_AOE_NUM, caster.enemy_tag)
		for i=0, n-1, 1 do
			local name = make_entity(flame_get_p(arr, i * 8)).get_name()
			local tar = characters[caster.enemy_tag][name]
			caster.inflict_damage(tar, "Physical", math.floor(0.5 + 150 * (math.random() * 0.2 + 0.9)))
		end
		flame_free(arr)
	end
}
SKILL_LIST[s.name] = s
