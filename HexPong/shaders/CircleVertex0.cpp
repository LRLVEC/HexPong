#version 450 core
layout(location = 0) in vec2 position;
layout(std140, row_major, binding = 0)uniform OffsetBuffer
{
	vec2 offsets[6];
	uint inversed;
};
void main()
{
	if (inversed == 0)gl_Position = vec4(position, 0, 1);
	else gl_Position = vec4(-position, 0, 1);
}