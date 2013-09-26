
varying vec3 vertex_light_position;
varying vec3 vertex_normal;
varying vec3 v;
varying vec4 position;

uniform vec2 mouse;
uniform float time;
uniform int numTriangles;
uniform int numballs;
uniform vec4 triangles[20];
uniform vec4 ball_pos[20];  // positions
uniform float ball_radius[20]; // radii
// uniform sampler2D sky;
uniform samplerCube skybox;

// ====== local variables ======

// enum hitType
int HIT_TYPE_NO_HIT   = 0;
int HIT_TYPE_SPHERE   = 1;
int HIT_TYPE_TRIANGLE = 2;

// re-tracing information
struct ret {
    vec4 colour;
    vec3 eye;
    vec3 dir;
    bool hit;
};

// float root2 = sqrt(2.0);
float PI = acos(0.0); // 3.14...
vec2 leftFront = normalize(vec2(1.0, 1.0));


// ====== functions ======

// vec3 horizontalLeftFront = normalize(vec3(-1.0, 0.0, -1.0)); // the point where the texture coordinates are (0.0,  0.5)
// vec3 horizontalLeftBack  = normalize(vec3(-1.0, 0.0,  1.0)); // the point where the texture coordinates are (0.25, 0.5)
vec4 getSkybox(vec3 dir) {
    // return vec4(0.0, dir.y, 0.0, 0.0);
    return textureCube(skybox, dir);
    // // float crossFromLeftFront = cross(horizontalLeftFront, dir).y; // TODO: this can be optimized
    // // float crossFromLeftBack  = cross(horizontalLeftBack , dir).y;
    // vec2 texPos = vec2(0.0, 0.5+0.5*dir.y); // texPos.x will change
    // if (abs(dir.y) != 1.0) { // if we're going straight up/down, we say that texPos.x=0.0 is good enough
    //     vec2 topVec = normalize(dir.xz); // dir viewed from above. (y=z) this helps us decide which face to pick

    //     // calculate the angle (from http://stackoverflow.com/questions/14066933/direct-way-of-computing-clockwise-angle-between-2-vectors)
    //     float det = leftFront.x*topVec.y - leftFront.y*topVec.x;
    //     float angle = atan(det, dot(leftFront, topVec));
    //     texPos.x = (angle / -2.0*PI)+1.0;
    // }
    // return texture2D(sky, texPos);
}

// gets called if a ray does not hit any objects
vec4 specular(vec3 dir) {
    return getSkybox(dir);
    return clamp((
        1.0 *
        pow(max(dot(dir, vertex_light_position), 0.5), 50.0) *
        vec4(1.0, 1.0, 1.0, 0.0)
    ), 0.0, 1.0);
}

float diffuse (vec3 normal) {  return clamp(dot(normal, vertex_light_position), 0.0, 1.0); }
float diffuse2(vec3 normal) {  return   abs(dot(normal, vertex_light_position)          ); } // a two-sided version.

ret trace(vec3 eye, vec3 dir) {
    dir = normalize(dir);

    int closestType = HIT_TYPE_NO_HIT;
    float closestOffset = -1.0; // should be positive multiples of dir
    int closestThing = 0; // index of a array of the type (either triangles or spheres)

    vec3 closestIntersect = vec3(0.0);
    vec3 closestNormal    = vec3(0.0);
    // int bestBall = -1;
    // float bestOffset = 0.0;
    // vec3 bestSphereIntersect = vec3(0,0,0);

    // int bestTriangle = -1;
    // float bestTriangleOffset = 0.0;
    // vec3 bestTriangleIntersect;
    // vec3 bestTriangleNormal;


    for (int i=0; i<numTriangles*3; i+=3) {
        vec3 normal = normalize(cross(triangles[i+1].xyz, triangles[i+2].xyz));

        float normalDot = dot(dir, normal);
        if (normalDot != 0.0) {
            // calculate be bestTriangleOffset
            // vec3 u =
            vec3 p = eye - triangles[i].xyz;
            float t = dot(p, normal) / normalDot;

            vec3 intersect = eye - t * dir; // the intersect point


            float len_ab = length(triangles[i+1]);
            float len_ac = length(triangles[i+2]);
            float v = dot(intersect-triangles[i].xyz, triangles[i+1].xyz) / (len_ab * len_ab);
            float u = dot(intersect-triangles[i].xyz, triangles[i+2].xyz) / (len_ac * len_ac);
            float uv = u + v;

            if (
                0.0 <= u  && u  <= 1.0 &&
                0.0 <= v  && v  <= 1.0 &&
                0.0 <= uv && uv <= 1.0
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

        vec3 projection = (eye-pos.xyz) - dot(eye-pos.xyz, dir)*dir;
        float minDist = length(projection);
        if (minDist <= radius) {
            vec3 intersect = (pos.xyz+projection) - sqrt((radius*radius)-(minDist*minDist)) * dir;
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
        closestNormal = normalize(closestIntersect - ball_pos[closestThing].xyz);
        return ret(
            vec4(0.5, 0.0, 0.0, 0.0) * diffuse(closestNormal),

            closestIntersect,
            reflect(dir, closestNormal),
            true
        );
    }
    else if (closestType == HIT_TYPE_TRIANGLE) {
        return ret(
            vec4(0.5, 0.0, 0.0, 0.0) * diffuse2(closestNormal),

            closestIntersect,
            reflect(dir, closestNormal),
            true
        );
    }
    else {
        return ret(
            specular(dir),
            vec3(0.0, 0.0, 0.0),
            vec3(0.0, 0.0, 0.0),
            false
        );
    }

}

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    ret r = ret(gl_FragColor, vec3(0.0, 0.0, -2.0), normalize(vec3(v.xy, 2.0)), true);


    for (float bounce = 1.0; r.hit && bounce<7.0; bounce+=1.0) {
        r = trace(r.eye, r.dir);

        vec4 result = r.colour;
        if (r.hit) {
            result += vec4(0.2, 0.0, 0.0, 0.0); // ambient
            if (! trace(r.eye, vertex_light_position).hit) // shadow
                result += r.colour; // diffuse
            result *= pow(0.5, bounce);
        }
        else {
            result += r.colour; // specular
            result *= pow(0.75, bounce); // it looks better if specular light bounces more
        }
        // result = clamp(result, 0.0, 1.0);

        gl_FragColor += result;// * pow(0.75, bounce);

    }

    // gl_FragColor = clamp(gl_FragColor, 0.0, 1.0);
    // for (int i=0; i<numballs; ++i) {
    //     vec4 ball = balls[i];
    //     if (distance(v.xy, ball.xy) < ball.w)
    //         gl_FragColor = balls[1];
    // }

}