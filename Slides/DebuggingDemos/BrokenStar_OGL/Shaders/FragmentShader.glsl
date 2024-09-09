// an ultra simple glsl fragment shader

#version 330 // GLSL 3.30

out vec4 Pixel;
in vec3 out_color;

void main() 
{	
	Pixel = vec3(out_color, 1);
}