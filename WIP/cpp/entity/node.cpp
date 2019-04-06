#include <flame/global.h>
#include <flame/engine/core/core.h>
#include <flame/engine/entity/node.h>

namespace flame
{
	bool Node::broadcast_upward(Object *src, Message msg)
	{
		if (Object::broadcast(src, msg))
			return true;
		if (parent)
			return parent->broadcast_upward(src, msg);
		return false;
	}

	Node::Node(NodeType _type) :
		coord(0.f), 
		euler(0.f), 
		quat(0.f, 0.f, 0.f, 1.f), 
		scale(1.f),
		axis(1.f),
		matrix(1.f),
		axis_dirty(false),
		euler_dirty(false),
		quat_dirty(false),
		matrix_dirty(false),
		transform_dirty_frame(0),
		type(_type),
		parent(nullptr)
	{
	}

	const glm::vec3 &Node::get_coord() const
	{
		return coord;
	}

	const glm::mat3 &Node::get_axis()
	{
		if (axis_dirty)
			update_axis();
		return axis;
	}

	const glm::vec3 &Node::get_scale() const
	{
		return scale;
	}

	const glm::vec3 &Node::get_euler()
	{
		if (euler_dirty)
			update_euler();
		return euler;
	}

	const glm::vec4 &Node::get_quat()
	{
		if (quat_dirty)
			update_quat();
		return quat;
	}

	const glm::mat4 &Node::get_matrix()
	{
		if (matrix_dirty)
			update_matrix();
		return matrix;
	}

	glm::mat4 Node::get_world_matrix()
	{
		auto m = get_matrix();
		if (parent)
			m = parent->get_world_matrix() * m;
		return m;
	}

	glm::vec3 Node::get_world_coord()
	{
		return glm::vec3(get_world_matrix()[3]);
	}

	glm::mat3 Node::get_world_axis()
	{
		auto a = get_axis();
		if (parent)
			a = parent->get_world_axis() * a;
		return a;
	}

	void Node::set_coord(const glm::vec3 &_coord)
	{
		coord = _coord;
		mark_coord_setted();
	}

	void Node::set_coord(float x, float y, float z)
	{
		coord.x = x;
		coord.y = y;
		coord.z = z;
		mark_coord_setted();
	}

	void Node::set_coord(float v)
	{
		coord.x = v;
		coord.y = v;
		coord.z = v;
		mark_coord_setted();
	}

	void Node::set_coord_x(float v)
	{
		coord.x = v;
		mark_coord_setted();
	}

	void Node::set_coord_y(float v)
	{
		coord.y = v;
		mark_coord_setted();
	}

	void Node::set_coord_z(float v)
	{
		coord.z = v;
		mark_coord_setted();
	}

	void Node::add_coord(const glm::vec3 &_coord)
	{
		coord += _coord;
		mark_coord_setted();
	}

	void Node::add_coord(float x, float y, float z)
	{
		coord.x += x;
		coord.y += y;
		coord.z += z;
		mark_coord_setted();
	}

	void Node::add_coord(float v)
	{
		coord.x += v;
		coord.y += v;
		coord.z += v;
		mark_coord_setted();
	}

	void Node::add_coord_x(float v)
	{
		coord.x += v;
		mark_coord_setted();
	}

	void Node::add_coord_y(float v)
	{
		coord.y += v;
		mark_coord_setted();
	}

	void Node::add_coord_z(float v)
	{
		coord.z += v;
		mark_coord_setted();
	}

