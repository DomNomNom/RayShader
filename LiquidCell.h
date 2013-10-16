#ifndef RAYSHADER_LIQUID_LIQUIDCELL_H_
#   define RAYSHADER_LIQUID_LIQUIDCELL_H_

#include <GL/glut.h>
#include <iostream>
#include <math.h>
#include <vector>

#include "glm/glm.hpp"

#include "Liquid.h"
#include "util.h"

//DEFINE
//the number of buffers we are using
#define BUFFER_COUNT 2

namespace liquid_cell {

    //ENUMERATORS
    //the buffers
    enum Buffer {

        FIRST = 0,
        SECOND
    };

    //STRUCTURES
    //the buffered data of the cell
    struct CellData {

        //the velocity of the cell
        glm::vec3 mVelocity;
    };

} //liquid cell

class LiquidCell {
public:

    //CONSTRUCTOR
    /*!Creates a new liquid cell
    @position the grid position of the cell
    @size the size of the cell
    @empty if the cell is empty*/
    LiquidCell(const glm::vec3& position, float size);

    //DESTRUCTOR
    /*!Destroys the liquid cell*/
    ~LiquidCell();

    //PUBLIC MEMBER FUNCTIONS
    void init(bool empty);

    /*!Renders the cell
    @offset the offset of the cell*/
    void render(const glm::vec3& offset);

    /*!Switchs the current buffer of all cells*/
    static void switchBuffers();

    /*!@return the current buffer*/
    static liquid_cell::Buffer getCurrentBuffer();

    /*!Clears the data in the buffer*/
    void clearNextData();

    /*!@return the id of the cell*/
    unsigned getId() const;

    /*!@return if there is no liquid in the cell*/
    bool isEmpty() const;

    /*!@return the current buffer cell data*/
    const liquid_cell::CellData* getData() const;

    /*!@velocity the velocity to add to the next active data in the buffer*/
    void addVelocity(const glm::vec3& velocity);

    /*!@velocity the new velocity of the cell*/
    void setVelocity(const glm::vec3& velocity);

private:

    //VARIABLES
    //static id counter
    static unsigned sId;
    //the unique id of the cell
    unsigned mId;

    //the grid position of the cell
    glm::vec3 mPosition;
    //the size of this cell
    float mSize;

    //the buffer we are currently using
    static liquid_cell::Buffer sBuffer;
    //the next buffer we will be using
    static liquid_cell::Buffer sNextBuffer;
    //the last buffer this cell has used
    liquid_cell::Buffer mLastBuffer;
    //the data buffers
    std::vector<liquid_cell::CellData*> mDataBuffers;

    //MACROS
    DISALLOW_COPY_AND_ASSIGN(LiquidCell);
};

#endif