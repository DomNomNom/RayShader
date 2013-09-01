
varying vec3 vertex_light_position;
varying vec3 vertex_normal;
varying vec3 v;
varying vec4 position;

uniform vec2 mouse;
uniform float time;
uniform vec4 ball_pos[20];  // positions
uniform float ball_radius[20]; // radii
uniform int numballs;

vec4 specular(vec3 dir) {
    return clamp((
        1.0 *
        pow(max(dot(dir, vertex_light_position), 0.5), 50.0) *
        vec4(1.0, 1.0, 1.0, 0.0)
    ), 0.0, 1.0);
}


struct ret {
    vec4 colour;
    vec3 eye; // for re-tracing
    vec3 dir;
    bool hit;
};

ret trace(vec3 eye, vec3 dir) {
    int bestBall = -1;
    float bestOffset = 0.0;
    vec3 bestSphereIntersect = vec3(0,0,0);

    // find the best ball
    for (int i=0; i<numballs; ++i) {
        vec4 pos = ball_pos[i];
        float radius = ball_radius[i];

        vec3 projection = (eye-pos.xyz) - dot(eye-pos.xyz, dir)*dir;
        float minDist = length(projection);
        if (minDist <= radius) {
            vec3 sphereIntersect = (pos.xyz+projection) - sqrt((radius*radius)-(minDist*minDist)) * dir;
            float offset = dot(eye-sphereIntersect, dir);
            if (offset< 0.0 && (bestBall < 0 || offset > bestOffset)) {
                bestBall = i;
                bestOffset = offset;
                bestSphereIntersect = sphereIntersect;
            }
        }
    }


    if (bestBall >= 0) {// && abs(eye.z+1.0) < 0.001) {
        vec3 normal = normalize(bestSphereIntersect - ball_pos[bestBall].xyz);
        vec3 reflected = normalize(reflect(dir,normal));
        return ret(
            vec4(0.5, 0.0, 0.0, 0.0) * clamp(dot(normal, vertex_light_position), 0.0, 1.0),

            bestSphereIntersect,
            reflected,
            true
        );

    }

    return ret(
        specular(dir),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        false
    );
}

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    ret r = ret(gl_FragColor, vec3(v.xy, -1.0), vec3(0.0, 0.0, 1.0), true);

    float bounce = 0.0; // actually a int
    while (r.hit && bounce < 30.0) {
        bounce += 1.0;
        r = trace(r.eye, r.dir);

        vec4 result = r.colour;
        if (r.hit) {
            result += vec4(0.2, 0.0, 0.0, 0.0); // ambient
            if (! trace(r.eye, vertex_light_position).hit) // shadow
                result += r.colour; // diffuse
             result *= pow(0.50, bounce);
        }
        else {
            result += r.colour; // specular
        }
        gl_FragColor += result;// * pow(0.75, bounce);

    }

    // gl_FragColor = clamp(gl_FragColor, 0.0, 1.0);
    // for (int i=0; i<numballs; ++i) {
    //     vec4 ball = balls[i];
    //     if (distance(v.xy, ball.xy) < ball.w)
    //         gl_FragColor = balls[1];
    // }


}