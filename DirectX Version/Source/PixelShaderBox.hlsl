
Texture2D Texture   : register(t0); SamplerState Linear : register(s0); 

float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0, in float2 TexCoord : TEXCOORD0) : SV_Target
{ 
	return Texture.Sample(Linear, TexCoord); 
}