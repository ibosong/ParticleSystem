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
#include "pch.h"
#include "ParticleRenderer.h"
#include "DDSTextureLoader.h"
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

ParticleRenderer::ParticleRenderer() :
	m_loadingComplete(false),
	m_isFirstDraw(true),
	m_indexCount(0)
{
}

bool ParticleRenderer::CheckSupportGS()
{
	return m_isSupportGS;
}

void ParticleRenderer::CreateDeviceResources()
{
	DirectXBase::CreateDeviceResources();
	//
	// Create 1D random texture
	//
	XMFLOAT4 randomValues[512];
	for (int i = 0; i < 512; ++i)
	{
		randomValues[i].x = DX::RandF(-1.0f, 1.0f);
		randomValues[i].y = DX::RandF(-1.0f, 1.0f);
		randomValues[i].z = DX::RandF(-1.0f, 1.0f);
		randomValues[i].w = DX::RandF(-1.0f, 1.0f);
	}
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 512 * sizeof(XMFLOAT4);
	initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 512;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;
	ComPtr<ID3D11Texture1D> randomTex;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture1D(&texDesc, &initData, &randomTex)
		);
	
	// Create the resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;

	DX::ThrowIfFailed(
		m_d3dDevice->CreateShaderResourceView(randomTex.Get(), &viewDesc, &m_randomTexSRV)
		);	

	// Create the sampler 
	D3D11_SAMPLER_DESC randomSamplerDesc;
	ZeroMemory(&randomSamplerDesc, sizeof(randomSamplerDesc));
	randomSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	randomSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	randomSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	randomSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	randomSamplerDesc.MipLODBias = 0.0f;
	randomSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	randomSamplerDesc.BorderColor[0] = 0.0f;
	randomSamplerDesc.BorderColor[1] = 0.0f;
	randomSamplerDesc.BorderColor[2] = 0.0f;
	randomSamplerDesc.BorderColor[3] = 0.0f;
	// Allow use of all mip levels 
	randomSamplerDesc.MinLOD = 0;
	randomSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateSamplerState(
		&randomSamplerDesc,
		&m_randomTexSampler)
		);

	//
	// Create const buffer
	//
	CD3D11_BUFFER_DESC constantBufferDesc1(sizeof(cbPerFrame), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateBuffer(
		&constantBufferDesc1,
		nullptr,
		&m_constantPerFrameBuff
		)
		);
	CD3D11_BUFFER_DESC constantBufferDesc2(sizeof(cbPerObject), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateBuffer(
		&constantBufferDesc2,
		nullptr,
		&m_constantPerObjectBuff
		)
		);

	//
	// Create DepthStencil State.
	//
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	m_d3dDevice->CreateDepthStencilState(&dsDesc, &m_depthStencilState);

	//
	// Create blend state
	//
	D3D11_BLEND_DESC1 desc = {};
	desc.IndependentBlendEnable = FALSE;
	desc.AlphaToCoverageEnable = FALSE;
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].LogicOpEnable = FALSE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_d3dDevice->CreateBlendState1(&desc, &m_blendState);
	
	//
	// Create rs state
	//
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(rsDesc));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;

	DX::ThrowIfFailed(
		m_d3dDevice->CreateRasterizerState(&rsDesc, &m_rsState)
		);

	auto loadVSTask = DX::ReadDataAsync("InitVertexShader.cso");
	auto loadDrawVSTask = DX::ReadDataAsync("DrawVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync("PixelShader.cso");
	auto loadGSSOTask = DX::ReadDataAsync("GeometryShaderWithSO.cso");
	auto loadDrawGSTask = DX::ReadDataAsync("DrawGeometryShader.cso");
	
	auto createVSTask = loadVSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(
 				fileData->Data,
				fileData->Length,
				nullptr,
				&m_vertexShader
				)
			);

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },		//Vertex position				
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },		//Vertex color
			{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },		//Particle velocity
			{ "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },				//Particle age
			{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 40,  D3D11_INPUT_PER_VERTEX_DATA, 0 },				//Particle type, emitter or not.
		};

		DX::ThrowIfFailed(
			m_d3dDevice->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				fileData->Data,
				fileData->Length,
				&m_inputLayout
				)
			);		

	});

	auto createGSSOTask = loadGSSOTask.then([this](Platform::Array<byte>^ fileData){
		D3D11_SO_DECLARATION_ENTRY entry[] =
		{
			{ 0, "POSITION", 0, 0, 3, 0 },			
			{ 0, "COLOR", 0, 0, 3, 0 },
			{ 0, "VELOCITY", 0, 0, 3, 0 },
			{ 0, "AGE", 0, 0, 1, 0 },
			{ 0, "TYPE", 0, 0, 1, 0 },
		};
		// Create geometry shader with stream-out
		m_d3dDevice->CreateGeometryShaderWithStreamOutput(
			fileData->Data,
			fileData->Length,
			entry,
			ARRAYSIZE(entry),
			nullptr,
			0,
			0,
			nullptr,
			&m_geometryShaderWithSO
			);
		
	
	});
	
	auto createDrawVSTask = loadDrawVSTask.then([this](Platform::Array<byte>^ fileData){
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(
 				fileData->Data,
				fileData->Length,
				nullptr,
				&m_drawVertexShader
				)
			);		
			
	});

	auto createDrawGSTask = loadDrawGSTask.then([this](Platform::Array<byte>^ fileData){
		DX::ThrowIfFailed(
			m_d3dDevice->CreateGeometryShader(
			fileData->Data,
			fileData->Length,
			nullptr,
			&m_drawGeometryShader
			)
			);
		
	});

	auto createPSTask = loadPSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreatePixelShader(
				fileData->Data,
				fileData->Length,
				nullptr,
				&m_pixelShader
				)
			);	

		// Load texture
		ComPtr<ID3D11Resource> texture;
		DX::ThrowIfFailed(
			DX::CreateDDSTextureFromFile(m_d3dDevice.Get(), L"Texture.dds", &texture, &m_textureView)
			);
		// Create the sampler 
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;

		// Allow use of all mip levels 
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		DX::ThrowIfFailed(
			m_d3dDevice->CreateSamplerState(
			&samplerDesc,
			&m_sampler)
			);
	});

	auto createCubeTask = (createPSTask && createVSTask && createDrawVSTask && createGSSOTask &&createDrawGSTask).then([this] () {
		// Initial the emitter particle		
		EmitterParticleVertex vertices[] = 
		{
			{XMFLOAT3(0.0f, 0.2f, 0.2f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), 0.0f, 0},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = vertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_initVB
				)
			);
		CD3D11_BUFFER_DESC vertexBufferDesc1(sizeof(EmitterParticleVertex) * 100, D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&vertexBufferDesc1,
				nullptr,
				&m_drawVB
				)
			);
		
		CD3D11_BUFFER_DESC vertexBufferDesc2(sizeof(EmitterParticleVertex) * 100, D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&vertexBufferDesc2, 
				nullptr, 
				&m_streamOutVB
				)
			);
		unsigned short indices[] = 
		{
			0
		};

		m_indexCount = ARRAYSIZE(indices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = indices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
				)
			);
	});

	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});


	
}

