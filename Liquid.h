#ifndef RAYSHADER_LIQUID_LIQUID_H_
#   define RAYSHADER_LIQUID_LIQUID_H_

#include <stdlib.h>

class LiquidCell;

#include "LiquidCell.h"
#include "util.h"

//TYPEDEF
typedef std::vector<LiquidCell*> LiquidRow;
typedef std::vector<LiquidRow> LiquidSlice;
typedef std::vector<LiquidSlice> LiquidGrid;
typedef std::vector<glm::vec3> FaceList;
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

    /*!Cleans up the liquid*/
    void cleanUp();

    /*!@return the vector of gravity*/
    static glm::vec3 getGravity();

private:

    //VARIABLES
    //the dimensions of the grid
    const glm::vec3 GRID_DIM;
    //the size of a grid cell
    float m_CellSize;

    //the grid of cells
    LiquidGrid mGrid;

    //the strength of gravity
    static float sGravityStrength;
    //the direction of gravity
    static glm::vec3 sGravityDirection;

    //ray tracing values
    t_HeightMap* m_HeightMap;
    t_NormalMap* m_NormalMap;
    float* m_TurbulentMin;
    float* m_TurbulentMax;

    //the wave (Testing)
    float mWave;
    bool mWaveUp;

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
