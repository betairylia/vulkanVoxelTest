#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform bufferVals
{
    mat4 model;
    mat4 view;
    mat4 proj;
} myBufferVals;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in uint meta;
layout (location = 0) out vec3 worldPosition;
layout (location = 1) out float depth;
layout (location = 2) out vec3 worldNormal;
layout (location = 3) out vec4 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    worldPosition = (myBufferVals.model * vec4(pos, 1.0)).xyz;
<<<<<<< HEAD
    worldNormal = transpose(inverse(mat3(myBufferVals.model))) * normalize(normal);
    vec4 outPos = myBufferVals.proj * myBufferVals.view * myBufferVals.model * vec4(pos, 1.0);
    outColor = vec4(0.3f, 0.5f, 1.0f, 1.0f);
=======
	worldNormal = transpose(inverse(mat3(myBufferVals.model))) * normalize(normal);
    vec4 outPos = myBufferVals.proj * myBufferVals.view * myBufferVals.model * vec4(pos, 1.0);
    outColor = vec4(0.2, 0.3, 0, 1);
>>>>>>> 5a2f215fb705280660f2a78b7c5e2bd3faf6d85d
    depth = outPos.z / outPos.w;
    gl_Position = outPos;
}
