#pragma once
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>
// Helper utilities to make Win32 APIs work with exceptions.
namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<Platform::Array<byte>^> ReadDataAsync(Platform::String^ filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;
		
		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		
		return create_task(folder->GetFileAsync(filename)).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> Platform::Array<byte>^ 
		{
			auto fileData = ref new Platform::Array<byte>(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(fileData);
			return fileData;
		});
	}

	//Returns random float in [0, 1).
	inline float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}
	// Returns random float in [a, b).
	inline float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}
	inline DirectX::XMVECTOR RandUnitVec3()
	{
		DirectX::XMVECTOR One = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		DirectX::XMVECTOR Zero = DirectX::XMVectorZero();
		// Keep trying until we get a point on/in the hemisphere.
		while(true)
		{
			// Generate random point in the cube [-1,1]^3.
			DirectX::XMVECTOR v = DirectX::XMVectorSet(
				RandF(-1.0f, 1.0f),
				RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);
			// Ignore points outside the unit sphere in order to
			// get an even distribution over the unit sphere. Otherwise
			// points will clump more on the sphere near the corners
			// of the cube.
			if(DirectX::XMVector3Greater(DirectX::XMVector3LengthSq(v), One))
				continue;
			return DirectX::XMVector3Normalize(v);
		}
	}
}
