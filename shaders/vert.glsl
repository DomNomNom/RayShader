varying vec3 vertex_light_position;
varying vec3 vertex_normal;
varying vec3 v;
varying vec4 position;



uniform vec2 mouse;
uniform float time;
uniform int drift;
uniform vec4 balls[2];
uniform vec4 omg;


float bump(float center, float range, float x) {
    return (
        (       smoothstep(center-range,    center,         x)) *
        (1.0 -  smoothstep(center,          center+range,   x))
    );
}

void main() {
    vertex_normal = normalize(gl_NormalMatrix * gl_Normal);  // Calculate the normal value for this vertex, in world coordinates (multiply by gl_NormalMatrix)

    vertex_light_position = normalize(gl_LightSource[0].position.xyz);  // Calculate the light position for this vertex

    // Set the front color to the color passed through with glColor*f
    // gl_FrontColor = gl_Color;
    v = vec3(gl_ModelViewProjectionMatrix * gl_Vertex); //gl_Position;

    // vec4 f = gl_Vertex;// + (0.0, 1000.0, 0.0, 0.0);
    // position = gl_Vertex;

    // float x = gl_Vertex.x;
    // float val = 0.01*cos(-10.0*x+12.0*time);
    // val *= bump(-0.25,0.7, x);

    mat4 transform = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Set the position of the current vertex
    gl_Position = (
        gl_ProjectionMatrix *
        gl_ModelViewMatrix *
        gl_Vertex *
        transform
    );

    // position = gl_Position;
    // if (gl_Vertex.z > 0.0) gl_Position = vec4(0.0,0.0,0.0,1.0);
}