void ParticleRenderer::CreateWindowSizeDependentResources()
{
	DirectXBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// Note that the m_orientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.
	XMStoreFloat4x4(
		&m_cbPerObject.projection,
		XMMatrixTranspose(
			XMMatrixMultiply(
				XMMatrixPerspectiveFovRH(
					fovAngleY,
					aspectRatio,
					0.01f,
					100.0f
					),
				XMLoadFloat4x4(&m_orientationTransform3D)
				)
			)
		);

	XMVECTOR eye = XMVectorSet(0.0f, 0.7f, -2.5f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&m_cbPerObject.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	XMStoreFloat4x4(&m_cbPerObject.model, XMMatrixTranspose(XMMatrixRotationY(XM_PI)));
}

void ParticleRenderer::Update(float timeTotal, float timeDelta)
{
	// Rotate view with time growing
	// XMStoreFloat4x4(&m_cbPerObject.view, (XMLoadFloat4x4(&m_cbPerObject.view) * XMMatrixRotationY(timeTotal * XM_PIDIV4)));
	
	// Update the gLook global variable.
	// Here are some mathematical knowledge.
	// Note that the origin view matrix was transposed in previous code.
	m_cbPerObject.gLook =
		XMFLOAT4(m_cbPerObject.view._31, m_cbPerObject.view._32, m_cbPerObject.view._33, 0.0f);
	// Growing time
	m_cbPerFrame.gTime = timeTotal;
	m_cbPerFrame.gTimeStep = timeDelta;
	
}

void ParticleRenderer::Render(float* backgroundColor)
{	
	m_d3dContext->ClearRenderTargetView(
		m_d3dRenderTargetView.Get(),
		backgroundColor
		);

	// Only draw the scene once it is loaded (loading is asynchronous).
	if (!m_loadingComplete)
	{
		return;
	}

	// Set the depth stencil view to null for transparent rendering.
	m_d3dContext->OMSetRenderTargets(
		1,
		m_d3dRenderTargetView.GetAddressOf(),
		nullptr
		);

	//
	// The first render pipeline round.
	//
	// Disable depth-stencil test
	m_d3dContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	m_d3dContext->UpdateSubresource(
		m_constantPerObjectBuff.Get(),
		0,
		NULL,
		&m_cbPerObject,
		0,
		0
		);
	
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	UINT stride = sizeof(EmitterParticleVertex);
	UINT offset = 0;
	if(m_isFirstDraw)
	{
		// We initial emitter particle on the the first rendering time.
		m_d3dContext->IASetVertexBuffers(
			0,
			1,
			m_initVB.GetAddressOf(),
			&stride,
			&offset
			);
	}
	else
	{
		m_d3dContext->IASetVertexBuffers(
			0,
			1,
			m_drawVB.GetAddressOf(),
			&stride,
			&offset
			);
	}
	// In the first rendering round, we just produce and update particles
	// to the next render pipeline round. So we don't need to render the pixels.
	// Set pixel shader to null for protecting rendering any pixels.
	m_d3dContext->PSSetShader( 
		nullptr, 
		nullptr,
		0
		);

	// Set geometry shader with stream-out for producing and updating particles.
	m_d3dContext->GSSetShader(
		m_geometryShaderWithSO.Get(),
		nullptr,
		0
		);
	
	m_d3dContext->UpdateSubresource(
		m_constantPerFrameBuff.Get(),
		0,
		NULL,
		&m_cbPerFrame,
		0,
		0
		);

	// Set pre-frame constant buffer to the GS.
	m_d3dContext->GSSetConstantBuffers(0, 1, m_constantPerFrameBuff.GetAddressOf());
	// Use this method to lead GS to output data to specify vertex buffer.
	m_d3dContext->SOSetTargets(1, m_streamOutVB.GetAddressOf(), &offset);

	m_d3dContext->GSSetShaderResources( // For random vector
        0,                              // starting at the first shader resource slot 
        1,                              // set one shader resource binding 
        m_randomTexSRV.GetAddressOf() 
        ); 
	m_d3dContext->GSSetSamplers( 
        0,                              // starting at the first sampler slot 
        1,                              // set one sampler binding 
        m_randomTexSampler.GetAddressOf() 
        ); 
	if(m_isFirstDraw)
	{
		m_d3dContext->IASetIndexBuffer(
			m_indexBuffer.Get(),
			DXGI_FORMAT_R16_UINT,
			0
			);
		m_d3dContext->DrawIndexed(
			m_indexCount,
			0,
			0
			);
		m_isFirstDraw = false;
	}
	else
	{
		m_d3dContext->DrawAuto();
	}
	//
	// The next render pipeline round.
	//
	// For transparent rendering.
	m_d3dContext->OMSetBlendState(m_blendState.Get(), nullptr, 0xffffffff);

	// Set SO target to null for normal geometry shader.
	ComPtr<ID3D11Buffer> bufferArray = nullptr;
	m_d3dContext->SOSetTargets(1, bufferArray.GetAddressOf(), &offset);
	// Set another geometry shader for implementing Billboard. 
	m_d3dContext->GSSetShader(
		m_drawGeometryShader.Get(),
		nullptr,
		0
		);
	m_d3dContext->GSSetConstantBuffers(
		0,
		1,
		m_constantPerObjectBuff.GetAddressOf()
		);

	// Input layout must be set again.
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(
		m_drawVertexShader.Get(),
		nullptr,
		0
		);
	
	m_d3dContext->IASetVertexBuffers(0, 1, m_streamOutVB.GetAddressOf(), &stride, &offset);
	// Here we set pixel shader.
	m_d3dContext->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);
	// Set particle texture
	m_d3dContext->PSSetShaderResources( 
        0,                          // starting at the first shader resource slot 
        1,                          // set one shader resource binding 
        m_textureView.GetAddressOf() 
        ); 
	m_d3dContext->PSSetSamplers( 
        0,                          // starting at the first sampler slot 
        1,                          // set one sampler binding 
        m_sampler.GetAddressOf() 
        ); 

	m_d3dContext->RSSetState(m_rsState.Get());

	m_d3dContext->DrawAuto();

	// Swap the buffers
	std::swap(m_drawVB, m_streamOutVB);
}
// Rotate the view space with coordinte offset.
void ParticleRenderer::RotateWithMouse(
	float x1,	// Start x-coordinate
	float y1,	// Start y-coordinate		
	float x2,	// End x-coordinate
	float y2	// End y-coordinate
	)
{
	float angle = (x2 - x1) / 100.0f;
	XMStoreFloat4x4(&m_cbPerObject.view, XMLoadFloat4x4(&m_cbPerObject.view) * XMMatrixRotationY(-angle));
}