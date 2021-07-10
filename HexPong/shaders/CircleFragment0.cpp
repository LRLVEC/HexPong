#version 450 core
//flat in vec4 in_color;
out vec4 o_color;
void main()
{
	vec2 temp = gl_PointCoord - vec2(0.5);
	float t = dot(temp, temp);
	if (t > 0.25)discard;
	vec4 color1 = vec4(0, 0, 1, 1);
	vec4 color2 = vec4(0, 0, 0, 0);
	o_color = mix(color2, color1, smoothstep(0., 0.25, t));
}