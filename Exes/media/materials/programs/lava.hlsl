float4x4 ViewProjectionMatrix;
//float4x4 worldMatrix;

float tiempo;

struct VsInput
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
};

struct VsOutput
{
	float2 uv0 : TEXCOORD0;
	float4 position : POSITION;
};

VsOutput vsMain(const VsInput In) {

	VsOutput vsOut;
	float4 pos = In.position;
	pos.y += (sin(pos.x+tiempo*2) + sin(pos.z + tiempo*2))*0.15;
	vsOut.position = mul(ViewProjectionMatrix, pos);
	vsOut.uv0 = In.uv0;
	return vsOut;
}


float4 mainPS() : COLOR 
{
	return float4(0, 0.0, 0.0, 0.0);
}