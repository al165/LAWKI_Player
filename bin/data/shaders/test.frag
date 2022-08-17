#version 150

//uniform sampler2DRect video;
//in vec2 texCoordVarying;
out vec4 outputColor;

void main()
{
	//vec4 color = texture(video, texCoordVarying);

	outputColor = vec4(1.0, 0.5, 1.0, 1.0); //color; //vec4(color.r, color.g, color.b, 1.0);

        //float windowWidth = 1024.0;
        //float windowHeight = 768.0;
    
	//float r = gl_FragCoord.x / windowWidth;
	//float g = gl_FragCoord.y / windowHeight;
	//float b = 1.0;
	//float a = 1.0;
	//outputColor = vec4(r, g, b, a);
}
