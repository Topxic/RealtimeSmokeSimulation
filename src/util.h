#pragma once

#include <vector>

#include "graphics/mesh.h"

namespace geometry
{

    std::vector<graphics::Mesh::Vertex> createCubeVertices(glm::vec3 gridRes)
    {
        float longestSide = glm::max(gridRes.x, glm::max(gridRes.y, gridRes.z));

        float xExtent = 0.5f * (gridRes.x / longestSide);
        float yExtent = 0.5f * (gridRes.y / longestSide);
        float zExtent = 0.5f * (gridRes.z / longestSide);

        return {
            // Front Face
            {{-xExtent, -yExtent, zExtent}, {0, 0, 1}, {0, 0}},
            {{xExtent, -yExtent, zExtent}, {0, 0, 1}, {1, 0}},
            {{xExtent, yExtent, zExtent}, {0, 0, 1}, {1, 1}},
            {{-xExtent, yExtent, zExtent}, {0, 0, 1}, {0, 1}},

            // Back Face
            {{xExtent, -yExtent, -zExtent}, {0, 0, -1}, {0, 0}},
            {{-xExtent, -yExtent, -zExtent}, {0, 0, -1}, {1, 0}},
            {{-xExtent, yExtent, -zExtent}, {0, 0, -1}, {1, 1}},
            {{xExtent, yExtent, -zExtent}, {0, 0, -1}, {0, 1}},

            // Left Face
            {{-xExtent, -yExtent, -zExtent}, {-1, 0, 0}, {0, 0}},
            {{-xExtent, -yExtent, zExtent}, {-1, 0, 0}, {1, 0}},
            {{-xExtent, yExtent, zExtent}, {-1, 0, 0}, {1, 1}},
            {{-xExtent, yExtent, -zExtent}, {-1, 0, 0}, {0, 1}},

            // Right Face
            {{xExtent, -yExtent, zExtent}, {1, 0, 0}, {0, 0}},
            {{xExtent, -yExtent, -zExtent}, {1, 0, 0}, {1, 0}},
            {{xExtent, yExtent, -zExtent}, {1, 0, 0}, {1, 1}},
            {{xExtent, yExtent, zExtent}, {1, 0, 0}, {0, 1}},

            // Top Face
            {{-xExtent, yExtent, -zExtent}, {0, 1, 0}, {0, 0}},
            {{-xExtent, yExtent, zExtent}, {0, 1, 0}, {0, 1}},
            {{xExtent, yExtent, zExtent}, {0, 1, 0}, {1, 1}},
            {{xExtent, yExtent, -zExtent}, {0, 1, 0}, {1, 0}},

            // Bottom Face
            {{-xExtent, -yExtent, zExtent}, {0, -1, 0}, {0, 0}},
            {{-xExtent, -yExtent, -zExtent}, {0, -1, 0}, {0, 1}},
            {{xExtent, -yExtent, -zExtent}, {0, -1, 0}, {1, 1}},
            {{xExtent, -yExtent, zExtent}, {0, -1, 0}, {1, 0}},
        };
    };

    std::vector<unsigned int> createCubeIndices()
    {
        return {
            // Front Face
            0, 1, 2, 2, 3, 0,
            // Back Face
            4, 5, 6, 6, 7, 4,
            // Left Face
            8, 9, 10, 10, 11, 8,
            // Right Face
            12, 13, 14, 14, 15, 12,
            // Top Face
            16, 17, 18, 18, 19, 16,
            // Bottom Face
            20, 21, 22, 22, 23, 20};
    };

} // namespace geometry

namespace perlin
{

    inline uint32_t hash(const glm::uvec3 &key, uint32_t seed)
    {
        glm::uvec3 k = key;
        k *= 0x27d4eb2fu;
        k ^= (k >> 16u);
        k *= 0x85ebca77u;

        uint32_t h = seed;

        h ^= k.x;
        h ^= h >> 16;
        h *= 0x9e3779b1u;

        h ^= k.y;
        h ^= h >> 16;
        h *= 0x9e3779b1u;

        h ^= k.z;
        h ^= h >> 16;
        h *= 0x9e3779b1u;

        h ^= h >> 16;
        h *= 0xed5ad4bbu;
        h ^= h >> 16;

        return h;
    }

    inline uint32_t hash(uint32_t key, uint32_t seed)
    {
        uint32_t k = key;

        k *= 0x27d4eb2fu;
        k ^= k >> 16;
        k *= 0x85ebca77u;

        uint32_t h = seed;

        h ^= k;
        h ^= h >> 16;
        h *= 0x9e3779b1u;

        return h;
    }

    inline glm::vec3 gradient(uint32_t h)
    {
        static const glm::vec3 gradients[12] =
            {
                {1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0}, {1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1}, {0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}};

        return gradients[h % 12u];
    }

    inline float interpolate(
        float v1, float v2, float v3, float v4,
        float v5, float v6, float v7, float v8,
        const glm::vec3 &t)
    {
        return glm::mix(
            glm::mix(glm::mix(v1, v2, t.x), glm::mix(v3, v4, t.x), t.y),
            glm::mix(glm::mix(v5, v6, t.x), glm::mix(v7, v8, t.x), t.y),
            t.z);
    }

