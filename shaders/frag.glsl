
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


// re-tracing information
struct ret {
    vec4 colour;
    vec3 eye;
    vec3 dir;
    bool hit;
};

int HIT_TYPE_NO_HIT   = 0;
int HIT_TYPE_SPHERE   = 1;
int HIT_TYPE_TRIANGLE = 2;


// gets called if a ray does not hit any objects
vec4 specular(vec3 dir) {
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

            vec3 intersect = triangles[i].xyz + eye - t * dir; // the intersect point


            float len_ab = length(triangles[i+1]);
            float len_ac = length(triangles[i+2]);
            float v = dot(intersect, triangles[i+2].xyz) / (len_ab * len_ab);
            float u = dot(intersect, triangles[i+1].xyz) / (len_ac * len_ac);
            float uv = u + v;

            if (
                0.0 <= u  && u  <= 1.0 &&
                0.0 <= v  && v  <= 1.0 &&
                0.0 <= uv && uv <= 1.0
            ) {
                float offset = dot(dir, intersect-eye);
                if (offset > 0.0 && (offset < closestOffset || closestType==HIT_TYPE_NO_HIT)) {
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
            if (offset > 0.0 && (offset < closestOffset || closestType==HIT_TYPE_NO_HIT)) {
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

            vec3(0.0, 0.0, 0.0),
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


    for (float bounce = 1.0; r.hit && bounce<10.0; bounce+=1.0) {
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