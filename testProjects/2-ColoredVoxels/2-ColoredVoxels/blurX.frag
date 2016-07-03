#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerColor;
layout (binding = 4) uniform sampler2D samplerResult;
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 outColor;
vec2 invRes = vec2(1.0 / 140.0, 1.0 / 90.0);
mat3 kernel9 = mat3(0.0751, 0.1238, 0.0751, 0.1238, 0.2042, 0.1238, 0.0751, 0.1238, 0.0751);
void main() {
    int i, j;
    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);vec2 nUV = vec2(0.0, 0.0);vec3 vNormal = texture(samplerNormal, uv).rgb;
    for(i=-1;i<=1;++i){for(j=-1;j<=1;++j){nUV = uv + vec2(i, j) * invRes; if(dot(vNormal, texture(samplerNormal, nUV).rgb) > 0.9) sum += (texture(samplerResult,nUV)*kernel9[i+1][j+1]);}}
    outColor = texture(samplerColor, uv) * sum;
    //outColor = sum;
    //outColor = texture(samplerResult, uv);
}