#include <flame/foundation/foundation.h>

using namespace flame; 

struct Quaternion {
    double w, x, y, z;
};

struct EulerAngles {
    double yaw, pitch, roll;
};

Quaternion ToQuaternion(double yaw, double pitch, double roll)
{
    // Abbreviations for the various angular functions
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

EulerAngles ToEulerAngles(Quaternion q) {
    EulerAngles angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    angles.roll = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = 2 * (q.w * q.y - q.z * q.x);
    if (std::abs(sinp) >= 1)
        angles.pitch = std::copysign(pi<float>() / 2, sinp); // use 90 degrees if out of range
    else
        angles.pitch = std::asin(sinp);

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    angles.yaw = std::atan2(siny_cosp, cosy_cosp);

    return angles;
}

int main(int argc, char** args) 
{
	glm::quat q(.5, .5, .5, .5);
	glm::vec3 euler = glm::eulerAngles(q);
    auto wtf1 = ToEulerAngles({ .5, .5, .5, .5 });
    auto wtf2 = ToQuaternion(wtf1.yaw, wtf1.pitch, wtf1.roll);
	quat QuatAroundX = angleAxis(euler.x, vec3(1.0, 0.0, 0.0));
	quat QuatAroundY = angleAxis(euler.y, vec3(0.0, 1.0, 0.0));
	quat QuatAroundZ = angleAxis(euler.z, vec3(0.0, 0.0, 1.0));
	quat finalOrientation = QuatAroundX * QuatAroundY * QuatAroundZ;

	return 0;
}

