#include <glm/glm.hpp>
#include <noise/noise.h>
#include <algorithm>
#include <future>
#include <thread>
#include <assert.h>
#include <vector_types.h>

#include "Sampler.h"

#include "NoiseSampler.cuh"

namespace Sampler
{

int index3D(int x, int y, int z, int dimensionLength)
{
	return x + dimensionLength * (y + dimensionLength * z);
}
// integer as floats only exact up to 2^24 = 16,777,216
// shouldn't be a problem?
int index3D(glm::ivec3 coord, int dimensionLength)
{
	return index3D(coord.x, coord.y, coord.z, dimensionLength);
}

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

std::vector<float4>* BuildCacheCuda(const glm::ivec3 min, const unsigned size)
{
	const int samplesPerUnit = 8;
	const int samplesPerDirection = 8 * size;
	const int numSamples = samplesPerDirection * samplesPerDirection * samplesPerDirection;

	std::vector<float4>* results = new std::vector<float4>();
	results->resize(numSamples);
	CudaNoise::CacheArea(min.x, min.y, min.z, size, results->data());
	//results.assign(res, res + numSamples);
	return results;
}

float4 SampleCacheCuda(const std::vector<float4>& cache, const int size, const glm::vec3 coordinate, const glm::ivec3 min)
{
	// Take into account 8 per unit
	const glm::ivec3 innerCoordinate
	(
		(coordinate.x - min.x) * 8.0,
		(coordinate.y - min.y) * 8.0,
		(coordinate.z - min.z) * 8.0
	);
	const unsigned long idx = index3D(innerCoordinate, size * 8);
	return cache[idx];
}

float DensityCuda(const std::vector<float4>& cache, const int size, const glm::vec3 pos, const glm::vec3 min)
{
	float4 sample = SampleCacheCuda(cache, size, pos, min);
	return sample.x;
}
}
