//
// This nomal geometry shader just expand a vertex to a quad, so that we can map texture on it.
// We also set the quad's facing direction based on gLook global variable.
//
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 gLook;
};

struct GSInput
{
	float3 pos : POSITION;
	float4 color : COLOR;
	uint type : TYPE;
};


struct GSOutput
{
	//uint primID : SV_PrimitiveID;
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
	float4 color : COLOR;
	uint type : TYPE;	
	
};
cbuffer cbFixed
{
	static const float2 gTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};
};

[maxvertexcount(4)]
void main(
	point GSInput input[1], 
	//uint primID : SV_PrimitiveID,
	inout TriangleStream< GSOutput > output
)
{
	if(input[0].type != 0)
	{
		float3 up = float3(0.0f, 1.0f, 0.0f);
		float3 look = gLook.xyz;
		look.y = 0.0f;
		float3 right = cross(up, look);

		float halfWidth = 0.1f;
		float halfHeight = 0.1f;
		float4 v[4];
		v[0] = float4(input[0].pos + halfWidth * right - halfHeight * up, 1.0f);
		v[1] = float4(input[0].pos + halfWidth * right + halfHeight * up, 1.0f);
		v[2] = float4(input[0].pos - halfWidth * right - halfHeight * up, 1.0f);
		v[3] = float4(input[0].pos - halfWidth * right + halfHeight * up, 1.0f);

		GSOutput gout;
		[unroll]
		for (int i = 0; i < 4; ++i)
		{
			//gout.primID = primID;
			gout.pos = mul(v[i], model);
			gout.pos = mul(gout.pos, view);
			gout.pos = mul(gout.pos, projection);
			gout.tex = gTexC[i];
			gout.color = input[0].color;
			gout.type = input[0].type;
			
			output.Append(gout);
		}
		
	}
}