#include <glm/glm.hpp>
#include <noise/noise.h>
#include <algorithm>

#include "Sampler.h"

namespace Sampler
{

float mod(float x, float y)
{
	return x - y * floor(x / y);
}
float repeatAxis(float p, float c)
{
	return mod(p, c) - 0.5f * c;
}
float Sphere(const glm::vec3& worldPosition, const glm::vec3& origin, float radius)
{
	return glm::length(worldPosition - origin) - radius; // non repeating
}

float Box(const glm::vec3& p, const glm::vec3& size)
{
	glm::vec3 d = abs(p) - size;
	glm::vec3 maxed(0);
	for (int i = 0; i < 3; i++)
	{
		maxed[i] = std::max(d[i], 0.f);
	}
	return glm::length(maxed) + std::min(std::max(d.x, std::max(d.y, d.z)), 0.f);
}

float Noise(const glm::vec3& p)
{
	double epsilon = 0.500f;
	static noise::module::Perlin myModule;
	float divider = 50.f;
	float value = (float)myModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);;
	if (p.y > 20.f)
	{
		divider = 200.f;
		value -= (float)myModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);
		value -= 1.0f;
	}
	//if (p.y > 40.f)
	//{
	//	value += 1.5f;
	//}
	return value;
}

float Waves(const glm::vec3& p)
{
	//printf("density at (%f, %f, %f) is = %f\n", p.x, p.y, p.z, value);
	//std::cout << "position: " << p.x << std::endl;
	//printf("position %f %f %f\n", p.x, p.y, p.z);
	//return sin(p.x * 1.0) + cos(p.y * 1.0) + p.z - 2;

	//float value = sin(p.x * 1.0f) / 1.f + cos(p.y * 1.0f) / 1.f + p.z - 5.50f;
	float value = sin(p.x * 0.5f) / 0.3f + p.y - 5.50f;

	//return sin(p.x) + cos(p.z) + p.y;
	return value;
}

float Plane(const glm::vec3& p)
{
	return p.x - 0.00001f *p.y;
}

float Density(const glm::vec3 pos)
{
	//glm::vec3 repeat(15, 15, 15);
	//glm::vec3 repeatPos(
	//	repeatAxis(pos.x, repeat.x),
	//	repeatAxis(pos.y, repeat.y),
	//	repeatAxis(pos.z, repeat.z)
	//);
	//return glm::length(pos - origin) - radius; // repeating
	return Noise(pos);
	//return Sphere(repeatPos, glm::vec3(0, 0, 0), 6.0);

	//return Box(pos - glm::vec3(16,16,16), glm::vec3(128, 8, 8));
	//return Box(repeatPos - glm::vec3(0,0,0), glm::vec3(5, 5, 5));

	//return Plane(pos);
	//printf("density %f \n", Waves(pos));
	//return Waves(pos);
}

}
