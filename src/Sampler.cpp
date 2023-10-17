#include <glm/glm.hpp>
#include <algorithm>
#include <future>
#include <thread>
#include <assert.h>

#include "Sampler.h"
//#include "morton.h"
#include "Noise.h"

namespace Sampler
{

int index3D(int x, int y, int z, int dimensionLength)
{
	//printf("pos x %i y %i z %i result %i\n", x, y, z, x + dimensionLength * (y + dimensionLength * z));
	return x + dimensionLength * (y + dimensionLength * z);
}
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


std::tuple<float, glm::vec3> Noise(const glm::vec3& p)
{
	float epsilon = 0.500f;
	float divider = 26.f;
	//float value = (float)noiseModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);

	//divider = 100.f;
	//value += (float)noiseModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);

	//divider = 400.f;
	//value += (float)noiseModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);

	glm::vec3 pos  = p / divider + epsilon;
	//float value = (float)noiseModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);
	glm::vec4 result = noise_grad(pos); // (value float, gradient vec3)
	float value = result.x;
	glm::vec3 grad(result.y, result.z, result.w);
	if (p.y > 40.f)
	{
		value += (p.y - 20.f) / 50.f;
	}

	//if (p.y > 20.f)
	//{
	//	divider = 200.f;
	//	value -= (float)noiseModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);
	//	value -= 1.0f;
	//}
	//if (p.y > 40.f)
	//{
	//	value += 1.5f;
	//}
	//return std::make_tuple<float, glm::vec3>(value, grad);
	return {value, grad};
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
const glm::ivec3 CHILD_MIN_OFFSETS[] =
{
        glm::ivec3(0,0,0),
        glm::ivec3(0,0,1),
        glm::ivec3(0,1,0),
        glm::ivec3(0,1,1),
        glm::ivec3(1,0,0),
        glm::ivec3(1,0,1),
        glm::ivec3(1,1,0),
        glm::ivec3(1,1,1),
};
// Size parameter instead of max coordinate to enforce same size in all coordinate axes for easier indexing
void AsyncCache(glm::ivec3 min, int segmentStart, int sampleCount, int size, std::vector<float>& samples)
{
	for (int i = 0; i < sampleCount; i++)
	{
		int idx = i + segmentStart;
		int x = idx % size;
		int y = (idx / size) % size;
		int z = idx / (size * size);
		samples[idx] = Sample(min + glm::ivec3(x, y, z));
	}
}

const unsigned int g_multiplier = 1; // todo move somewhere nicer
std::vector<float> BuildCache(const glm::ivec3 min, const unsigned size)
{
	// assert(size)
	// int size_with_padding = size + 

	unsigned int idxCount = size * size * size * g_multiplier * g_multiplier * g_multiplier;
	//unsigned int idxCount = size * size * size;
	unsigned int concurrentThreadsSupported = std::thread::hardware_concurrency();

	// perf-wise no sense in computing less than thousands of samples per thread, but for debugging it's nice
	unsigned int threads = std::min(concurrentThreadsSupported, idxCount / 8) - 1; 

	unsigned int samplesPerSegment = idxCount / threads;

	unsigned int extras = idxCount % threads;
	//assert(size % threads == 0);

	std::vector<std::future<void>> futureSamples;
	std::vector<float> samples(idxCount);
	for (unsigned i = 0; i < threads; i++)
	{
		int segmentStart = i * (idxCount / threads);
		futureSamples.push_back(std::async(
			std::launch::async,
			AsyncCache,
			min,
			segmentStart,
			i < threads-1 ? samplesPerSegment : samplesPerSegment + extras,
			size,
			std::ref(samples)));
	}

	for (auto& future : futureSamples)
	{
		// The data will stay in order due to waiting for each to finish in order they were started (and .get() blocks)
		future.get();
	}

	return samples;
}

float SampleCache(const std::vector<float>& cache, const int size, const glm::ivec3 coordinate)
{
	return cache[index3D(coordinate, size)];
}

float Density(const glm::vec3 pos)
{

	//printf("density %f %f %f \n", pos.x, pos.y, pos.z);
	//glm::vec3 repeat(15, 15, 15);
	//glm::vec3 repeatPos(
	//	repeatAxis(pos.x, repeat.x),
	//	repeatAxis(pos.y, repeat.y),
	//	repeatAxis(pos.z, repeat.z)
	//);
	//return glm::length(pos - origin) - radius; // repeating
	return std::get<0>(Noise(pos));
	//return fastNoise(pos);
	//return Sphere(repeatPos, glm::vec3(0, 0, 0), 6.0);

	//return Box(pos - glm::vec3(16,16,16), glm::vec3(128, 8, 8));
	//return Box(repeatPos - glm::vec3(0,0,0), glm::vec3(5, 5, 5));

	//return Plane(pos);

	//return Waves(pos);
}

float Sample(const glm::vec3 pos)
{

	//glm::vec4 value = noise_grad(pos).x; // (value float, gradient vec3)
	float value = Density(pos);
	//printf("position (%f %f %f) value = %f \n", pos.x, pos.y, pos.z, value);
	//return value >= 0.0f;
	return value;
}

glm::vec3 SampleGradient(const glm::vec3 pos)
{
	return std::get<1>(Noise(pos));
}


}
