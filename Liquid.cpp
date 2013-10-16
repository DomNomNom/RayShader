#include "Liquid.h"

//VARIABLES
//the strength of gravity
float Liquid::sGravityStrength = 1.0f;
//the direction of gravity
glm::vec3 Liquid::sGravityDirection(0.0f, -1.0f, 0.0f);

//CONSTRUCTOR
Liquid::Liquid(t_HeightMap* heightMap, t_NormalMap* normalMap,
        float* turbulentMin, float* turbulentMax, float* waterBottom) :
    GRID_DIM(40, 40, 40),
    m_HeightMap(heightMap),
    m_NormalMap(normalMap),
    m_TurbulentMin(turbulentMin),
    m_TurbulentMax(turbulentMax),
    mWave(0.0f),
    mWaveUp(true) {

    //seed the random number generator
    srand(time(0));

    //calculate the cell size
    m_CellSize = 2.0f / GRID_DIM.x;

    //create the grid
    for (unsigned z = 0; z < GRID_DIM.z; ++z) {

        //create a new slice in the grid
        mGrid.push_back(LiquidSlice());

        for (unsigned y = 0; y < GRID_DIM.y; ++y) {

            //create a new row in the slice
            mGrid[z].push_back(LiquidRow());

            for (unsigned x = 0; x < GRID_DIM.x; ++x) {

                LiquidCell* cell = new LiquidCell(glm::vec3(x, y, z), m_CellSize);

                //add cells to the grid
                if (y < GRID_DIM.y / 2.0f) {

                    cell->init(false);
                }   
                else {

                    cell->init(true);
                }

                mGrid[z][y].push_back(cell);
            }
        }
    }

    //set the bottom of the water
    *waterBottom = -(GRID_DIM.y / 2.0f) * m_CellSize;
}

//DESTRUCTOR
Liquid::~Liquid() {
}

//PUBLIC MEMBER FUNCTIONS
void Liquid::update() {

    //TESTING SIMULATE A WAVE
    if (mWaveUp) {

        mWave += 0.01f;

        if (mWave >= 1.0f) {

            mWaveUp = false;
        }
    }
    else {

        mWave -= 0.01f;

        if (mWave <= -1.0f) {

            mWaveUp = true;
        }
    }

    int waveIndex = (((mWave + 1.0f) / 2.0f) * GRID_DIM.x);
    float middle = GRID_DIM.y / 2.0f;

    //set the wave values
    for (unsigned z = 0; z < GRID_DIM.z; ++z) {
        for (unsigned y = GRID_DIM.y / 2.0f; y < GRID_DIM.y; ++y) {
            for (unsigned x = 0; x < GRID_DIM.x; ++x) {

                //check if in the wave
                if (static_cast<int>(x) == waveIndex &&
                    static_cast<int>(y) < middle + 2) {

                    mGrid[z][y][x]->setVelocity(getGravity());
                }
                else if ((static_cast<int>(x) == waveIndex - 1 ||
                          static_cast<int>(x) == waveIndex + 1) &&
                          static_cast<int>(y) == middle) {

                    mGrid[z][y][x]->setVelocity(getGravity());
                }
                else if ((waveIndex - 1 == -1 || waveIndex + 1 == GRID_DIM.x) &&
                          static_cast<int>(x) == waveIndex &&
                          static_cast<int>(y) == middle + 2) {

                    mGrid[z][y][x]->setVelocity(getGravity());
                }
                else {

                    mGrid[z][y][x]->setVelocity(glm::vec3());
                }
            }
        }
    }
}

void Liquid::render(liquid::e_RenderMode renderMode) {

    //render based on the rendering mode
    switch (renderMode) {

        case liquid::NONE: {

            //do nothing
            break;
        }
        case liquid::GRID: {

            renderParticles();

            //TODO: render border?

            break;
        }
        case liquid::RAYTRACE: {

            computeHeightMap();
            break;
        }
    }
}

void Liquid::cleanUp() {

    for (unsigned z = 0; z < mGrid.size(); ++z) {
        for (unsigned y = 0; y < mGrid[z].size(); ++y) {

            while(!mGrid[z][y].empty()) {

                delete mGrid[z][y].back();
                mGrid[z][y].pop_back();
            }
        }
    }

    mGrid.clear();
}

