#version 440

layout(binding = 0) buffer dcA1 { float A1 [ ]; };
layout(binding = 1) buffer dcA2 { float A2 [ ]; };

layout(rgba8, binding = 2) uniform writeonly image2D img;
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform ivec2 resolution;
uniform sampler2DRect seedSource;

uniform float seed;
uniform float decay;
uniform float reaction;
uniform float elapsedTime;
uniform int react;


int per(int x, int nx){
    if (x < 0) x += nx;
    if (x >= nx) x -= nx;
    return x;
}

float rand(vec2 n){ 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main(){
    int i, j;

    i = int(gl_GlobalInvocationID.x);
    j = int(gl_GlobalInvocationID.y);

    int W = resolution.x;
    int H = resolution.y;

    int idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8;
    int ir, jd, il, ju;
    ir = clamp(i+1, 0, W);
    il = clamp(i-1, 0, W);
    jd = clamp(j+1, 0, H);
    ju = clamp(j-1, 0, H);

    idx0 = i + W*j;
    idx1 = i + W*jd;
    idx2 = i + W*ju;

    idx3 = ir + W*j;
    idx4 = ir + W*jd;
    idx5 = ir + W*ju;

    idx6 = il + W*j;
    idx7 = il + W*jd;
    idx8 = il + W*ju;

    //float T = elapsedTime / 60.0;
    float tr = rand(gl_GlobalInvocationID.xy + vec2(elapsedTime*1.0, elapsedTime*.2));

    float r = 0.0;
    r += A1[idx1];
    r += A1[idx2];
    r += A1[idx3];
    r += A1[idx4];
    r += A1[idx5];
    r += A1[idx6];
    r += A1[idx7];
    r += A1[idx8];

    float val = A1[idx0];
    vec2 texCoord = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    float s = texture(seedSource, texCoord).r;
    val += s * seed;

    if (react > 0 && r > 0.9 && tr > reaction && val < 0.001){
    	val = 1.0;
    }

    A2[idx0] = val * decay;

    // render reaction
    //vec4 col = vec4(texCoord.x / W, texCoord.y / H, 0.0, 1.0);
    vec4 col = vec4(val, s, 0.0, 1.0);
    imageStore(img, ivec2(i, j), col);
}
