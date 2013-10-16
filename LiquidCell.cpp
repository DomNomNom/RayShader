#include "LiquidCell.h"

//VARIABLES
unsigned LiquidCell::sId = 0;
liquid_cell::Buffer LiquidCell::sBuffer = liquid_cell::FIRST;
liquid_cell::Buffer LiquidCell::sNextBuffer = liquid_cell::SECOND;

//CONSTRUCTOR
LiquidCell::LiquidCell(const glm::vec3& position, float size) :
    mId(++sId),
    mPosition(position),
    mSize(size),
    mLastBuffer(sNextBuffer) {

    //add data objects to the data buffers
    for (unsigned i = 0; i < BUFFER_COUNT; ++i) {

        mDataBuffers.push_back(new liquid_cell::CellData());
    }
}

//DESTRUCTOR
LiquidCell::~LiquidCell() {

    while (!mDataBuffers.empty()) {

        delete mDataBuffers.back();
        mDataBuffers.pop_back();
    }
}

//PUBLIC MEMBER FUNCTIONS
void LiquidCell::init(bool empty) {

    if (!empty) {

        //set the velocity of the active buffer to gravity
        mDataBuffers[sBuffer]->mVelocity = Liquid::getGravity();
    }
    else {

        mDataBuffers[sBuffer]->mVelocity = glm::vec3();
    }

    //initialise the other buffered data
    for (unsigned i = 1; i < BUFFER_COUNT; ++i) {

        mDataBuffers[i]->mVelocity = glm::vec3();
    }
}


void LiquidCell::render(const glm::vec3& offset) {

    if (!isEmpty()) {

        float quaterSize = mSize / 4.0f;
        float x1 = offset.x + quaterSize;
        float x2 = offset.x + (quaterSize * 3.0f);
        float xMid = x1 + ((x2 - x1) / 2.0f);
        float y1 = offset.y + quaterSize;
        float y2 = offset.y + (quaterSize * 3.0f);
        float z1 = offset.z + quaterSize;
        float z2 = offset.z + (quaterSize * 3.0f);
        float zMid = z1 + ((z2 - z1) / 2.0f);

        glColor4f(0.3f, 0.0f, 0.8f, 1.0f);

        glBegin(GL_QUADS);

            //front face
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(x2, y1, z2);
            glVertex3f(x2, y2, z2);
            glVertex3f(x1, y2, z2);
            glVertex3f(x1, y1, z2);

            //back face
            glNormal3f(0.0f, 0.0f, -1.0f);
            glVertex3f(x1, y1, z1);
            glVertex3f(x1, y2, z1);
            glVertex3f(x2, y2, z1);
            glVertex3f(x2, y1, z1);

            //left face
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(x1, y1, z2);
            glVertex3f(x1, y2, z2);
            glVertex3f(x1, y2, z1);
            glVertex3f(x1, y1, z1);

            //right face
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(x2, y1, z1);
            glVertex3f(x2, y2, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x2, y1, z2);

            //top face
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(x1, y2, z1);
            glVertex3f(x1, y2, z2);
            glVertex3f(x2, y2, z2);
            glVertex3f(x2, y2, z1);

            //bottom face
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(x2, y1, z1);
            glVertex3f(x2, y1, z2);
            glVertex3f(x1, y1, z2);
            glVertex3f(x1, y1, z1);

        glEnd();
    }
}

void LiquidCell::switchBuffers() {

    sBuffer     = liquid_cell::Buffer((sBuffer + 1) % BUFFER_COUNT);
    sNextBuffer = liquid_cell::Buffer((sBuffer + 1) % BUFFER_COUNT);
}

liquid_cell::Buffer LiquidCell::getCurrentBuffer() {

    return sBuffer;
}

void LiquidCell::clearNextData() {

    mDataBuffers[sNextBuffer]->mVelocity = glm::vec3();
}

unsigned LiquidCell::getId() const {

    return mId;
}

bool LiquidCell::isEmpty() const {

    return  fabs(mDataBuffers[sBuffer]->mVelocity.x) < 0.001f &&
            fabs(mDataBuffers[sBuffer]->mVelocity.y) < 0.001f &&
            fabs(mDataBuffers[sBuffer]->mVelocity.z) < 0.001f;
}

const liquid_cell::CellData* LiquidCell::getData() const {

    return mDataBuffers[sBuffer];
}

void LiquidCell::addVelocity(const glm::vec3& velocity) {

    //check if the first time we have made a change since the last buffer switch
    if (sBuffer != mLastBuffer) {

        //add gravity
        mDataBuffers[sNextBuffer]->mVelocity = Liquid::getGravity();
        mLastBuffer = sBuffer;
    }

    mDataBuffers[sNextBuffer]->mVelocity += velocity;
}

void LiquidCell::setVelocity(const glm::vec3& velocity) {

    mDataBuffers[sBuffer]->mVelocity = velocity;
}