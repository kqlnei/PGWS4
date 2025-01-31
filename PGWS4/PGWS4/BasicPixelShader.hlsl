#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    //float3 light = normalize(float3(1, -1, 1));
    
    // ライトを回転させる
    float angle = time_mat + 6.28f; // 時間に応じた回転角度
    float3 light = normalize(float3(cos(angle), -1.0f, sin(angle)));
//    float3 light = normalize(float3(1, -1, 1));

    
    float brightness = max(dot(-light, normalize(input.normal.xyz)), 0) * 0.9 + 0.1;
    return float4(brightness, brightness, brightness, 1);
}