    inline glm::vec3 fade(const glm::vec3 &t)
    {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    inline float perlinNoise(const glm::vec3 &position, uint32_t seed)
    {
        glm::vec3 floorPos = glm::floor(position);
        glm::vec3 fractPos = position - floorPos;

        glm::uvec3 cell = glm::uvec3(glm::ivec3(floorPos));

        float v1 = glm::dot(gradient(hash(cell, seed)), fractPos);
        float v2 = glm::dot(gradient(hash(cell + glm::uvec3(1, 0, 0), seed)), fractPos - glm::vec3(1, 0, 0));
        float v3 = glm::dot(gradient(hash(cell + glm::uvec3(0, 1, 0), seed)), fractPos - glm::vec3(0, 1, 0));
        float v4 = glm::dot(gradient(hash(cell + glm::uvec3(1, 1, 0), seed)), fractPos - glm::vec3(1, 1, 0));

        float v5 = glm::dot(gradient(hash(cell + glm::uvec3(0, 0, 1), seed)), fractPos - glm::vec3(0, 0, 1));
        float v6 = glm::dot(gradient(hash(cell + glm::uvec3(1, 0, 1), seed)), fractPos - glm::vec3(1, 0, 1));
        float v7 = glm::dot(gradient(hash(cell + glm::uvec3(0, 1, 1), seed)), fractPos - glm::vec3(0, 1, 1));
        float v8 = glm::dot(gradient(hash(cell + glm::uvec3(1, 1, 1), seed)), fractPos - glm::vec3(1, 1, 1));

        return interpolate(v1, v2, v3, v4, v5, v6, v7, v8, fade(fractPos));
    }

    inline std::vector<glm::vec4> perlinNoise(
        glm::ivec3 resolution,
        int octaveCount,
        float persistence,
        float lacunarity,
        float amplitude,
        float scale,
        uint32_t seed)
    {
        const int gridSize = resolution.x * resolution.y * resolution.z;
        std::vector<glm::vec4> values(gridSize);
        for (int x = 0; x < resolution.x; ++x)
        {
            for (int y = 0; y < resolution.y; ++y)
            {
                for (int z = 0; z < resolution.z; ++z)
                {

                    float m_amplitude = amplitude;
                    glm::vec3 position = {x, y, z};
                    // Normalize
                    position /= scale;
                    float value = 0.0f;

                    for (int i = 0; i < octaveCount; i++)
                    {
                        uint32_t s = hash((uint32_t)i, seed);

                        value += perlinNoise(position, s) * m_amplitude;

                        m_amplitude *= persistence;
                        position *= lacunarity;
                    }

                    values[z * resolution.x * resolution.y + y * resolution.x + x] = glm::vec4(value);
                }
            }
        }
        return values;
    }

} // namespace perlin

namespace voronoi
{

    static float randomFloat()
    {
        return (float)(rand()) / (float)(RAND_MAX);
    }

    static std::vector<glm::vec4> createVoronoiNoise(const glm::ivec3 resolution, const glm::ivec3 gridRes)
    {
        // Create voronoi texture
        const int gridSize = gridRes.x * gridRes.y * gridRes.z;
        const int texSize = resolution.x * resolution.y * resolution.z;
        auto samples = std::vector<glm::vec3>(gridSize);
        auto voronoiNoise = std::vector<glm::vec4>(texSize);
        srand(time(0));
        for (int i = 0; i < gridSize; ++i)
        {
            samples[i].x = randomFloat();
            samples[i].y = randomFloat();
            samples[i].z = randomFloat();
        }

        const glm::vec3 cellSize = glm::vec3(resolution) / glm::vec3(gridRes);
        for (int x = 0; x < resolution.x; ++x)
        {
            for (int y = 0; y < resolution.y; ++y)
            {
                for (int z = 0; z < resolution.z; ++z)
                {

                    // Maximum possible distance
                    float minDistance = glm::length(cellSize);
                    glm::vec3 pixel = {x, y, z};
                    glm::ivec3 gridCell = glm::ivec3(pixel / cellSize);

                    for (int i = -1; i <= 1; ++i)
                    {
                        for (int j = -1; j <= 1; ++j)
                        {
                            for (int k = -1; k <= 1; ++k)
                            {

                                glm::vec3 cellIdx = gridCell + glm::ivec3{i, j, k};

                                int idx = cellIdx.z * gridRes.x * gridRes.y + cellIdx.y * gridRes.x + cellIdx.x;
                                if (idx >= 0 && idx < gridSize)
                                {
                                    glm::vec3 sample = (cellIdx + samples[idx]) * cellSize;
                                    glm::vec3 diff = pixel - sample;
                                    float dist = glm::length(diff);
                                    minDistance = glm::min(minDistance, dist);
                                }
                            }
                        }
                    }
                    minDistance /= glm::length(glm::vec3(cellSize));

                    voronoiNoise[z * resolution.x * resolution.y + y * resolution.x + x] = glm::vec4(minDistance, minDistance, minDistance, 1.0);
                }
            }
        }
        return voronoiNoise;
    }

    inline std::vector<glm::vec4> composedVoronoiNoise(const glm::ivec3 voronoiResolution, const float c1, const float c2, const float c3)
    {
        auto voronoi4Noise = createVoronoiNoise(voronoiResolution, glm::ivec3(4));
        auto voronoi8Noise = createVoronoiNoise(voronoiResolution, glm::ivec3(8));
        auto voronoi16Noise = createVoronoiNoise(voronoiResolution, glm::ivec3(16));
        auto voronoiNoise = std::vector<glm::vec4>(voronoiResolution.x * voronoiResolution.y * voronoiResolution.z);
        for (int i = 0; i < voronoiNoise.size(); ++i)
        {
            voronoiNoise[i] = c1 * voronoi4Noise[i] + c2 * voronoi8Noise[i] + c3 * voronoi16Noise[i];
        }

        return voronoiNoise;
    }

} // namespace voronoi