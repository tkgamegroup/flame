ITEM_SLOTS_COUNT = 10

EQUIPMENT_SLOTS_COUNT = 6
EQUIPMENT_SLOT_MAIN_HAND = 1
EQUIPMENT_SLOT_SUB_HAND = 2
EQUIPMENT_SLOT_HEAD = 3
EQUIPMENT_SLOT_CHEST = 4
EQUIPMENT_SLOT_LEG = 5
EQUIPMENT_SLOT_FOOT = 6

EQUIPMENT_SLOT_NAMES = {
	"Main Hand",
	"Sub Hand",
	"Head",
	"Chest",
	"Leg",
	"Foot"
}

ITEM_LIST = {}

local i = {
	type = "EQUIPMENT",
	name = "wooden_stick",
	display_name = "Wooden Stick",
	stack_num = 1,
	slot = EQUIPMENT_SLOT_MAIN_HAND,
	attributes = {
		ATK = 5
	}
}
ITEM_LIST[i.name] = i

local i = {
	type = "EQUIPMENT",
	name = "wooden_shield",
	display_name = "Wooden Shield",
	stack_num = 1,
	slot = EQUIPMENT_SLOT_SUB_HAND,
	attributes = {
		ARMOR = 3
	}
}
ITEM_LIST[i.name] = i

local i = {
	type = "EQUIPMENT",
	name = "leather_hat",
	display_name = "Leather Hat",
	stack_num = 1,
	slot = EQUIPMENT_SLOT_HEAD,
	attributes = {
		ARMOR = 1
	}
}
ITEM_LIST[i.name] = i

local i = {
	type = "EQUIPMENT",
	name = "leather_clothes",
	display_name = "Leather Clothes",
	stack_num = 1,
	slot = EQUIPMENT_SLOT_CHEST,
	attributes = {
		ARMOR = 2
	}
}
ITEM_LIST[i.name] = i

local i = {
	type = "EQUIPMENT",
	name = "leather_pants",
	display_name = "Leather Pants",
	stack_num = 1,
	slot = EQUIPMENT_SLOT_LEG,
	attributes = {
		ARMOR = 1
	}
}
ITEM_LIST[i.name] = i

local i = {
	type = "EQUIPMENT",
	name = "leather_shoes",
	display_name = "Leather Shoes",
	stack_num = 1,
	slot = EQUIPMENT_SLOT_FOOT,
	attributes = {
		ARMOR = 1,
		MOV_SP = 10
	}
}
ITEM_LIST[i.name] = i

item_objs = {}

function make_item_obj(entity, id, num)
	local item_obj = {
		name = entity.get_name(),
		tag = TAG_ITEM_OBJ,
		dead = false,

		entity = entity,
		node = entity.find_component("cNode"),
		pos = vec3(0),

		id = id,
		num = num
	}

	entity.set_tag(item_obj.tag)

	item_obj.die = function()
		item_objs[item_obj.name] = nil

		item_obj.entity.get_parent().remove_child(item_obj.entity)
		item_obj.dead = true
	end

	item_objs[item_obj.name] = item_obj

	return item_obj
end
