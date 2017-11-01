#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 worldPosition;
layout (location = 1) in float depth;
layout (location = 2) in vec3 worldNormal;
layout (location = 3) in vec4 color;
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outColor2;
layout (location = 2) out vec4 outColor3;

void main()
{
    outColor = vec4(worldPosition, depth);
    outColor2 = vec4(worldNormal, 1.0);

    vec3 lightDircInv = normalize(vec3(0.3f, 1.0f, 0.8f));
    vec4 viewPos = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    //basic lighting
    outColor3 = 0.7f * color + 0.3f * dot(worldNormal, lightDircInv) * color;
}
