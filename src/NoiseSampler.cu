#include <iostream>

#include "cuda_noise.cuh"

#include "NoiseSampler.cuh"

__device__
float SampleNoise(float3 pos)
{
  int seed = 42;
  float scale = 0.5f;
  return cudaNoise::perlinNoise(pos, scale, seed);  // float3 pos, float scale, int seed
}

__global__ 
void test_kernel(float* result) {
  float3 pos = make_float3(0.1, 0.1, 0.1);
  *result = SampleNoise(pos);
}

__global__ 
void NoiseDensity(float* result, float3 position) {
  //float3 pos = make_float3(0.1, 0.1, 0.1);
  *result = SampleNoise(position);
}

__global__ 
void NoiseGradient(float3* result, float3 position) {
  //float3 pos = make_float3(0.1, 0.1, 0.1);
  //*result = sample_noise(pos);

	const float epsilon = 0.0001f;
	const float dx = SampleNoise(position + make_float3(epsilon, 0.f, 0.f)) - SampleNoise(position - make_float3(epsilon, 0.f, 0.f));
	const float dy = SampleNoise(position + make_float3(0.f, epsilon, 0.f)) - SampleNoise(position - make_float3(0.f, epsilon, 0.f));
	const float dz = SampleNoise(position + make_float3(0.f, 0.f, epsilon)) - SampleNoise(position - make_float3(0.f, 0.f, epsilon));

  *result = make_float3(dx, dy, dz);
  //return glm::normalize(glm::vec3(dx, dy, dz));
}

namespace CudaNoise
{
  void CacheArea()
  {
  }

  void CacheDensity(float* results, float3 min, int size)
  {

  }

  void CacheGradient(float* results, float3 min, int size)
  {
  }

  void Sample(void)
  {
    //int N = 3;
    //float3* result;
    float* result;
    //*result = 0.0f;

    printf("Malloc\n");
    cudaMallocManaged(&result, sizeof(float));

    printf("Launching kernel\n");

    test_kernel <<<1, 1>>> (result);

    printf("Syncing \n");
    cudaDeviceSynchronize();

    printf("Done\n");

    printf("Result %f\n", *result);
  }

  //std::vector<float> AsyncCache(glm::ivec3 min, int segmentStart, int sampleCount, int size)
  //{
  //  noise::module::Perlin noiseModule;
  //  std::vector<float> samples(sampleCount);

  //  for (int i = 0; i < sampleCount; i++)
  //  {
  //    int idx = i + segmentStart;
  //    int z = idx % size;
  //    int y = (idx / size) % size;
  //    int x = idx / (size * size);
  //    samples[i] = Noise(min + glm::ivec3(x, y, z), noiseModule);
  //  }
  //  return samples;
  //}

}
//   std::vector<std::vector<float>> BuildCache(const glm::ivec3 min, const unsigned size)
//   {
//     unsigned idxCount = size * size * size;
//     unsigned concurrentThreadsSupported = std::thread::hardware_concurrency();
// 
//     // perf-wise no sense in computing less than thousands of samples per thread, but for debugging it's nice
//     unsigned threads = std::min(concurrentThreadsSupported, idxCount / 8); 
// 
//     unsigned samplesPerSegment = idxCount / threads;
// 
//     unsigned extras = size % threads;
//     //assert(size % threads == 0);
// 
//     std::vector<std::future<std::vector<float>>> futureSamples;
//     std::vector<std::vector<float>> results;
//     for (unsigned i = 0; i < threads; i++)
//     {
//       int segmentStart = i * (idxCount / threads);
//       futureSamples.push_back(std::async(
//             std::launch::async,
//             AsyncCache,
//             min,
//             segmentStart,
//             i < threads-1 ? samplesPerSegment : samplesPerSegment + extras,
//             size));
//     }
// 
//     for (auto& future : futureSamples)
//     {
//       results.push_back(future.get());
//     }
// 
//     return results;
//   }
