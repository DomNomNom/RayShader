
varying vec4 vertex_light_position;
varying vec4 v;

// uniforms

uniform vec2 mouse;
uniform float time;
uniform int numTriangles;
uniform int numBalls;
uniform int numWater;
uniform vec4 vertecies[40];
uniform int triangles[40];
uniform vec4 ball_pos[20];     // positions
uniform float ball_radius[20]; // radii
uniform samplerCube skybox;
uniform bool skybox_enabled;
uniform mat4 cameraTransform;

uniform vec3 water        [40];
uniform vec3 water_normals[40];
uniform float turbulent_min;
uniform float turbulent_max;

// ====== local variables ======

int shadowSamples = 4;
float shadowPerSample = pow(0.50, 1.0/float(shadowSamples));

// enum hitType
int HIT_TYPE_NO_HIT   = 0;
int HIT_TYPE_SPHERE   = 1;
int HIT_TYPE_TRIANGLE = 2;
int HIT_TYPE_INITIAL  = 3;
int HIT_TYPE_WATER    = 4;

// float root2 = sqrt(2.0);
float PI = acos(0.0)*2.0; // 3.14...
vec2 leftFront = normalize(vec2(1.0, 1.0));

bool debug = false;

// ====== ret ======

// re-tracing information
struct ret {
    vec4 normal;
    vec4 eye;
    vec4 dir;
    float t; // intersect = eye + t*dir
    int hit;   // what hitType it is
    int thing; // the index of what we collided with
};


// returns a new ret saying that there was no hit
ret noHit() {
    return ret(
        vec4(0.0),
        vec4(0.0),
        vec4(0.0),
        9001.0, // something far away
        HIT_TYPE_NO_HIT,
        0
    );
}


// if B is closer than A, the contents of B are put in A
ret min_ret(ret a, ret b) {
    if (a.hit == HIT_TYPE_NO_HIT || b.t < a.t)
        return b;
    return a;
}

// ====== functions ======

vec2 randomV = v.xy * sin(time);
float rand() {
    float random = fract(sin(dot(randomV.xy, vec2(12.9898, 78.233)))* 43758.5453)  *2.0 - 1.0;
    randomV = vec2(random, randomV.y+1.0);
    return random;
}

vec4 rand3D() {
    return vec4(rand(), rand(), rand(), 0.0);
}

// gets called if a ray does not hit any objects
vec4 specular(vec4 dir) {
    if (skybox_enabled)
        return textureCube(skybox, dir.xyz);

    return clamp((
        1.0 *
        pow(max(dot(dir, vertex_light_position), 0.5), 50.0) *
        vec4(1.0, 1.0, 1.0, 0.0)
    ), 0.0, 1.0);
}

float diffuse (vec4 normal) {  return clamp(dot(normal, vertex_light_position), 0.0, 1.0); }
float diffuse2(vec4 normal) {  return   abs(dot(normal, vertex_light_position)          ); } // a two-sided version.

bool floatZero(float f) {
    return (-0.0001 < f && f < 0.0001);
}

// takes only the x and y component
vec3 twoD(vec4 v) { return vec3(v.xy, 0.0); }
vec3 twoD(vec3 v) { return vec3(v.xy, 0.0); }

// ====== trace functions ======

ret trace_water(vec4 eye, vec4 dir) {
    ret r = noHit();
    vec3 v = twoD(dir);
    vec3 p = twoD(eye);
    for (int i=0; i<numWater-1; ++i) {
        // ray segment intersection from: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
        vec3 q  = twoD(water[i  ]);
        vec3 q1 = twoD(water[i+1]);
        vec3 s = q1 - q;

        float c = cross(v, s).z;
        if (floatZero(c)) continue; // parallel case
        float t = cross(q - p, s).z / c;
        float u = cross(q - p, v).z / c;

        if (u < 0.0 || u > 1.0) continue; // not in segment range

        if (t > 0.0001 && t < r.t || r.t==0.0) {
            vec4 intersect = eye + t*dir;
            if (intersect.z < -1.0 || intersect.z > 1.0) continue; // z bounds
            r.t = t;
            r.eye = intersect;
            r.hit = HIT_TYPE_WATER;
            // debug = true;
            r.normal = vec4(normalize(
                // water_normals[i]
                mix( // interpolate the normals smoothly
                    water_normals[i],
                    water_normals[i+1],
                    smoothstep(0.0, 1.0, u)
                )
                // + 0.1*vec3(0.0, 0.0, sin(intersect.z * 10.0))
            ), 0.0);
        }
    }


    return r;
}

