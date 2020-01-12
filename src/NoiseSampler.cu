#include <iostream>
#include <vector>
#include <chrono>

#include "cuda_noise.cuh"

#include "NoiseSampler.cuh"


__device__
float SampleNoise(float3 pos)
{
	int seed = 42;
	float scale = 0.01f;
	float result = cudaNoise::perlinNoise(pos, scale, seed);  // float3 pos, float scale, int seed

	scale = 0.06f;
	result += cudaNoise::perlinNoise(pos, scale, seed);

	scale = 0.12f;
	result += cudaNoise::perlinNoise(pos, scale, seed);
	return result;
}

__device__
float Waves(float3 p)
{
	//printf("density at (%f, %f, %f) is = %f\n", p.x, p.y, p.z, value);
	//std::cout << "position: " << p.x << std::endl;
	//printf("position %f %f %f\n", p.x, p.y, p.z);
	//return sin(p.x * 1.0) + cos(p.y * 1.0) + p.z - 2;

	//float value = sin(p.x * 1.0f) / 1.f + cos(p.y * 1.0f) / 1.f + p.z - 5.50f;
	//float value = sin(p.x * 0.5f) / 0.3f + p.y - 5.50f;
	float value = sin(p.x) + p.y - 6.0;

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
	int x = idx % size;
	int y = (idx / size) % size;
	int z = idx / (size * size);
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
	void CacheArea(int minX, int minY, int minZ, int size, float4* cpuResults)
	{
		auto t0 = std::chrono::high_resolution_clock::now();
		float3 min = make_float3(minX, minY, minZ);
		float4* results;

		// 1 float for density, 3 floats for gradient, 
		// size^3 points in grid
		//8 samples per unit of grid (for figuring out zero crossing)
		unsigned int samplesPerDirection = 8 * size;
		unsigned long samples = samplesPerDirection * samplesPerDirection * samplesPerDirection;
		unsigned long dataSize = sizeof(float) * 4 * samples;

		int threadsPerBlock = 256;
		int numBlocks = (samples + threadsPerBlock - 1) / threadsPerBlock; // round up

		cudaMallocManaged(&results, dataSize);
		auto t1 = std::chrono::high_resolution_clock::now();


		printf("Launching kernel\n");
		CacheKernel <<<numBlocks, threadsPerBlock >> > (results, min, size);
		auto t2 = std::chrono::high_resolution_clock::now();

		//printf("Syncing \n");
		//cudaDeviceSynchronize();
		auto t3 = std::chrono::high_resolution_clock::now();

		printf("Copying to vector \n");
		cudaMemcpy(cpuResults, results, dataSize, cudaMemcpyDeviceToHost);
		auto t4 = std::chrono::high_resolution_clock::now();

		printf("Freeing memory \n");
		cudaFree(results);
		auto t5 = std::chrono::high_resolution_clock::now();

		printf("Done \n");

		auto initializationTime = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
		auto computeTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		auto syncTime = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
		auto memcpyTime = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
		auto freeTime = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count();
		std::cout << "init and malloc time " << initializationTime / 1000.f << "ms" << std::endl;
		std::cout << "compute time " << computeTime / 1000.f << "ms" << std::endl;
		std::cout << "synchronize time " << syncTime / 1000.f << "ms" << std::endl;
		std::cout << "memcpy time" << memcpyTime / 1000.f << "ms" << std::endl;
		std::cout << "cudaFree time " << freeTime / 1000.f << "ms" << std::endl;
	}
}
