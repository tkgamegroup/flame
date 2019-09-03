namespace flame
{
	enum NodeType
	{
		NodeTypeNode,
		NodeTypeScene
	};

	struct Component;

	 class Node : public Object
	{
	public:
		enum Axis
		{
			AxisNull = -1,
			AxisX,
			AxisY,
			AxisZ
		};
	private:
		glm::vec3 coord;
		glm::vec3 euler; // (yaw, pitch, roll)
		glm::vec4 quat;
		glm::vec3 scale;

		glm::mat3 axis;
		glm::mat4 matrix;

		bool axis_dirty;
		bool euler_dirty;
		bool quat_dirty;
		bool matrix_dirty;
	protected:
		long long transform_dirty_frame;
	private:
		NodeType type;
		Node *parent;
		std::vector<std::unique_ptr<Node>> children; 
		std::vector<std::unique_ptr<Component>> components;
	public:
		bool broadcast_upward(Object *src, Message msg);

		Node(NodeType _type = NodeTypeNode);
	};
}
