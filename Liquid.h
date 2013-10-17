#ifndef RAYSHADER_LIQUID_LIQUID_H_
#   define RAYSHADER_LIQUID_LIQUID_H_

#include <GL/glut.h>
#include "glm/glm.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "util.h"

//TYPEDEF
typedef std::vector<float> t_HeightRow;
typedef std::vector<t_HeightRow> t_HeightMap2;
typedef std::vector<glm::vec3> t_HeightMap;
typedef std::vector<glm::vec3> t_NormalMap;

namespace liquid {

    //ENUMERATOR
    //the render modes for the liquid
    enum e_RenderMode {

        //don't render
        NONE = 0,
        //render as particle grid
        GRID,
        //pass as height map to be ray traced
        RAYTRACE
    };
} //liquid

class Liquid {
public:

    //CONSTRUCTOR
    /*!Creates a new liquid simulation
    @heightMap a pointer to the height map of the turbulent liquid
    @normalMap a pointer to the normals of the height map
    @turbulentMin a pointer to the bottom of the turbulent liquid
    @turbulentMax a pointer to the top of the turbulent water
    @waterBottom a pointer to the bottom of the water*/
    Liquid(t_HeightMap* heightMap, t_NormalMap* normalMap,
        float* turbulentMin, float* turbulentMax, float* waterBottom);

    //DESTRUCTOR
    /*!Destroys this liquid*/
    ~Liquid();

    //PUBLIC MEMBER FUNCTIONS
    /*!Updates the liquid*/
    void update();

    /*!Renders the liquid
    @renderMode the rendering mode of the water*/
    void render(liquid::e_RenderMode renderMode);

private:

    //VARIABLES
    //the dimensions of the grid (in cell size)
    const glm::vec2 GRID_DIM;
    //the size of a grid cell
    float m_CellSize;

    //the height map of the water
    t_HeightMap2 m_HeightMap2;

    //ray tracing values
    t_HeightMap* m_HeightMap;
    t_NormalMap* m_NormalMap;
    float* m_TurbulentMin;
    float* m_TurbulentMax;

    //MACROS
    DISALLOW_COPY_AND_ASSIGN(Liquid);

    //PRIVATE MEMBER FUNCTIONS
    /*!Renders the liquid as particles*/
    void renderParticles();

    /*!Renders the border of the liquid*/
    void renderBorder();

    /*!Calculates and sets the ray tracing height map*/
    void computeHeightMap();
};

#endif