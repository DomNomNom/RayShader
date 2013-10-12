
varying vec4 vertex_light_position;
varying vec4 v;

uniform vec2 mouse;
uniform float time;
uniform int numTriangles;
uniform int numballs;
uniform vec4 triangles[40];    // this array must be of size >= numTriangles * 3
uniform vec4 ball_pos[20];     // positions
uniform float ball_radius[20]; // radii
// uniform sampler2D sky;
uniform samplerCube skybox;
uniform mat4 cameraTransform;


// ====== local variables ======

// enum hitType
int HIT_TYPE_NO_HIT   = 0;
int HIT_TYPE_SPHERE   = 1;
int HIT_TYPE_TRIANGLE = 2;

// re-tracing information
struct ret {
    vec4 colour;
    vec4 eye;
    vec4 dir;
    bool hit;
};

// float root2 = sqrt(2.0);
float PI = acos(0.0)*2.0; // 3.14...
vec2 leftFront = normalize(vec2(1.0, 1.0));


// ====== functions ======



vec2 randomV = v.xy * sin(time);
float rand() {
    float random = fract(sin(dot(randomV.xy, vec2(12.9898, 78.233)))* 43758.5453);
    randomV = vec2(random, randomV.y);
    return random;
}

vec3 rand3D() {
    return vec3(rand(), rand(), rand());
}

// gets called if a ray does not hit any objects
vec4 specular(vec4 dir) {
    return textureCube(skybox, dir.xyz);

    return clamp((
        1.0 *
        pow(max(dot(dir, vertex_light_position), 0.5), 50.0) *
        vec4(1.0, 1.0, 1.0, 0.0)
    ), 0.0, 1.0);
}

float diffuse (vec4 normal) {  return clamp(dot(normal, vertex_light_position), 0.0, 1.0); }
float diffuse2(vec4 normal) {  return   abs(dot(normal, vertex_light_position)          ); } // a two-sided version.

ret trace(vec4 eye, vec4 dir) {
    dir = normalize(dir);

    int closestType = HIT_TYPE_NO_HIT;
    float closestOffset = -1.0; // should be positive multiples of dir
    int closestThing = 0; // index of a array of the type (either triangles or spheres)

    vec4 closestIntersect = vec4(0.0);
    vec4 closestNormal    = vec4(0.0);


    for (int i=0; i<numTriangles*3; i+=3) {
        vec4 normal = vec4(normalize(cross(triangles[i+1].xyz, triangles[i+2].xyz)), 0.0);

        float normalDot = dot(dir, normal);
        if (normalDot != 0.0) {
            // calculate be bestTriangleOffset
            // vec4 u =
            vec4 p = eye - triangles[i];
            float t = dot(p, normal) / normalDot;

            vec4 intersect = eye - t * dir; // the intersect point


            // float len_ab = length(triangles[i+1]);
            // float len_ac = length(triangles[i+2]);
            // float v = dot(intersect-triangles[i], normalize(triangles[i+1])) / len_ab; // (len_ab * len_ab);
            // float u = dot(intersect-triangles[i], normalize(triangles[i+2])) / len_ab; // (len_ac * len_ab);
            // float uv = u + v;

            vec3 P = vec3(intersect);
            vec3 v0 = vec3(triangles[i]);
            vec3 v1 = vec3(triangles[i] + triangles[i+1]);
            vec3 v2 = vec3(triangles[i] + triangles[i+2]);
            vec3 edge0 = v1 - v0;
            vec3 edge1 = v2 - v1;
            vec3 edge2 = v0 - v2;
            vec3 C0 = P - v0;
            vec3 C1 = P - v1;
            vec3 C2 = P - v2;
            vec3 N = vec3(normal);

            if (
                dot(N, cross(edge0, C0)) > 0.0 &&
                dot(N, cross(edge1, C1)) > 0.0 &&
                dot(N, cross(edge2, C2)) > 0.0
                // 0.0 <= u  && u  <= 1.0 &&
                // 0.0 <= v  && v  <= 1.0 &&
                // 0.0 <= uv && uv <= 1.0
            ) {
                float offset = dot(dir, intersect-eye);
                if (offset > 0.001 && (offset < closestOffset || closestType==HIT_TYPE_NO_HIT)) {
                    closestOffset = offset;
                    closestType = HIT_TYPE_TRIANGLE;
                    closestIntersect = intersect;
                    closestNormal = normal;
                    closestThing = i;
                }
            }
        }
    }

    // find the best ball
    for (int i=0; i<numballs; ++i) {
        vec4 pos = ball_pos[i];
        float radius = ball_radius[i];

        vec4 projection = (eye-pos) - dot(eye-pos, dir)*dir;
        float minDist = length(projection);
        if (minDist <= radius) {
            vec4 intersect = (pos+projection) - sqrt((radius*radius)-(minDist*minDist)) * dir;
            float offset = dot(dir, intersect-eye);
            if (offset > 0.001 && (offset < closestOffset || closestType==HIT_TYPE_NO_HIT)) {
                closestOffset = offset;
                closestType = HIT_TYPE_SPHERE;
                closestIntersect = intersect;
                // closestNormal = ; // we can easily compute it later
                closestThing = i;
            }
            // float offset = dot(dir, eye-intersect);
            // if (offset< 0.0 && (bestBall < 0 || offset > bestOffset)) { // TODO; invert direction
            //     bestBall = i;
            //     bestOffset = offset;
            //     bestintersect = intersect;
            // }
        }
    }


    // render the closest object

    if (closestType == HIT_TYPE_SPHERE) {
        closestNormal = normalize(closestIntersect - ball_pos[closestThing]);
        return ret(
            vec4(1.0, 0.0, 0.0, 0.0) * diffuse(closestNormal),

            closestIntersect,
            reflect(dir, closestNormal),
            true
        );
    }
    else if (closestType == HIT_TYPE_TRIANGLE) {
        return ret(
            vec4(1.0, 0.0, 0.0, 0.0) * diffuse2(closestNormal),

            closestIntersect,
            reflect(dir, closestNormal),
            true
        );
    }
    else { // no hit
        return ret(
            vec4(0.0, 0.0, 0.0, 0.0), //specular(dir),
            vec4(0.0, 0.0, 0.0, 0.0),
            dir,
            false
        );
    }

}




// ====== main ======

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);

    ret r = ret(
        gl_FragColor,
        cameraTransform * vec4(0.0, 0.0, -3.0, 1.0),
        cameraTransform * normalize(vec4(v.xy, 2.0, 0.0)),
        true
    );


    float bounce;
    for (bounce = 0.0; r.hit && bounce<7.0; bounce+=1.0) {
        r = trace(r.eye, r.dir);

        if (r.hit) {
            // result += vec4(0.2, 0.0, 0.0, 0.0); // ambient
            vec4 diffuse = r.colour * pow(0.5, bounce+1.0);
            if (trace(r.eye + vec4(rand3D(), 0.0), vertex_light_position).hit) // shadow
                diffuse *= 0.7;
            gl_FragColor += diffuse;
        }

    }


    gl_FragColor += specular(r.dir) * pow(0.85, bounce);
}