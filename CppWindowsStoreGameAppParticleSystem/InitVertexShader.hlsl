//
// Initial particles.
//
struct Particle
{
	float3 pos : POSITION;	
	float3 color : COLOR;
	float3 vel : VELOCITY;
	float age : AGE;
	uint type : TYPE;
};

Particle main(Particle input)
{	
	return input;
}
