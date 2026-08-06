#version 330

// Vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

// Uniforms provided by raylib
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Outputs to fragment shader
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragWorldPos;

void main()
{
    fragTexCoord = vertexTexCoord;

    // Convert normal to world space correctly
    fragNormal = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);

    // Compute world position
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