ret trace_triangles(vec4 eye, vec4 dir) {
    ret r = noHit();
    for (int i=0; i<numTriangles*3; i+=3) {

        // Möller Trumbore method
        vec3 edge2 = vertecies[triangles[i+1]].xyz - vertecies[triangles[i  ]].xyz;
        vec3 edge1 = vertecies[triangles[i+2]].xyz - vertecies[triangles[i  ]].xyz;
        vec3 pvec = cross(dir.xyz, edge2);
        float det = dot(edge1, pvec);
        if (det == 0.0) continue;

        float invDet = 1.0 / det;
        vec3 tvec = vec3(eye - vertecies[triangles[i  ]]);
        vec2 isectData; // UV vector (xy=uv)
        isectData.x = dot(tvec, pvec) * invDet;
        if (isectData.x < 0.0 || isectData.x > 1.0) continue;

        vec3 qvec = cross(tvec, edge1);
        isectData.y = dot(dir.xyz, qvec) * invDet;
        if (isectData.y < 0.0 || isectData.x + isectData.y > 1.0) continue;

        float tt = dot(edge2, qvec) * invDet;
        vec4 intersect = eye + tt * dir; // the intersect point
        float t = dot(dir, intersect-eye);
        if (t > 0.001 && (t < r.t || r.hit==HIT_TYPE_NO_HIT)) {
            r.t = t;
            r.hit = HIT_TYPE_TRIANGLE;
            r.eye = intersect;
            r.normal = vec4(normalize(cross(edge1, edge2)), 0.0);
            r.thing = i;
        }
        else { // no hit
        }




        // // baycentric coordinate method
        // vec3 v0 = vec3(vertecies[triangles[i  ]]);
        // vec3 v1 = vec3(vertecies[triangles[i+1]]);
        // vec3 v2 = vec3(vertecies[triangles[i+2]]);
        // vec4 normal = vec4(normalize(cross(v1-v0, v2-v0)), 0.0);

        // float normalDot = dot(dir, normal);
        // if (normalDot != 0.0) {
        //     vec4 p = eye - vertecies[triangles[i]];
        //     float t = dot(p, normal) / normalDot;

        //     vec4 intersect = eye - t * dir; // the intersect point


        //     vec3 P = vec3(intersect);
        //     vec3 edge0 = v1 - v0;
        //     vec3 edge1 = v2 - v1;
        //     vec3 edge2 = v0 - v2;
        //     vec3 C0 = P - v0;
        //     vec3 C1 = P - v1;
        //     vec3 C2 = P - v2;
        //     vec3 N = vec3(normal);


        //     if (
        //         dot(N, cross(edge0, C0)) > 0.0 &&
        //         dot(N, cross(edge1, C1)) > 0.0 &&
        //         dot(N, cross(edge2, C2)) > 0.0
        //     ) {
        //         float offset = dot(dir, intersect-eye);
        //         if (offset > 0.001 && (offset < closestOffset || closestType==HIT_TYPE_NO_HIT)) {
        //             closestOffset = offset;
        //             closestType = HIT_TYPE_TRIANGLE;
        //             closestIntersect = intersect;
        //             closestNormal = normal;
        //             closestThing = i;
        //         }
        //         else { // no hit
        //         }
        //     }
        // }
    }
    return r;
}

ret trace_spheres(vec4 eye, vec4 dir) {
    ret r = noHit();

    // find the best ball
    for (int i=0; i<numBalls; ++i) {
        vec4 pos = ball_pos[i];
        float radius = ball_radius[i];

        vec4 projection = (eye-pos) - dot(eye-pos, dir)*dir;
        float minDist = length(projection);
        if (minDist <= radius) {
            vec4 intersect = (pos+projection) - sqrt((radius*radius)-(minDist*minDist)) * dir;
            float t = dot(dir, intersect-eye);
            if (t > 0.001 && (t < r.t || r.hit==HIT_TYPE_NO_HIT)) {
                r.t = t;
                r.hit = HIT_TYPE_SPHERE;
                r.eye = intersect;
                r.thing = i;
                r.normal = normalize(intersect - pos); // we can easily compute it later
            }
        }
    }

    return r;
}

// the returned r.dir is NOT reflected
ret trace(vec4 eye, vec4 dir) {
    dir = normalize(dir);

    ret r = trace_water(eye, dir);/*min_ret(
        trace_spheres(eye, dir),
        trace_triangles(eye, dir)
    );
    r = min_ret(
        r,
        trace_water(eye, dir)
    );*/




    r.dir = dir;
    return r;
}




// ====== main ======

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    ret r = ret(
        vec4(0.0),
        cameraTransform * vec4(0.0, 0.0, 0.0, 1.0),
        cameraTransform * normalize(vec4(v.xy, 1.5, 0.0)), // decrease the y component for more FoV
        0.0,
        HIT_TYPE_INITIAL,
        0
    );


    float shadow = 1.0;
    float bounce;
    for (bounce=0.0; r.hit>HIT_TYPE_NO_HIT && bounce<7.0; bounce+=1.0) {
        r = trace(r.eye, r.dir);
        // return;

        // if (r.thing != 3)
        r.dir = reflect(r.dir, r.normal);
        if (debug) {
            gl_FragColor = vec4((r.normal.xyz + vec3(1.0, 1.0, 1.0))*0.5, 1.0);
            return;
        }

        if (r.hit > HIT_TYPE_NO_HIT) {
            // shadow
            for (int i=0; i<shadowSamples; ++i) {
                if (trace(r.eye, vertex_light_position + 0.09*rand3D()).hit > HIT_TYPE_NO_HIT) {
                    shadow *= shadowPerSample;
                }
            }

            // diffuse only if we don't have a skybox
            if (!skybox_enabled) {
                vec4 diffuse = vec4(1.0, 0.0, 0.0, 0.0) * diffuse(r.normal);
                // diffuse += vec4(0.0, 0.3, 0.0, 0.0); // ambient
                gl_FragColor += diffuse * pow(0.4, bounce+1.0);
            }

        }

    }

    gl_FragColor += specular(r.dir) * pow(0.85, bounce);
    gl_FragColor *= shadow;
    gl_FragColor.a = 1.0;


    // gl_FragColor = vec4(rand3D(), 0.0);
    // gl_FragColor *= 1.5;
    // gl_FragColor *= 0.5;

}
