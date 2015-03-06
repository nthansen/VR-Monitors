
float4x4 Proj, View;

void main(in  float4 Position  : POSITION, in  float4 Color : COLOR0, in  float2 TexCoord : TEXCOORD0,
	out float4 oPosition : SV_Position, out float3 oTexCoord : TEXCOORD0)
{
	// update position so now it's a result of the multiplication of the position of the skybox, 
	// the view and projection so we know where it is in relation to the camera
	oPosition = mul(Proj, mul(View, Position)); 

	// make the texture coordinates the position of the skybox so it'll map to each spot on
	// the cube
	oTexCoord = Position; 
}