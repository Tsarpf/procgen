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

glm::vec4 compute_noise_octave(const glm::vec3& pos, float frequency, float amplitude) {
    glm::vec3 scaled_pos = pos * frequency;
    glm::vec4 noise_result = noise_grad(scaled_pos) * amplitude;
    return noise_result;
}

std::pair<float, glm::vec3> Noise(const glm::vec3& p)
{
	float epsilon = 0.500f;
	float divider = 26.f;
	float seaLevel = 32.f;

	glm::vec3 pos = p / divider + epsilon;
    //glm::vec4 result = compute_noise_octave(pos, 1.0f, 1.0f);
    glm::vec4 result = compute_noise_octave(pos, .5f, 0.5f);
    result += compute_noise_octave(pos, 2.0f, 0.10f);
    result += compute_noise_octave(pos, 0.01f, 1.0f);

    float value = result.x;
    glm::vec3 grad = glm::vec3(result.y, result.z, result.w);

    // Adjust value and gradient for sea level
	if (p.y > seaLevel)
	{

		float seaScaler = 20.0f;
		float adjustment = (p.y - seaLevel) / seaScaler;
		value += adjustment;
		grad.y += 1.f / seaScaler; // Applying chain rule: g'(p.y) = gy + 1/10
	}

	grad = glm::normalize(grad);
	return {value, grad};
}

float Sample(const glm::vec3 pos)
{
	//glm::vec4 value = noise_grad(pos).x; // (value float, gradient vec3)
	return Noise(pos).first;
	//printf("position (%f %f %f) value = %f \n", pos.x, pos.y, pos.z, value);
	//return value >= 0.0f;
}

glm::vec3 SampleGradient(const glm::vec3 pos)
{
	return Noise(pos).second;
}
}
