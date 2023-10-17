// The MIT License
// Copyright � 2017 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org/

// Edited from https://iquilezles.org/articles/gradientnoise/

#define GLM_FORCE_SWIZZLE 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
//#include <glm/gtc/swizzle.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <iostream>
#include <array>

using namespace glm;

constexpr std::uint32_t compile_time_hash(std::uint32_t index, std::uint32_t seed) {
    constexpr std::uint32_t prime1 = 16777619;
    constexpr std::uint32_t prime2 = 2166136261;
    constexpr std::uint32_t prime3 = 3074457;
    return (prime1 * (prime2 * index + seed) + prime3) & 0xFF;
}

template <typename T>
constexpr void constexpr_swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

template <std::size_t N>
constexpr auto generate_permutation() {
    std::array<int, N> arr{};
    std::uint32_t seed = 42;
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = i;
    }

    for (std::size_t i = 0; i < N; ++i) {
        std::uint32_t hashed_index = compile_time_hash(i, seed);
        constexpr_swap(arr[i], arr[hashed_index]);
    }
    return arr;
}

template <std::size_t N>
constexpr auto permuted_array = generate_permutation<N>();

constexpr auto duplicated_permuted_array = [] {
    constexpr auto arr = permuted_array<256>;
    std::array<int, 512> dup_arr{};
    for (std::size_t i = 0; i < 512; ++i) {
        dup_arr[i] = arr[i & 0xFF];
    }
    return dup_arr;
}();

void printTestRandom() {
    for (auto i : duplicated_permuted_array) {
        std::cout << i << ' ';
    }
}

vec3 hash(const vec3& p) {
    // 16 pre-defined direction vectors
    static const std::array<vec3, 16> gradients = {
        vec3(1, 1, 0), vec3(-1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0),
        vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
        vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, 1, -1), vec3(0, -1, -1),
        vec3(1, 1, 0), vec3(-1, 1, 0), vec3(0, -1, 1), vec3(0, -1, -1)
    };

    int x = int(p.x) & 0xFF;
    int y = int(p.y) & 0xFF;
    int z = int(p.z) & 0xFF;

    int x_idx = duplicated_permuted_array[x];
    int y_idx = duplicated_permuted_array[x_idx] + y;
    int z_idx = duplicated_permuted_array[x_idx + y] + z;

    int gradient_idx = duplicated_permuted_array[z_idx] & 0x0F;

    return gradients[gradient_idx];
}


vec4 noise_grad(const vec3& x) {
    // grid
    vec3 p = floor(x);
    vec3 w = fract(x);

    // quintic interpolant
    vec3 u = w * w * w * (w * (w * 6.0f - 15.0f) + 10.0f);
    vec3 du = 30.0f * w * w * (w * (w - 2.0f) + 1.0f);

    // gradients
    vec3 ga = hash(p + vec3(0.0, 0.0, 0.0));
    vec3 gb = hash(p + vec3(1.0, 0.0, 0.0));
    vec3 gc = hash(p + vec3(0.0, 1.0, 0.0));
    vec3 gd = hash(p + vec3(1.0, 1.0, 0.0));
    vec3 ge = hash(p + vec3(0.0, 0.0, 1.0));
    vec3 gf = hash(p + vec3(1.0, 0.0, 1.0));
    vec3 gg = hash(p + vec3(0.0, 1.0, 1.0));
    vec3 gh = hash(p + vec3(1.0, 1.0, 1.0));

    // projections
    float va = dot(ga, w - vec3(0.0, 0.0, 0.0));
    float vb = dot(gb, w - vec3(1.0, 0.0, 0.0));
    float vc = dot(gc, w - vec3(0.0, 1.0, 0.0));
    float vd = dot(gd, w - vec3(1.0, 1.0, 0.0));
    float ve = dot(ge, w - vec3(0.0, 0.0, 1.0));
    float vf = dot(gf, w - vec3(1.0, 0.0, 1.0));
    float vg = dot(gg, w - vec3(0.0, 1.0, 1.0));
    float vh = dot(gh, w - vec3(1.0, 1.0, 1.0));

    // interpolation
    float v = va +
        u.x * (vb - va) +
        u.y * (vc - va) +
        u.z * (ve - va) +
        u.x * u.y * (va - vb - vc + vd) +
        u.y * u.z * (va - vc - ve + vg) +
        u.z * u.x * (va - vb - ve + vf) +
        u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);

    vec3 d = ga +
        u.x * (gb - ga) +
        u.y * (gc - ga) +
        u.z * (ge - ga) +
        u.x * u.y * (ga - gb - gc + gd) +
        u.y * u.z * (ga - gc - ge + gg) +
        u.z * u.x * (ga - gb - ge + gf) +
        u.x * u.y * u.z * (-ga + gb + gc - gd + ge - gf - gg + gh) +

        du * (vec3(vb - va, vc - va, ve - va) +
            u.yzx() * vec3(va - vb - vc + vd, va - vc - ve + vg, va - vb - ve + vf) +
            u.zxy() * vec3(va - vb - ve + vf, va - vb - vc + vd, va - vc - ve + vg) +
            u.yzx() * u.zxy() * (-va + vb + vc - vd + ve - vf - vg + vh));

    return vec4(v, d);
}