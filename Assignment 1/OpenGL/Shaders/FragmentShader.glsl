#version 330 // GLSL 3.30
out vec4 Pixel;
// an ultra simple glsl fragment shader
void main() 
{	
	Pixel = vec4(170/255.0f, 100/255.0f, 44/255.0f, 1); // TODO: Part 1a
}