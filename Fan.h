#ifndef RAYSHADER_FAN_H_
#	define RAYSHADER_FAN_H_

#include "glm/glm.hpp"
#include <iostream>
#include <math.h>

#include "util.h"

class Fan {
public:

	//CONSTRUCTOR
	Fan(const glm::vec3& position);

	//DESTRUCTOR
	~Fan();

	//PUBLIC MEMBER FUNCTIONS
	/*!Computes the height for the given point*/
	float computeHeight(const glm::vec2& point, float height) const;

	void setPos(const glm::vec3& position);

private:

	//VARIABLES
	//the position of the ripple point
	glm::vec3 m_Position;

    //MACROS
    DISALLOW_COPY_AND_ASSIGN(Fan);
};

#endif