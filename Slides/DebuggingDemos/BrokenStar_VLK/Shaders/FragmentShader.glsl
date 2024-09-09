#version 430 // GLSL 4.30
layout(location = 0) out vec4 Pixel;
layout(location = 1) in vec2 out_color;
// an ultra simple glsl fragment shader
void main() 
{	
	Pixel = vec4(out_color, 1);
}