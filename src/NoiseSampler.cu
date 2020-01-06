#include <iostream>

#include "cuda_noise.cuh"

#include "NoiseSampler.cuh"

__device__
float SampleNoise(float3 pos)
{
  int seed = 42;
  float scale = 0.05f;
  return cudaNoise::perlinNoise(pos, scale, seed);  // float3 pos, float scale, int seed
}

__device__
float Waves(float3 p)
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

__device__
void NoiseDensity(float* result, float3 position) {
  //float3 pos = make_float3(0.1, 0.1, 0.1);
  *result = SampleNoise(position);
}

__device__
void NoiseGradient(float3* result, float3 position) {
  //float3 pos = make_float3(0.1, 0.1, 0.1);
  //*result = sample_noise(pos);

	const float epsilon = 0.0001f;
	const float dx = SampleNoise(make_float3(position.x + epsilon, position.y, position.z)) - SampleNoise(make_float3(position.x - epsilon, position.y, position.z));
	const float dy = SampleNoise(make_float3(position.x, position.y + epsilon, position.z)) - SampleNoise(make_float3(position.x, position.y - epsilon, position.z));
	const float dz = SampleNoise(make_float3(position.x, position.y, position.z + epsilon)) - SampleNoise(make_float3(position.x, position.y, position.z - epsilon));

	//const float dy = SampleNoise(position + make_float3(0.f, epsilon, 0.f)) - SampleNoise(position - make_float3(0.f, epsilon, 0.f));
	//const float dz = SampleNoise(position + make_float3(0.f, 0.f, epsilon)) - SampleNoise(position - make_float3(0.f, 0.f, epsilon));

  *result = make_float3(dx, dy, dz);
  //return glm::normalize(glm::vec3(dx, dy, dz));
}

__device__
void Sample(float4* sample, float3 position)
{
	float density;
	NoiseDensity(&density, position);
	//density = Waves(position);

	float3 gradient;
	NoiseGradient(&gradient, position);

	*sample = make_float4(density, gradient.x, gradient.y, gradient.z);
}

__device__
float3 indexToPos(long idx, int size)
{
	//int idx = i + segmentStart;
	int z = idx % size;
	int y = (idx / size) % size;
	int x = idx / (size * size);
	return make_float3(x, y, z);
}
//samples[i] = Noise(min + glm::ivec3(x, y, z), noiseModule);

__global__
void CacheKernel(float4* results, float3 min, int size)
{
	unsigned long index = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int stride = blockDim.x * gridDim.x;

	int samplesPerDirection = 8 * size;
	unsigned long n = samplesPerDirection * samplesPerDirection * samplesPerDirection;
	for (int i = index; i < n; i += stride)
	{
		float3 offset = indexToPos(i, samplesPerDirection); // like this right?
		//float3 offset = indexToPos(i, size);

		// has to be float because sub-grid-point sampling for crossing points
		float3 position = make_float3(min.x + offset.x / 8.0, min.y + offset.y / 8.0, min.z + offset.z / 8.0);

		float4 sample;
		Sample(&sample, position);

		results[i] = sample;
	}
}

namespace CudaNoise
{
	float4* CacheArea(int minX, int minY, int minZ, int size)
	{
		float3 min = make_float3(minX, minY, minZ);
		float4* results;

		// 1 float for density, 3 floats for gradient, 
		// size^3 points in grid
		//8 samples per unit of grid (for figuring out zero crossing)
		unsigned int samplesPerDirection = 8 * size;
		unsigned long samples = samplesPerDirection * samplesPerDirection * samplesPerDirection;
		unsigned long dataSize = sizeof(float) * 4 * samples;
		cudaMallocManaged(&results, dataSize);

		int threadsPerBlock = 256;
		int numBlocks = (samples + threadsPerBlock - 1) / threadsPerBlock; // round up

		printf("Launching kernel\n");
		CacheKernel <<<numBlocks, threadsPerBlock >> > (results, min, size);

		printf("Syncing \n");
		cudaDeviceSynchronize();

		printf("Done\n");
		return results;
	}
}
