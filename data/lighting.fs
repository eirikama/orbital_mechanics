#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragWorldPos;

uniform sampler2D texture0;
uniform vec3 sunPos;

out vec4 finalColor;

void main()
{
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(sunPos - fragWorldPos);

    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);

    // Soft ambient so dark side isn't pure black
    float ambient = 0.15;

    float lighting = ambient + diff;

    vec4 texColor = texture(texture0, fragTexCoord);

    finalColor = vec4(texColor.rgb * lighting, texColor.a);
}
