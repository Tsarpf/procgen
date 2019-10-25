#pragma once

namespace Sampler
{

float Sphere(const glm::vec3 & worldPosition, const glm::vec3 & origin, float radius);

float Box(const glm::vec3 & p, const glm::vec3 & size);

float Noise(const glm::vec3 & p);

float Waves(const glm::vec3 & p);

float Plane(const glm::vec3 & p);

float Density(const glm::vec3 pos);

}
