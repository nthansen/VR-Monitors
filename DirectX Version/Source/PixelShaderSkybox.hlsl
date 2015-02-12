TextureCube skyMap   : register(t0); SamplerState Linear : register(s0); 

float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0, in float3 TexCoord : TEXCOORD0) : SV_Target
{   
	return skyMap.Sample(Linear,TexCoord); 
}