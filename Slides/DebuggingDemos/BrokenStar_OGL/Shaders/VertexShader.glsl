// an ultra simple glsl vertex shader

#version 330 // GLSL 3.30

layout(location = 1) in vec2 local_pos;
layout(location = 1) in vec3 in_color;
out vec3 out_color;

void main()
{
	out_color = in_color;
	gl_Position = vec4(local_pos,0,0,1);
}