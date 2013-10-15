
#include <vector>
#include "glm/glm.hpp"


void loadScene(
    const char* filepath,
    std::vector<glm::vec4>  &vertecies,
    std::vector<int>        &triangles,
    std::vector<glm::vec4>  &ball_pos,
    std::vector<float>      &ball_radius
);