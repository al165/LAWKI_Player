#version 330

//in vec2 TexCoord;
in vec2 texCoordVarying;

out vec4 FragColor;
//layout(location = 0) out vec4 FragColor;

#define ROOTTWO 1.414

uniform vec2 resolution;

uniform sampler2DRect video_src;
uniform sampler2DRect prev_frame;

uniform float time;
//uniform float seed;
uniform float decay;
uniform float reaction;
//uniform int highlight;
uniform float threshold;

//uniform vec3 clusterCenters[18];
uniform vec3 center;


float rand(vec2 n) {
  return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}


float when_gt(float x, float y) {
    return max(sign(x - y), 0.0);
}

float when_lt(float x, float y) {
    return max(sign(y - x), 0.0);
}


float distSquared(vec3 A, vec3 B) {
    vec3 C = A - B;
    return dot( C, C );
}


float getSource(vec2 uv) {
    float g = texture(video_src, uv).g;
    float x = when_gt(g, threshold);

    return x; //vec4(x, 0, 0, x);
}


//float getSource2(vec2 uv){
//    // returns the index of the closest color
//    vec3 c = texture(video_src, uv).rgb;
//
//    float minDist = 100000.0;
//    int argMin = 0;
//    int i;
//    for (i = 0; i < 6; i++){
//        float distance = distSquared(c, clusterCenters[i]);
//        if(distance < minDist){
//            minDist = distance;
//            argMin = i;
//        }
//        //minDist = min(distance, minDist);
//    }
//
//    if (argMin == highlight){
//        return 1.0;
//    } else {
//        return 0.0;
//    }
//}

float getSource3(vec2 uv){
    vec3 c = texture(video_src, uv).rgb;
    float distance = distSquared(c, center);

    float x = when_lt(distance, threshold);
    return x;
}

void main(){
    vec2 res = 1.0 / resolution;
    //vec2 uv = gl_FragCoord.xy * res;
    vec2 uv = texCoordVarying;

    vec4 color = vec4(0, 0, 0, 1.0); //texture(prev_frame, texCoordVarying);

    float r = 0.0;
    r += texture(prev_frame, uv + vec2(-ROOTTWO,-ROOTTWO)*res).r;
    r += texture(prev_frame, uv + vec2( 0,-1)*res).r;
    r += texture(prev_frame, uv + vec2( ROOTTWO,-ROOTTWO)*res).r;
    r += texture(prev_frame, uv + vec2( 1, 0)*res).r;
    r += texture(prev_frame, uv + vec2( ROOTTWO, ROOTTWO)*res).r;
    r += texture(prev_frame, uv + vec2( 0, 1)*res).r;
    r += texture(prev_frame, uv + vec2(-ROOTTWO, ROOTTWO)*res).r;
    r += texture(prev_frame, uv + vec2(-1, 0)*res).r;

    float tr = rand(uv + vec2(time*3, time*0.2));

    //float cr = color.r;
    //color.g = highlight/255.0;

    color.r *= decay;
    color.r += getSource3(uv);

    // color.r += source*seed

    float T = 0.5;

    if (r > 0.5 + 0.7*T && tr > 0.5 + 0.45 * -cos(uv.y*3.0+ time*0.5) && color.r < reaction/200){
        color.r = 1.0;
    }

    // test texture coordinates...
    //r = texCoordVarying.x / resolution.x;
    //float g = texCoordVarying.y / resolution.y;
    //color = vec4(r, g, 1.0, 1.0);

    FragColor = color;
}
