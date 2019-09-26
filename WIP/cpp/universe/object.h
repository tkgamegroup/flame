namespace flame
{
	struct Object
	{
		Model* m;

		Vec3 pos;
		EulerYawPitchRoll euler;
		Vec3 scale;
		Mat4 mat;

		AABB aabb;
		Vec3 aabb_c;
		float aabb_r;

		void update_mat()
		{
			mat = Mat4(Mat3(euler) * Mat3(scale), pos);
		}
	};
}
