/****************************** Module Header ******************************\
*
* This sample shows how to implement particles system in Windows Store DirectX 
* game app. For the purpose of efficiency, this sample uses Billboard technique
* to implement particles. With this technique, we can input points, and then 
* expand them to quads in geometry shader. To produce and update particles, we 
* need to create another geometry shader with Stream-Out for outputting produced
* points. Since Geometry Shader was first introduced in Direct3D 10, if your video
* device doesn't support D3D_FEATURE_LEVEL_10_0 or higher, the WRAP device will be
* used instead.
*
\***************************************************************************/
#pragma once

#include "DirectXBase.h"

// Constant buffer
struct cbPerObject
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4 gLook;
};
// Constant buffer
struct cbPerFrame
{
	FLOAT gTime;
	FLOAT gTimeStep;
	FLOAT pad1;
	FLOAT pad2;

};
// Vertex buffer struct
struct EmitterParticleVertex
{
	DirectX::XMFLOAT3 pos;	
	DirectX::XMFLOAT3 color;
	DirectX::XMFLOAT3 vel;
	FLOAT age;	
	UINT type;
};

//
// This class implement particles rendering.
//
ref class ParticleRenderer sealed : public DirectXBase
{
public:
	ParticleRenderer();
	// DirectXBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Render(float* backgroundColor) override;
	
	// Method for updating time-dependent objects.
	void Update(float timeTotal, float timeDelta);
	
internal:
	//Check if the device support Geometry Shader.
	bool CheckSupportGS();
	// Rotate the view space with coordinte offset.
	void ParticleRenderer::RotateWithMouse(
		float x1,	// Start x-coordinate
		float y1,	// Start y-coordinate		
		float x2,	// End x-coordinate
		float y2	// End y-coordinate
		);

private:
	bool m_loadingComplete;
	bool m_isFirstDraw;
	
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_drawInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_initVB;									// Initial vertex buffer for emitter point in particles.
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_drawVB;									// Vertex buffer for drawing particles.
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_streamOutVB;								// Vertex buffer from geometry shader with stream-out.
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;								
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_drawVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShaderWithSO;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_drawGeometryShader;
	Microsoft::WRL::ComPtr<ID3D11BlendState1> m_blendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_rsState;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_textureView;				// Particle texture view
	Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_sampler;                  // Particle texture sampler state
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_randomTexSRV;				// Random texture view
	Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_randomTexSampler;         // Random texture sampler state for producing random number

	uint32 m_indexCount;
	
	cbPerObject m_cbPerObject;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantPerObjectBuff;
	cbPerFrame m_cbPerFrame;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantPerFrameBuff;


};