	void Node::set_euler(const glm::vec3 &_euler)
	{
		euler = glm::mod(_euler, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler(float x, float y, float z)
	{
		euler.x = glm::mod(x, 360.f);
		euler.y = glm::mod(y, 360.f);
		euler.z = glm::mod(z, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler(float v)
	{
		v = glm::mod(v, 360.f);
		euler.x = v;
		euler.y = v;
		euler.z = v;
		mark_euler_setted();
	}

	void Node::set_euler_x(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler_y(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.y = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Node::set_euler_z(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.z = glm::mod(v, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler(const glm::vec3 &_euler)
	{
		if (euler_dirty)
			update_euler();
		euler += _euler;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler(float x, float y, float z)
	{
		if (euler_dirty)
			update_euler();
		euler.x += x;
		euler.y += y;
		euler.z += z;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x += v;
		euler.y += v;
		euler.z += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler_x(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.x += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler_y(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.y += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::add_euler_z(float v)
	{
		if (euler_dirty)
			update_euler();
		euler.z += v;
		euler = glm::mod(euler, 360.f);
		mark_euler_setted();
	}

	void Node::set_quat(const glm::vec4 &_quat)
	{
		quat = _quat;
		mark_quat_setted();
	}

	void Node::set_quat(float x, float y, float z, float w)
	{
		quat.x = x;
		quat.y = y;
		quat.z = z;
		quat.w = w;
		mark_quat_setted();
	}

	void Node::set_axis(const glm::mat3 &_axis)
	{
		axis = _axis;
		mark_axis_setted();
	}

	void Node::left_rotate(const glm::mat3 &left)
	{
		axis = left * axis;
		mark_axis_setted();
	}

	void Node::right_rotate(const glm::mat3 &right)
	{
		axis = axis * right;
		mark_axis_setted();
	}

	void Node::axis_rotate(Axis which, float angle)
	{
		switch (which)
		{
			case AxisX:
			{
				auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[0]));
				axis[1] = glm::normalize(m * axis[1]);
				axis[2] = glm::normalize(m * axis[2]);
				break;
			}
			case AxisY:
			{
				auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[1]));
				axis[0] = glm::normalize(m * axis[0]);
				axis[2] = glm::normalize(m * axis[2]);
				break;
			}
			case AxisZ:
			{
				auto m = glm::mat3(glm::rotate(glm::radians(angle), axis[2]));
				axis[1] = glm::normalize(m * axis[1]);
				axis[0] = glm::normalize(m * axis[0]);
				break;
			}
		}
		mark_axis_setted();
	}

	void Node::set_scale(const glm::vec3 &_scale)
	{
		scale = _scale;
		mark_scale_setted();
	}

	void Node::set_scale(float x, float y, float z)
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;
		mark_scale_setted();
	}

	void Node::set_scale(float v)
	{
		scale.x = v;
		scale.y = v;
		scale.z = v;
		mark_scale_setted();
	}

	void Node::set_scale_x(float v)
	{
		scale.x = v;
		mark_scale_setted();
	}

	void Node::set_scale_y(float v)
	{
		scale.y = v;
		mark_scale_setted();
	}

	void Node::set_scale_z(float v)
	{
		scale.z = v;
		mark_scale_setted();
	}

	void Node::add_scale(const glm::vec3 &_scale)
	{
		scale += _scale;
		mark_scale_setted();
	}

	void Node::add_scale(float x, float y, float z)
	{
		scale.x += x;
		scale.y += y;
		scale.z += z;
		mark_scale_setted();
	}

	void Node::add_scale(float v)
	{
		scale.x += v;
		scale.y += v;
		scale.z += v;
		mark_scale_setted();
	}

	void Node::add_scale_x(float v)
	{
		scale.x += v;
		mark_scale_setted();
	}

	void Node::add_scale_y(float v)
	{
		scale.y += v;
		mark_scale_setted();
	}

	void Node::add_scale_z(float v)
	{
		scale.z += v;
		mark_scale_setted();
	}

	void Node::scale_relate(Node *t)
	{
		coord *= t->scale;
		coord *= t->scale;
		axis = t->axis * axis;
		coord = t->axis * coord;
		coord += t->coord;
		matrix_dirty = true;

		transform_dirty_frame = total_frame_count;
	}

	void Node::relate(Node *t)
	{
		coord -= t->coord;
		axis *= glm::transpose(t->axis);
		matrix_dirty = true;

		transform_dirty_frame = total_frame_count;
	}

	void Node::update_matrix()
	{
		if (axis_dirty)
			update_axis();
		matrix = glm::translate(coord) * glm::mat4(axis) * glm::scale(scale);
		matrix_dirty = false;
	}

	void Node::update_axis()
	{
		if (!quat_dirty)
			axis = quaternion_to_mat3(quat);// update by quat
		else
			axis = euler_to_mat3(euler);// update by euler
		axis_dirty = false;
	}

	void Node::update_euler()
	{
		if (quat_dirty)
			update_quat(); // updata by quat

		euler = quaternion_to_euler(quat);
		euler_dirty = false;
	}

	void Node::update_quat()
	{
		if (axis_dirty)
			update_axis(); // update by axis
		quat = mat3_to_quaternion(axis);
		quat_dirty = false;
	}

	long long Node::get_transform_dirty_frame()
	{
		return transform_dirty_frame;
	}

	void Node::update()
	{
		for (auto &c : children)
			c->update();
		for (auto &c : components)
			c->on_update();
		on_update();
	}

	NodeType Node::get_type() const
	{
		return type;
	}

	Node *Node::get_parent() const
	{
		return parent;
	}

	const std::vector<std::unique_ptr<Node>> &Node::get_children() const
	{
		return children;
	}

	const std::vector<std::unique_ptr<Component>> &Node::get_components() const
	{
		return components;
	}

	void Node::add_child(Node *n)
	{
		n->parent = this;
		children.emplace_back(n);
		broadcast_upward(n, MessageNodeAdd);
		component_boardcast(n, MessageComponentAdd);
	}

	void Node::remove_child(Node *n)
	{
		for (auto it = children.begin(); it != children.end(); it++)
		{
			if (it->get() == n)
			{
				component_boardcast(n, MessageComponentRemove);
				broadcast_upward(n, MessageNodeRemove);
				children.erase(it);
				return;
			}
		}
	}

	void Node::add_component(Component *c)
	{
		c->parent = this;
		components.emplace_back(c);
		broadcast_upward(c, MessageComponentAdd);
	}

	void Node::remove_component(Component *c)
	{
		for (auto it = components.begin(); it != components.end(); it++)
		{
			if (it->get() == c)
			{
				components.erase(it);
				broadcast_upward(c, MessageComponentRemove);
				return;
			}
		}
	}

	void Node::mark_coord_setted()
	{
		matrix_dirty = true;
		transform_dirty_frame = total_frame_count;
	}

	void Node::mark_euler_setted()
	{
		axis_dirty = true;
		euler_dirty = false;
		quat_dirty = true;
		matrix_dirty = true;
		transform_dirty_frame = total_frame_count;
	}

	void Node::mark_quat_setted()
	{
		axis_dirty = true;
		euler_dirty = true;
		quat_dirty = false;
		matrix_dirty = true;
		transform_dirty_frame = total_frame_count;
	}

	void Node::mark_axis_setted()
	{
		axis_dirty = false;
		euler_dirty = true;
		quat_dirty = true;
		matrix_dirty = true;
		transform_dirty_frame = total_frame_count;
	}

	void Node::mark_scale_setted()
	{
		matrix_dirty = true;
		transform_dirty_frame = total_frame_count;
	}

	void Node::component_boardcast(Node *n, Message msg)
	{
		for (auto &c : n->components)
			n->broadcast_upward(c.get(), msg);
		for (auto &c : n->children)
			c->component_boardcast(c.get(), msg);
	}
}
