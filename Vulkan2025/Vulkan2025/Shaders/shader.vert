#version 450 		// Use GLSL 4.5 

layout(location = 0) out vec3 fragColour;	// Output colour for vertex (location is required)

// Triangle vertes possitions (will put in to vertes buffer later)
// Screen coordinates is  btween 1 and -1
vec3 positions[3] = vec3[](
	vec3(0.0, -0.4, 0.0),
	vec3(0.4, 0.4, 0.0),
	vec3(-0.4, 0.4, 0.0)
);

// Triangle vertex colours
vec3 colours[3] = vec3[](
	vec3(1.0, 0.0, 0.0), 	// RED colour
	vec3(0.0, 1.0, 0.0), 	// GREEN colour
	vec3(0.0, 0.0, 1.0)		// BLUE colour
);

void main()	{
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	fragColour = colours[gl_VertexIndex];
}