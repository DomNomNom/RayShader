
varying vec3 vertex_light_position;
varying vec3 vertex_normal;
varying vec3 v;
varying vec4 position;

uniform vec2 mouse;
uniform float time;
uniform bool drift;
uniform vec4 balls[20];
uniform int numballs;

vec4 specular(vec3 dir) {
    return clamp((
        1.0 *
        pow(max(dot(dir, vertex_light_position), 0.5), 50.0) *
        vec4(1.0, 1.0, 1.0, 0.0)
    ), 0.0, 1.0);
}

vec4 phong(vec3 view, vec3 normal) {

    // gl_LightSourceParameters light = gl_LightSource[0];
    // vec4 diffuse = gl_FrontMaterial.diffuse * diffuse_value;

    vec3 L = normalize(gl_LightSource[0].position.xyz); // -v.xyz
    vec3 E = normalize(view).xyz; // we are in Eye Coordinates, so EyePos is (0,0,0)
    vec3 R = normalize(reflect(L,normal));

    // Ambient:
    float Iamb = 0.05;

    // Diffuse:
    float Idiff = dot(normal,L);

    // Specular:
    float Ispec = 1.0 * pow(max(dot(R,E),0.5),50.0); //gl_FrontMaterial.shininess
    // if (Idiff < 0.0) Ispec = 0.0; // don't let specular light through to the other side

    Iamb  = clamp(Iamb,  0.0, 1.0);
    Idiff = clamp(Idiff, 0.0, 1.0);
    Ispec = clamp(Ispec, 0.0, 1.0);

    return clamp(
        Iamb    * vec4(1.0, 0.0, 0.0, 0.0) +
        Idiff   * vec4(0.8, 0.0, 0.0, 0.0) +
        Ispec   * vec4(1.0, 1.0, 1.0, 0.0)

        , 0.0, 1.0
    );
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
        vec4 ball = balls[i];
        vec3 projection = (eye-ball.xyz) - dot(eye-ball.xyz, dir)*dir;
        float minDist = length(projection);
        if (minDist <= ball.w) {
            vec3 sphereIntersect = (ball.xyz+projection) - sqrt((ball.w*ball.w)-(minDist*minDist)) * dir;
            float offset = dot(eye-sphereIntersect, dir);
            if (offset< 0.0 && (bestBall < 0 || offset > bestOffset)) {
                bestBall = i;
                bestOffset = offset;
                bestSphereIntersect = sphereIntersect;
            }
        }
    }


    if (bestBall >= 0) {// && abs(eye.z+1.0) < 0.001) {
        vec3 normal = normalize(bestSphereIntersect - balls[bestBall].xyz);
        vec3 reflected = normalize(reflect(dir,normal));
        return ret(
            vec4(0.5, 0.0, 0.0, 0.0) * clamp(dot(normal, vertex_light_position), 0.0, 1.0)
            // + specular(reflected)
            ,

            bestSphereIntersect,
            reflected,
            true
        );

        // return phong(dir, normal);


        // vec4 ball = balls[bestBall];
        // float offset = dot(eye-ball.xyz, dir);
        // vec3 projection = (eye-ball.xyz) - offset*dir;
        // // projection = eye-ball.xyz;
        // // projection.z=0.0;
        // float minDist = length(projection);
        // if (minDist <= ball.w) {
        //     vec3 sphereIntersect = (ball.xyz+projection) - sqrt((ball.w*ball.w)-(minDist*minDist)) * dir;
        //     vec3 normal = normalize(sphereIntersect - ball.xyz);
        //     return phong(dir, normal);
        //     // return vec4(
        //     //     0,
        //     //     0,
        //     //     1.0-length(projection)/ball.w, // (sqrt((ball.w*ball.w)/(minDist*minDist)) * 0.2 * dir).z,
        //     //     1.0
        //     // );
        //     // return vec4(distance(sphereIntersect, eye)*0.5, 0.5, 0.5, 0.0);
        //     // return vec4(minDist/ball.w, 0.5, 0.5, 0.0);
        // }

        // // return vec4(0.5, 0.5, 0.5, 0.0);
    }

    return ret(
        specular(dir),
        // vec4(0.0, 0.0, 0.0, 0.0),
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