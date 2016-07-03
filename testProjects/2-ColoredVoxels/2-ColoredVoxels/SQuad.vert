#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform bufferVals {
    mat4 model;
    mat4 view;
    mat4 proj;
} myBufferVals;
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 0) out vec2 outUV;
out gl_PerVertex {
    vec4 gl_Position;
};
void main() {
   outUV = vec2(uv.x, 1.0 - uv.y);
   gl_Position = vec4(pos, 1.0);
}
