#version 450 core
layout(location = 0) in vec2 position;
//layout(location = 1) in vec3 color;
layout(std140, row_major, binding = 0)uniform OffsetBuffer
{
	vec2 offsets[6];
	uint inversed;
};
//flat out vec4 in_color;
void main()
{
	unsigned int id = gl_VertexID / 6;
	vec2 temp = position + offsets[id];
	if (inversed == 0)gl_Position = vec4(temp, 0, 1);
	else gl_Position = vec4(-temp, 0, 1);
	//in_color = vec4(color, 1);
}