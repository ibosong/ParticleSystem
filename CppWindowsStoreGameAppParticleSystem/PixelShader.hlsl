//
// Pixel shader set texture to the particles.
//
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
	float4 color : COLOR;
	uint type : TYPE;	
	
};

Texture2D gTexture;
SamplerState samLinear;
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 color = gTexture.Sample(samLinear, input.tex);
	if(input.type == 0)//Emitters
	{
		return input.color;
	}
	else 
	{
		return color * input.color;
	}
}
