#version 150

in vec2 texCoordVarying;

out vec4 FragColor;

uniform sampler2DRect video;
uniform sampler2DRect reaction;
uniform ivec2 videoResolution;
uniform ivec2 reactionResolution;
uniform ivec3 color;

void main()
{
    vec4 color1 = texture(video, texCoordVarying); // * videoResolution);
    vec4 color2 = texture(reaction, (texCoordVarying / videoResolution) * reactionResolution);
    vec4 hi = vec4(color.r/255.0, color.g/255.0, color.b/255.0, 1.0);

    //FragColor = color2;
    FragColor = mix(color1, hi, color2.r);
    //FragColor = mix(color1, color2, 0.5);
}
