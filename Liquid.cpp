#include "Liquid.h"

//CONSTRUCTOR
Liquid::Liquid(t_HeightMap* heightMap, t_NormalMap* normalMap,
    float* turbulentMin, float* turbulentMax, float* waterBottom) :
    GRID_DIM(40, 40),
    m_HeightMap(heightMap),
    m_NormalMap(normalMap),
    m_TurbulentMin(turbulentMin),
    m_TurbulentMax(turbulentMax) {

    srand(time(0));

    //calculate the cell size
    m_CellSize = 2.0f / GRID_DIM.x;

    //set the level of the water to be at zero
    for (unsigned y = 0; y < GRID_DIM.y; ++y) {

        m_HeightMap2.push_back(t_HeightRow());

        for (unsigned x = 0; x < GRID_DIM.x; ++x) {

            m_HeightMap2[y].push_back((rand() % 100) / 200.0f);
        }
    }

    //set the bottom of the water
    *waterBottom = -(GRID_DIM.x / 2.0f) * m_CellSize;
}

//DESTRUCTOR
Liquid::~Liquid() {
}

//PUBLIC MEMBER FUNCTIONS
void Liquid::update() {

    //randomise the height map
    for (unsigned y = 0; y < GRID_DIM.y; ++y) {
        for (unsigned x = 0; x < GRID_DIM.x; ++x) {

            m_HeightMap2[y][x] = (rand() % 100) / 200.0f;
        }
    }
}

void Liquid::render(liquid::e_RenderMode renderMode) {

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

//PRIVATE MEMBER FUNCTIONS
void Liquid::renderParticles() {

    //the far offsets
    float x1 = -(GRID_DIM.x / 2.0f) * m_CellSize;
    float y1 = -(GRID_DIM.x / 2.0f) * m_CellSize;
    float z1 = -(GRID_DIM.y / 2.0f) * m_CellSize;
    float halfCell = m_CellSize / 2.0f;

    glColor4f(0.0f, 0.5f, 0.8f, 1.0f);

    //iterate over the grid and draw as particles
    for (unsigned y = 0; y < GRID_DIM.y; ++y) {
        for (unsigned x = 0; x < GRID_DIM.x; ++x) {

            //get the height of the particle
            float y2 = m_HeightMap2[y][x];

            glPushMatrix();

            glTranslatef(x1 + (x * m_CellSize),
                0.0f, z1 + (y * m_CellSize));

            glBegin(GL_QUADS);

                //front face
                glNormal3f(0.0f, 0.0f, 1.0f);
                glVertex3f( halfCell, y1, halfCell);
                glVertex3f( halfCell, y2, halfCell);
                glVertex3f(-halfCell, y2, halfCell);
                glVertex3f(-halfCell, y1, halfCell);

                //back face
                glNormal3f(0.0f, 0.0f, -1.0f);
                glVertex3f(-halfCell, y1, -halfCell);
                glVertex3f(-halfCell, y2, -halfCell);
                glVertex3f( halfCell, y2, -halfCell);
                glVertex3f( halfCell, y1, -halfCell);

                //left face
                glNormal3f(-1.0f, 0.0f, 0.0f);
                glVertex3f(-halfCell, y1,  halfCell);
                glVertex3f(-halfCell, y2,  halfCell);
                glVertex3f(-halfCell, y2, -halfCell);
                glVertex3f(-halfCell, y1, -halfCell);

                //right face
                glNormal3f(1.0f, 0.0f, 0.0f);
                glVertex3f(halfCell, y1, -halfCell);
                glVertex3f(halfCell, y2, -halfCell);
                glVertex3f(halfCell, y2,  halfCell);
                glVertex3f(halfCell, y1,  halfCell);

                //top face
                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(-halfCell, y2, -halfCell);
                glVertex3f(-halfCell, y2,  halfCell);
                glVertex3f( halfCell, y2,  halfCell);
                glVertex3f( halfCell, y2, -halfCell);

                //bottom face
                glNormal3f(0.0f, -1.0f, 0.0f);
                glVertex3f( halfCell, y1, -halfCell);
                glVertex3f( halfCell, y1,  halfCell);
                glVertex3f(-halfCell, y1,  halfCell);
                glVertex3f(-halfCell, y1, -halfCell);

            glEnd();

            glPopMatrix();
        }
    }
}

void Liquid::renderBorder() {

}

// roates the given vector 90 degrees anticlockwise along the z axis
void rotate90(glm::vec3 *v){
    float x = v->x;
    v->x =  v->y;
    v->y = -x;
}

void Liquid::computeHeightMap() {

    float turbMax = -(GRID_DIM.x / 2.0f) * m_CellSize;
    float turbMin = -turbMax;

    m_HeightMap->clear();
    m_NormalMap->clear();

    float x1 = -(GRID_DIM.x / 2.0f) * m_CellSize;

    unsigned middleIndex = static_cast<unsigned>(GRID_DIM.y / 2.0f);

    //iterate over the middle row of the height map
    for (unsigned x = 0; x < GRID_DIM.x; ++x) {

        float height = m_HeightMap2[middleIndex][x];

        float hx = x1 + (x * m_CellSize);

        m_HeightMap->push_back(glm::vec3(hx, height, m_CellSize));

        //check the max and mins
        if (height > turbMax) {

            turbMax = height;
        }
        if (height < turbMin) {

            turbMin = height;
        }
    }

    //compute the normals
    for (unsigned int i=0; i<m_HeightMap->size(); ++i) {
        glm::vec3 ht = (*m_HeightMap)[i];

        // make copies of left and right points, if none exist, they are the same height
        unsigned int index_L = (i == 0)? 0 : i-1;
        unsigned int index_R = glm::min(i+1, (unsigned int)m_HeightMap->size()-1);
        glm::vec3 L = glm::vec3((*m_HeightMap)[index_L]);
        glm::vec3 R = glm::vec3((*m_HeightMap)[index_R]);
        L.x = ht.x - m_CellSize; // force them to be to the left/right
        R.x = ht.x + m_CellSize;

        // convert them to normalized tangents towards +x
        L = glm::normalize(ht - L);
        R = glm::normalize(R - ht);

        // rotate them 90 degrees counter-clockwise to be a normal of the line
        rotate90(&L);
        rotate90(&R);

        // printf("%f %f %f\n", R.x, R.y, R.z);
        // the final normal is a average of the two
        m_NormalMap->push_back(glm::normalize(L + R));
    }

    //set the min and max
    *m_TurbulentMin = turbMin;
    *m_TurbulentMax = turbMax;
}