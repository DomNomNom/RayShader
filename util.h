#ifndef RAYSHADER_VALUES_H_
#   define RAYSHADER_VALUES_H_

const bool DRAW_GRID = false;
const bool POLYGON_DEBUG = false;

const float DEGREES_TO_RADIANS = 0.0174532925;
const float RADIANS_TO_DEGREES = 57.2957795;

const float PI = acos(0.0) * 2.0;

#define DISALLOW_CONSTRUCTION(TypeName) \
TypeName();                            \
TypeName(const TypeName&);             \
void operator=(const TypeName&)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);             \
void operator=(const TypeName&)

#endif