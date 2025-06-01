//post_pro.vs
#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;

// Uniform variables
uniform mat4 mvp;

void main()
{
    // Pass texture coordinates to fragment shader
    fragTexCoord = vertexTexCoord;

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

