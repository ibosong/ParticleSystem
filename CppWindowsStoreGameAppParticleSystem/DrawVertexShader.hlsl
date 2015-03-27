//
// This shader update the particles' position.
//
struct VSOutput
{
	float3 pos : POSITION;
	float4 color : COLOR;
	uint type : TYPE;
};
struct Particle
{
	float3 pos : POSITION;	
	float3 color : COLOR;
	float3 vel : VELOCITY;
	float age : AGE;
	uint type : TYPE;
};
#define gAccel float3(2.0f, 2.0f, 2.0f)
VSOutput main( Particle input) 
{
	float t = input.age;
	VSOutput output;
	if(input.type == 1)
	{
		output.pos = 0.5f * t * t * float3(0.0f, -0.3f, 0.0f) + t * input.vel + input.pos;
	}
	else //Emitters
	{
		output.pos = input.pos;
	}
	
	output.color = float4(input.color, 1.0f);
	output.type = input.type;
	return output;
}