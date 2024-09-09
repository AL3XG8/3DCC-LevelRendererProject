#version 430 // GLSL 4.30
// an ultra simple glsl vertex shader
in vec2 local_pos;
layout(location = 1) in vec2 in_color;
layout(location = 0) out vec3 out_color;
void main()
{
	out_color = in_color;
	gl_Position = vec4(local_pos, 1);
}