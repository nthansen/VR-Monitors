
float4x4 Proj, View;

void main(in  float4 Position  : POSITION, in  float4 Color : COLOR0, in  float2 TexCoord : TEXCOORD0,
	out float4 oPosition : SV_Position, out float4 oColor : COLOR0, out float2 oTexCoord : TEXCOORD0)
{
    //float4 tmp = mul(-1, Position);
   // float4 shift
    oTexCoord = mul(-0.5f , Position);
	oPosition = mul(Proj, mul(View, Position));
	oColor = Color;
}