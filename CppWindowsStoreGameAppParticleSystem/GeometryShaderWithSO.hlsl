//
// Geometry shader with stream-out. This shader produces particles with random position and color, 
// and output (or not) the existing particles based on their ages.
//
cbuffer cbPerFrame
{
	float gTime;
	float gTimeStep;
};

Texture1D gRandomTex;
SamplerState samLinear;
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gTime + offset);
	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	// project onto unit sphere
	return normalize(v);
}
struct Particle
{
	float3 pos : POSITION;	
	float3 color : COLOR;
	float3 vel : VELOCITY;
	float age : AGE;
	uint type : TYPE;
};

#define PT_EMITTER 0
#define PT_PARTICLE 1

[maxvertexcount(2)]
void main(
	point Particle input[1], 
	inout PointStream<Particle> output
)
{
	input[0].age += gTimeStep;	
	
	if(input[0].type == 0)
	{
		if(input[0].age > 0.05f)
		{
			Particle p;
			float3 vRandom = RandUnitVec3(1.0f);
			p.pos = input[0].pos;
			float3 randColor = RandUnitVec3(0.0f);			
			if(randColor.x < 0.0000f) randColor.x *= -1;
			if(randColor.y < 0.0000f) randColor.y *= -1;
			if(randColor.z < 0.0000f) randColor.z *= -1;
			p.color = randColor;
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			p.vel = vRandom / 2;
			p.age = 0.0f;
			p.type = PT_PARTICLE;
			output.Append(p);
			input[0].age = 0.0f;
		}
		// Keep emitters
		output.Append(input[0]);
	}
	else if (input[0].type == PT_PARTICLE)
	{
		if(input[0].age < 3.0f)
		{
			output.Append(input[0]);
		}
	}
	
}