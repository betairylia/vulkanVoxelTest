#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 2, binding = 0) uniform sampler2D samplerPosition;
layout (set = 2, binding = 1) uniform sampler2D samplerNormal;
layout (set = 2, binding = 2) uniform sampler2D samplerColor;
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 outColor;

int sampleCount = 8;
int selAngle[8] = int[](2, 4, 3, 7, 6, 5, 0, 1);
int selDist[8]  = int[](3, 2, 4, 7, 1, 6, 5, 0);
float fSelAngle = 6.2831853 / sampleCount;
float fSelDist  = 1.0 / sampleCount;
float fScaler = 1.0;
float fRadius = 3.0;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 vSampleVector;
    vec3 vPos = texture(samplerPosition, uv).xyz;
    vec3 vNormal = texture(samplerNormal, uv).xyz;
    float occ = 0.0;

    float diffuse = 1.0f;

    for(int i = 0; i < sampleCount; ++i)
    {
        int n = selAngle[i];
        int m = selDist[i];
        vSampleVector = vec2(cos(fSelAngle * n), sin(fSelAngle * n)) * (fSelDist * (m + 1)) * fScaler / vPos.z;
        float rAngle = 3.14159265 * rand(vPos.yx);
        vSampleVector = mat2(cos(rAngle), -sin(rAngle), sin(rAngle), cos(rAngle)) * vSampleVector;
        vec3 sPos = texture(samplerPosition, uv + vSampleVector).rgb;
        vec3 sdPos = sPos - vPos;
        float fAbsV = dot(sdPos, sdPos);
        float fRes = 0;
        fRes = max(0.0, 1.0 - sqrt(fAbsV) / fRadius) * max(0.0, dot(sdPos / sqrt(fAbsV), vNormal) - 0.01);
        occ += fScaler * fRes;
    }
    occ = 1-(occ / sampleCount);
    /*outColor = vec4(texture(samplerColor, uv).rgb * diffuse * occ, 1.0);*/
    outColor = vec4(occ, occ, occ, 1.0);
}
