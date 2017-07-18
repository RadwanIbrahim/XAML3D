#pragma once
using namespace Windows::Foundation::Numerics;

namespace DirectXModel
{
	public ref class Transform sealed
	{
	public:
		Transform();
		property float3 Translation;
		property float3 Rotation;
		property float3 Scale;

	internal:
		DirectX::XMMATRIX GetWorldMatrix();
		
	};
}

