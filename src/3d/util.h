#include <vector>

#include "graphics/mesh.h"

std::vector<graphics::Mesh::Vertex> createCubeVertices(glm::vec3 gridRes) {
    float longestSide = glm::max(gridRes.x, glm::max(gridRes.y, gridRes.z));

    float xExtent = 0.5f * (gridRes.x / longestSide);
    float yExtent = 0.5f * (gridRes.y / longestSide);
    float zExtent = 0.5f * (gridRes.z / longestSide);

    return {
        // Front Face
        {{-xExtent, -yExtent,  zExtent}, {0, 0, 1}, {0, 0}},
        {{ xExtent, -yExtent,  zExtent}, {0, 0, 1}, {1, 0}},
        {{ xExtent,  yExtent,  zExtent}, {0, 0, 1}, {1, 1}},
        {{-xExtent,  yExtent,  zExtent}, {0, 0, 1}, {0, 1}},

        // Back Face
        {{ xExtent, -yExtent, -zExtent}, {0, 0, -1}, {0, 0}},
        {{-xExtent, -yExtent, -zExtent}, {0, 0, -1}, {1, 0}},
        {{-xExtent,  yExtent, -zExtent}, {0, 0, -1}, {1, 1}},
        {{ xExtent,  yExtent, -zExtent}, {0, 0, -1}, {0, 1}},

        // Left Face
        {{-xExtent, -yExtent, -zExtent}, {-1, 0, 0}, {0, 0}},
        {{-xExtent, -yExtent,  zExtent}, {-1, 0, 0}, {1, 0}},
        {{-xExtent,  yExtent,  zExtent}, {-1, 0, 0}, {1, 1}},
        {{-xExtent,  yExtent, -zExtent}, {-1, 0, 0}, {0, 1}},

        // Right Face
        {{ xExtent, -yExtent,  zExtent}, {1, 0, 0}, {0, 0}},
        {{ xExtent, -yExtent, -zExtent}, {1, 0, 0}, {1, 0}},
        {{ xExtent,  yExtent, -zExtent}, {1, 0, 0}, {1, 1}},
        {{ xExtent,  yExtent,  zExtent}, {1, 0, 0}, {0, 1}},

        // Top Face
        {{-xExtent,  yExtent, -zExtent}, {0, 1, 0}, {0, 0}},
        {{-xExtent,  yExtent,  zExtent}, {0, 1, 0}, {0, 1}},
        {{ xExtent,  yExtent,  zExtent}, {0, 1, 0}, {1, 1}},
        {{ xExtent,  yExtent, -zExtent}, {0, 1, 0}, {1, 0}},

        // Bottom Face
        {{-xExtent, -yExtent,  zExtent}, {0, -1, 0}, {0, 0}},
        {{-xExtent, -yExtent, -zExtent}, {0, -1, 0}, {0, 1}},
        {{ xExtent, -yExtent, -zExtent}, {0, -1, 0}, {1, 1}},
        {{ xExtent, -yExtent,  zExtent}, {0, -1, 0}, {1, 0}},
    };
};

std::vector<unsigned int> createCubeIndices() {
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
        20, 21, 22, 22, 23, 20
    };
};