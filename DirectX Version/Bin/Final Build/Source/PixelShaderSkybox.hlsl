TextureCube skyMap   : register(t0); SamplerState Linear : register(s0); 

float4 main(in float4 Position : SV_Position, in float3 TexCoord : TEXCOORD0) : SV_Target
{   
	// map the skybox to the coordinantes 
	return skyMap.Sample(Linear,TexCoord); 
}