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

   float diffuse = 0.4 + 0.3 * dot(worldNormal, normalize(vec3(1.0f, 2.0f, -0.8f)));
   outColor3 = color * diffuse;
}
