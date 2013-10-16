#include "Liquid.h"

//VARIABLES
//the strength of gravity
float Liquid::sGravityStrength = 1.0f;
//the direction of gravity
glm::vec3 Liquid::sGravityDirection(0.0f, -1.0f, 0.0f);

//CONSTRUCTOR
Liquid::Liquid() :
    GRID_DIM(40, 20, 20),
    CELL_SIZE(0.02f),
    mWave(0.0f),
    mWaveUp(true) {

    //seed the random number generator
    srand(time(0));

    //create the grid
    for (unsigned z = 0; z < GRID_DIM.z; ++z) {

        //create a new slice in the grid
        mGrid.push_back(LiquidSlice());

        for (unsigned y = 0; y < GRID_DIM.y; ++y) {

            //create a new row in the slice
            mGrid[z].push_back(LiquidRow());

            for (unsigned x = 0; x < GRID_DIM.x; ++x) {

                LiquidCell* cell = new LiquidCell(glm::vec3(x, y, z), CELL_SIZE);

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
}

//DESTRUCTOR
Liquid::~Liquid() {
}

//PUBLIC MEMBER FUNCTIONS
void Liquid::update() {

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

void Liquid::render(bool polygonWrap) {

    //render with polygon wrapping
    if (polygonWrap) {

        //still render the particles if polygon debugging
        if (POLYGON_DEBUG) {

            renderParticles();
        }

        renderPolygonWrap();
    }
    //render as particles
    else {

        renderParticles();
    }

    //render the grid if in debug mode
    // if (DRAW_GRID) {

    //     renderGrid();
    // }
    // //render the border (excluding the top)
    // else {

    //     renderBorder();
    // }
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
void Liquid::renderPolygonWrap() {

    //probe to find the deepest concave and highest convex
    unsigned deepestConcave = GRID_DIM.y - 1;
    unsigned highestConvex  = 0;

    for (unsigned z = 0; z < GRID_DIM.z; ++z) {
        for (unsigned y = 0; y < GRID_DIM.y; ++y) {
            for (unsigned x = 0; x < GRID_DIM.x; ++x) {

                if (mGrid[z][y][x]->isEmpty() && y < deepestConcave) {

                    deepestConcave = y;
                }

                if (!mGrid[z][y][x]->isEmpty() && y > highestConvex) {

                    highestConvex = y;
                }
            }
        }
    }

    //produce the the polygons for the lower half
    FaceList* nonTurbPolys = computeNonTurbulent(deepestConcave);

    //produce the polygons for the upper half
    FaceList* turbPolys = computeTurbulent(highestConvex);

    //render the polygons
    renderPolygons(nonTurbPolys);
    renderPolygons(turbPolys);

    //clean up
    nonTurbPolys->clear();
    delete nonTurbPolys;
    turbPolys->clear();
    delete turbPolys;
}   

FaceList* Liquid::computeNonTurbulent(unsigned top) const {

    float x1 = -((GRID_DIM.x / 2.0f) * CELL_SIZE);
    float x2 = -x1;
    float y1 = -((GRID_DIM.y / 2.0f) * CELL_SIZE);
    float y2 = y1 + (top * CELL_SIZE);
    float z1 = -((GRID_DIM.z / 2.0f) * CELL_SIZE);
    float z2 = -z1;

    //the face list
    FaceList* faces = new FaceList();

    //add the triangles
    //front face
    faces->push_back(glm::vec3(x2, y1, z2));
    faces->push_back(glm::vec3(x2, y2, z2));
    faces->push_back(glm::vec3(x1, y2, z2));
    faces->push_back(glm::vec3(x1, y2, z2));
    faces->push_back(glm::vec3(x1, y1, z2));
    faces->push_back(glm::vec3(x2, y1, z2));

    //back face
    faces->push_back(glm::vec3(x1, y1, z1));
    faces->push_back(glm::vec3(x1, y2, z1));
    faces->push_back(glm::vec3(x2, y2, z1));
    faces->push_back(glm::vec3(x2, y2, z1));
    faces->push_back(glm::vec3(x2, y1, z1));
    faces->push_back(glm::vec3(x1, y1, z1));

    //left face
    faces->push_back(glm::vec3(x1, y1, z2));
    faces->push_back(glm::vec3(x1, y2, z2));
    faces->push_back(glm::vec3(x1, y2, z1));
    faces->push_back(glm::vec3(x1, y2, z1));
    faces->push_back(glm::vec3(x1, y1, z1));
    faces->push_back(glm::vec3(x1, y1, z2));

    //right face
    faces->push_back(glm::vec3(x2, y1, z1));
    faces->push_back(glm::vec3(x2, y2, z1));
    faces->push_back(glm::vec3(x2, y2, z2));
    faces->push_back(glm::vec3(x2, y2, z2));
    faces->push_back(glm::vec3(x2, y1, z2));
    faces->push_back(glm::vec3(x2, y1, z1));

    //bottom face
    faces->push_back(glm::vec3(x2, y1, z1));
    faces->push_back(glm::vec3(x2, y1, z2));
    faces->push_back(glm::vec3(x1, y1, z2));
    faces->push_back(glm::vec3(x1, y1, z2));
    faces->push_back(glm::vec3(x1, y1, z1));
    faces->push_back(glm::vec3(x2, y1, z1));

    return faces;
}

FaceList* Liquid::computeTurbulent(unsigned top) const {

    //recursively find the polygons
    FaceList* faces = computeTurbulentRec(
        glm::vec3(0, top, 0), glm::vec3(GRID_DIM.x - 1, top, GRID_DIM.z - 1));

    return faces;
}

FaceList* Liquid::computeTurbulentRec(const glm::vec3& topLeft,
    const glm::vec3& bottomRight) const {

    int currentY = -1;
    unsigned maxX = bottomRight.x;
    unsigned maxZ = bottomRight.z;

    //break the square down into faces
    for (unsigned z = topLeft.z; z <= maxZ; ++z) {
        for (unsigned x = topLeft.x; x <= maxX; ++x) {

            //find the first y position
            if (currentY == -1) {

                //probe down
                for (int y = topLeft.y; y >= 0; --y) {

                    //record the y index of the first non empty cell
                    if (!mGrid[z][y][x]->isEmpty()) {

                        currentY = y;
                        break;
                    }
                }
            }
            else {

                //probe down
                for (int y = topLeft.y; y >= 0; --y) {

                    if (!mGrid[z][y][x]->isEmpty()) {

                        //we found a cell that is at a different height
                        if (y != currentY) {

                            if (x <= maxX && z != topLeft.z) {

                                //exit the entire loop
                                maxZ = z -1;
                                goto LOOP_EXIT;
                            }
                            else {

                                //set the new max x index
                                maxX = x - 1;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    LOOP_EXIT:

    //the list of faces
    FaceList* faces = new FaceList();

    //check if we need to make more recursive calls
    if (maxZ < bottomRight.z) {

        //create polygons on the z side of this
        FaceList* facesZ = computeTurbulentRec(
            glm::vec3(topLeft.x, topLeft.y, maxZ + 1),
            glm::vec3(bottomRight.x, topLeft.y, bottomRight.z));

        //add to the face list
        for (FaceList::const_iterator it = facesZ->begin();
            it != facesZ->end(); ++it) {

            faces->push_back(*it);
        }

        facesZ->clear();
        delete facesZ;
    }
    if (maxX < bottomRight.x) {

        //create polygons on the x side of this
        FaceList* facesX = computeTurbulentRec(
            glm::vec3(maxX + 1, topLeft.y, topLeft.z),
            glm::vec3(bottomRight.x, topLeft.y, bottomRight.z));

        //add to the face list
        for (FaceList::const_iterator it = facesX->begin();
            it != facesX->end(); ++it) {

            faces->push_back(*it);
        }

        facesX->clear();
        delete facesX;
    }

    float x1 = -((GRID_DIM.x / 2.0f) * CELL_SIZE);
    float y = -((GRID_DIM.y / 2.0f) * CELL_SIZE);
    float z1 = -((GRID_DIM.z / 2.0f) * CELL_SIZE);

    y += (CELL_SIZE * (currentY + 1));

    float x2 = x1 + (CELL_SIZE * (maxX + 1));
    float z2 = z1 + (CELL_SIZE * (maxZ + 1));

    x1 += (CELL_SIZE * topLeft.x);
    z1 += (CELL_SIZE * topLeft.z);

    faces->push_back(glm::vec3(x1, y, z1));
    faces->push_back(glm::vec3(x1, y, z2));
    faces->push_back(glm::vec3(x2, y, z2));
    faces->push_back(glm::vec3(x2, y, z2));
    faces->push_back(glm::vec3(x2, y, z1));
    faces->push_back(glm::vec3(x1, y, z1));

    return faces;
}

void Liquid::renderPolygons(FaceList* faces) {

    //render as normal filled in polygons
    if (!POLYGON_DEBUG) {

        glColor4f(0.5f, 0.8f, 1.0f, 1.0f);

        glBegin(GL_TRIANGLES);

            for (unsigned i = 0; i < faces->size(); i += 3) {

                //the points of the triangle
                glm::vec3 p1 = (*faces)[i];
                glm::vec3 p2 = (*faces)[i + 1];
                glm::vec3 p3 = (*faces)[i + 2];

                //compute the normal
                glm::vec3 u = p2 - p1;
                glm::vec3 v = p3 - p1;

                glm::vec3 normal;
                normal.x = (u.y * v.z) - (u.z * v.y);
                normal.y = (u.z * v.x) - (u.x * v.z);
                normal.z = (u.x * v.y) - (u.y * v.x);

                normal = glm::normalize(normal);

                glNormal3f(normal.x, normal.y, normal.z);

                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
                glVertex3f(p3.x, p3.y, p3.z);
            }

            for (FaceList::iterator it = faces->begin();
                it != faces->end(); ++it) {

                glVertex3f(it->x, it->y, it->z);
            }

        glEnd();
    }
    //render in wireframe
    else {

        glDisable(GL_LIGHTING);

        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

        glBegin(GL_LINES);

            for (unsigned i = 0; i < faces->size(); i += 3) {

                glVertex3f((*faces)[i].x, (*faces)[i].y, (*faces)[i].z);
                glVertex3f((*faces)[i + 1].x,
                    (*faces)[i + 1].y, (*faces)[i + 1].z);
                glVertex3f((*faces)[i + 1].x,
                    (*faces)[i + 1].y, (*faces)[i + 1].z);
                glVertex3f((*faces)[i + 2].x,
                    (*faces)[i + 2].y, (*faces)[i + 2].z);
                glVertex3f((*faces)[i + 2].x,
                    (*faces)[i + 2].y, (*faces)[i + 2].z);
                glVertex3f((*faces)[i].x, (*faces)[i].y, (*faces)[i].z);
            }

        glEnd();

        glEnable(GL_LIGHTING);
    }
}


void Liquid::renderParticles() {

    float x1 = -((GRID_DIM.x / 2.0f) * CELL_SIZE);
    float y1 = -((GRID_DIM.y / 2.0f) * CELL_SIZE);
    float z1 = -((GRID_DIM.z / 2.0f) * CELL_SIZE);

    //render the cells
    for (unsigned z = 0; z < GRID_DIM.z; ++z) {
        for (unsigned y = 0; y < GRID_DIM.y; ++y) {
            for (unsigned x = 0; x < GRID_DIM.x; ++x) {

                mGrid[z][y][x]->render(
                    glm::vec3(x1 + (x * CELL_SIZE),
                              y1 + (y * CELL_SIZE),
                              z1 + (z * CELL_SIZE)));
            }
        }
    }
}

void Liquid::renderGrid() {

    float x1 = -((GRID_DIM.x / 2.0f) * CELL_SIZE);
    float x2 = -x1;
    float y1 = -((GRID_DIM.y / 2.0f) * CELL_SIZE);
    float y2 = -y1;
    float z1 = -((GRID_DIM.z / 2.0f) * CELL_SIZE);

    glColor4f(0.0f, 0.0f, 0.0f, 0.1f);

    for (unsigned z  = 0; z < GRID_DIM.z + 1; ++z) {

        float currentZ = z1 + (z * CELL_SIZE);

        for (unsigned y = 0; y < GRID_DIM.y + 1; ++y) {

            glBegin(GL_LINES);
                glVertex3f(x1, y1 + (y * CELL_SIZE), currentZ);
                glVertex3f(x2, y1 + (y * CELL_SIZE), currentZ);
            glEnd();
        }

        for (unsigned x = 0; x < GRID_DIM.x + 1; ++x) {

            glBegin(GL_LINES);
                glVertex3f(x1 + (x * CELL_SIZE), y1, currentZ);
                glVertex3f(x1 + (x * CELL_SIZE), y2, currentZ);
            glEnd();
        }
    }
}

void Liquid::renderBorder() {

    float x1 = -((GRID_DIM.x / 2.0f) * CELL_SIZE) - 0.001f;
    float x2 = -x1;
    float y1 = -((GRID_DIM.y / 2.0f) * CELL_SIZE) - 0.001f;
    float y2 = -y1;
    float z1 = -((GRID_DIM.z / 2.0f) * CELL_SIZE) - 0.001f;
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