glm::vec3 Liquid::getGravity() {

    return glm::vec3(0.0f, -1.0f, 0.0f);
}

//PRIVATE MEMBER FUNCTIONS
void Liquid::renderParticles() {

    float x1 = -((GRID_DIM.x / 2.0f) * m_CellSize);
    float y1 = -((GRID_DIM.y / 2.0f) * m_CellSize);
    float z1 = -((GRID_DIM.z / 2.0f) * m_CellSize);

    //render the cells
    for (unsigned z = 0; z < GRID_DIM.z; ++z) {
        for (unsigned y = 0; y < GRID_DIM.y; ++y) {
            for (unsigned x = 0; x < GRID_DIM.x; ++x) {

                mGrid[z][y][x]->render(
                    glm::vec3(x1 + (x * m_CellSize),
                              y1 + (y * m_CellSize),
                              z1 + (z * m_CellSize)));
            }
        }
    }
}

void Liquid::renderBorder() {

    float x1 = -((GRID_DIM.x / 2.0f) * m_CellSize) - 0.001f;
    float x2 = -x1;
    float y1 = -((GRID_DIM.y / 2.0f) * m_CellSize) - 0.001f;
    float y2 = -y1;
    float z1 = -((GRID_DIM.z / 2.0f) * m_CellSize) - 0.001f;
    float z2 = -z1;

    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

        //left face inner
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y2, z1);
        glVertex3f(x1, y2, z2);
        glVertex3f(x1, y1, z2);

        //right face inner
        glVertex3f(x2, y1, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x2, y2, z1);
        glVertex3f(x2, y1, z1);

        //back face outer
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y2, z1);
        glVertex3f(x2, y2, z1);
        glVertex3f(x2, y1, z1);
        //back face inner
        glVertex3f(x2, y1, z1);
        glVertex3f(x2, y2, z1);
        glVertex3f(x1, y2, z1);
        glVertex3f(x1, y1, z1);

        //front face outer
        glVertex3f(x2, y1, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x1, y2, z2);
        glVertex3f(x1, y1, z2);
        //front face inner
        glVertex3f(x1, y1, z2);
        glVertex3f(x1, y2, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x2, y1, z2);

        //left face outer
        glVertex3f(x1, y1, z2);
        glVertex3f(x1, y2, z2);
        glVertex3f(x1, y2, z1);
        glVertex3f(x1, y1, z1);

        //right face outer
        glVertex3f(x2, y1, z1);
        glVertex3f(x2, y2, z1);
        glVertex3f(x2, y2, z2);
        glVertex3f(x2, y1, z2);

    glEnd();
}

void Liquid::computeHeightMap() {

    unsigned turbMax = GRID_DIM.y - 1;
    unsigned turbMin = 0;

    m_HeightMap->clear();
    m_NormalMap->clear();

    //iterate along the first layer of the water and build the heightMap
    for (unsigned x = 0; x < GRID_DIM.x; ++x) {

        //probe down to find the first non-empty cell
        for (unsigned y = GRID_DIM.y - 1; y >= 0; --y) {

            if (!mGrid[0][y][x]->isEmpty()) {

                //check if min or max
                if (y < turbMax) {

                    turbMax = y;
                }
                if (y > turbMin) {

                    turbMin = y;
                }

                //store this point in the height map
                float hx = -(GRID_DIM.x / 2.0) * m_CellSize;
                hx += x * m_CellSize;

                float hy = -(GRID_DIM.y / 2.0) * m_CellSize;
                hy += y * m_CellSize;

                m_HeightMap->push_back(glm::vec3(hx, hy, m_CellSize));
                m_NormalMap->push_back(glm::vec3(0.0f, 1.0f, 0.0f));

                break;
            }
        }
    }

    //set the turbulent min and max
    *m_TurbulentMin = -(GRID_DIM.y / 2.0f) * m_CellSize;
    *m_TurbulentMin *= turbMin * m_CellSize;
    *m_TurbulentMax = -(GRID_DIM.y / 2.0f) * m_CellSize;
    *m_TurbulentMax *= turbMax * m_CellSize;
}
