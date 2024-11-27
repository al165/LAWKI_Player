#version 150

in vec2 texCoordVarying;

out vec4 FragColor;

uniform sampler2DRect video;
uniform float threshold;
uniform vec3 center;

float when_lt(float x, float y){
    return max(sign(y - x), 0.0);
}

float distSquared(vec3 a, vec3 b){
    vec3 c = a - b;
    return dot(c, c);
}

float getSource(vec2 uv){
    vec3 c = texture(video, uv).rgb;
    float distance = distSquared(c, center);
    float x = when_lt(distance, threshold);
    return x;
}

void main(){
    float val = getSource(texCoordVarying);
    FragColor = vec4(val, 0.0, 0.0, 1.0);
    //FragColor = vec4(texCoordVarying.x, texCoordVarying.y, 0.0, 1.0);
}

