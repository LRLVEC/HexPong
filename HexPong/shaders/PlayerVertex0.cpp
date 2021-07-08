#version 450 core
layout(location = 0) in vec2 position;
//layout(location = 1) in vec3 color;
layout(std140, row_major, binding = 1)uniform OffsetBuffer
{
	vec2 offsets[6];
};
//flat out vec4 in_color;
void main()
{
	unsigned int id = gl_VertexID / 6;
	gl_Position = vec4(position + offsets[id], 0, 1);
	//in_color = vec4(color, 1);
}