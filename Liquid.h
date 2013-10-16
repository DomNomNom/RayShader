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

class Liquid {
public:

    //CONSTRUCTOR
    /*!Creates a new liquid simulation*/
    Liquid();

    //DESTRUCTOR
    /*!Destroys this liquid*/
    ~Liquid();

    //PUBLIC MEMBER FUNCTIONS
    /*!Updates the liquid*/
    void update();

    /*!Renders the liquid
    @polygonWrap is true to render in polygon wrapping mode*/
    void render(bool polygonWrap);

    /*!Cleans up the liquid*/
    void cleanUp();

    /*!@return the vector of gravity*/
    static glm::vec3 getGravity();

private:

    //VARIABLES
    //the dimensions of the grid
    const glm::vec3 GRID_DIM;
    //the size of a grid cell
    const float CELL_SIZE;

    //the grid of cells
    LiquidGrid mGrid;

    //the strength of gravity
    static float sGravityStrength;
    //the direction of gravity
    static glm::vec3 sGravityDirection;

    //the wave (Testing)
    float mWave;
    bool mWaveUp;

    //MACROS
    DISALLOW_COPY_AND_ASSIGN(Liquid);

    //PRIVATE MEMBER FUNCTIONS
    /*!Renders the liquid with polygon wrapping*/
    void renderPolygonWrap();

    /*!computes the lower (non turbulent) half of the liquid's polygons
    @top the highest point of the lower half
    @return the list of faces of the polygons*/
    FaceList* computeNonTurbulent(unsigned top) const;

    /*!Computes the upper (turbulent) half of the liquid's polygons
    @top the highest point of the upper half
    @return the list of faces of the polygons*/
    FaceList* computeTurbulent(unsigned top) const;

    /*!Recursivly computes the turbulent polygons
    @topLeft the top left indices of the square
    @bottomRight the bottom right indices of the square
    @return the list of faces of the square*/
    FaceList* computeTurbulentRec(const glm::vec3& topLeft,
        const glm::vec3& bottomRight) const;

    /*!Renders a list of polygons as liquid*/
    void renderPolygons(FaceList* faces);

    /*!Renders the liquid as particles*/
    void renderParticles();

    /*!Renders the liquid grid*/
    void renderGrid();

    /*!Renders the border of the liquid*/
    void renderBorder();
};

#endif