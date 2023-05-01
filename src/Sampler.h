#pragma once
#include <vector>

namespace Sampler
{

float Sphere(const glm::vec3 & worldPosition, const glm::vec3 & origin, float radius);

float Box(const glm::vec3 & p, const glm::vec3 & size);

std::tuple<float, glm::vec3> Noise(const glm::vec3 & p);

float Waves(const glm::vec3 & p);

float Plane(const glm::vec3 & p);

std::vector<float> BuildCache(glm::ivec3 min, unsigned range);
std::vector<float> BuildSIMDCache(glm::ivec3 min, unsigned range);

float SampleCache(const std::vector<std::vector<float>>& cache, const int coordinate);

float SampleCache(const std::vector<float>& cache, const glm::ivec3 min, const int size, const glm::ivec3 coordinate);

float Density(const glm::vec3 pos);

float Sample(const glm::vec3 pos);
glm::vec3 SampleGradient(const glm::vec3 pos);

}
