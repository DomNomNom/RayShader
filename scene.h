
#include <vector>
#include "glm/glm.hpp"


enum scene {
    SCENE_BEACH,
    SCENE_SURFACE
};

void loadScene(
    scene s,
    std::vector<glm::vec4>  &vertecies,
    std::vector<int>        &triangles,
    std::vector<glm::vec4>  &ball_pos,
    std::vector<float>      &ball_radius
);