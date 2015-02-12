
float4x4 Proj, View;

void main(in  float4 Position  : POSITION, in  float4 Color : COLOR0, in  float2 TexCoord : TEXCOORD0,
	out float4 oPosition : SV_Position, out float4 oColor : COLOR0, out float3 oTexCoord : TEXCOORD0)
{
	oPosition = mul(Proj, mul(View, Position)); 
	oTexCoord = Position; 
	oColor = Color;
}