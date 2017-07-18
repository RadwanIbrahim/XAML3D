#pragma once
#include "pch.h"
#include "DXModel.h"
#include "Common\DirectXHelper.h"
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <math.h>
#include <ppltasks.h>
#include <windows.ui.xaml.media.dxinterop.h>

using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Interop;
using namespace Concurrency;
using namespace DirectX;
using namespace D2D1;
using namespace DirectXModel;
using namespace DX;


DXModel::DXModel()
{
	critical_section::scoped_lock lock(m_criticalSection);
	m_Transform = ref new DirectXModel::Transform();
	m_Camera = ref new DirectXModel::Camera();
	m_Camera->LookAt(float3(0.0f, 0.0f, -4.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
	this->Unloaded += ref new RoutedEventHandler(this, &DXModel::OnUnloaded);

	CreateDeviceIndependentResources();
	CreateDeviceResources();
	CreateSizeDependentResources();	
	
}

DXModel::~DXModel()
{
	m_renderLoopWorker->Cancel();

	m_renderLoopWorker->Cancel();
	for (auto m : m_meshModels)
		delete m;
	m_meshModels.clear();

	if (m_skinnedMeshRenderer != nullptr)
		delete	m_skinnedMeshRenderer;
}

Platform::String^ DXModel::ModelName::get()
{
	return m_ModelName;
}

DirectXModel::Transform^ DXModel::Transform::get()
{
	return m_Transform;
}

DirectXModel::Camera^ DXModel::Camera::get()
{
	return m_Camera;
}

void DXModel::ModelName::set(Platform::String^ value)
{
	m_ModelName = value;

	auto CreateModelTask = LoadAsync(m_graphics, ModelName->Data());
	CreateModelTask.then([this]() {
		m_loadingComplete = true;
		ModelLoaded(this, true);
	});

	StartRenderLoop();
}


void DXModel::OnUnloaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
	StopRenderLoop();
}


concurrency::task<void> DXModel::LoadAsync(Graphics& graphics,
	const std::wstring& meshFilename,
	const bool IsAnimated,
	const std::wstring& shaderPathLocation,
	const std::wstring& texturePathLocation
)

{
	isanimated = IsAnimated;

	auto task = Mesh::LoadFromFileAsync(graphics, meshFilename, shaderPathLocation, texturePathLocation, m_meshModels);

	if (isanimated)
	{

		return task.then([this]()
		{
			for (Mesh* m : m_meshModels)
			{
				if (m->BoneInfoCollection().empty() == false)
				{
					auto animState = new AnimationState();
					animState->m_boneWorldTransforms.resize(m->BoneInfoCollection().size());
					m->Tag = animState;
				}
			}

		}).then([this, &graphics]() {
			m_skinnedMeshRenderer = new SkinnedMeshRenderer();
			return m_skinnedMeshRenderer->InitializeAsync(graphics.GetDevice(), graphics.GetDeviceContext());
		});
	}
	else
	{
		return task;
	}


}

void DXModel::UpdateAnimation(float TimeDelta)
{
	if (isanimated)
		m_skinnedMeshRenderer->UpdateAnimation(TimeDelta, m_meshModels);
}


void DXModel::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Create a task that will be run on a background thread.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{
			m_timer.Tick([&]()
			{
				critical_section::scoped_lock lock(m_criticalSection);
				Render();
			});

			// Halt the thread until the next vblank is reached.
			// This ensures the app isn't updating and rendering faster than the display can refresh,
			// which would unnecessarily spend extra CPU and GPU resources.  This helps improve battery life.
			m_dxgiOutput->WaitForVBlank();
		}
	});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DXModel::StopRenderLoop()
{
	// Cancel the asynchronous task and let the render thread exit.
	m_renderLoopWorker->Cancel();
}

void DXModel::Render()
{
	if (!m_loadingComplete || m_timer.GetFrameCount() == 0)
	{
		return;
	}

	// Set render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_renderTargetView.Get() };
	m_d3dContext->OMSetRenderTargets(1, targets, m_depthStencilView.Get());
	XMVECTORF32 Transparent = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
	// Clear the back buffer and depth stencil view.
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), DirectX::Colors::Transparent);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_TransformMatrix = this->Transform->GetWorldMatrix();

	if (isanimated)
	{

		for (UINT i = 0; i < m_meshModels.size(); i++)
		{

			if (m_meshModels[i]->Tag != nullptr)
			{
				// Mesh has animation - render skinned mesh.
				m_skinnedMeshRenderer->RenderSkinnedMesh(m_meshModels[i], m_graphics, m_Camera, m_TransformMatrix);
			}
			else
			{
				// Mesh does not have animation - render as usual.
				m_meshModels[i]->Render(m_graphics, m_Camera, m_TransformMatrix);
			}
		}
	}
	else
	{
		for (UINT i = 0; i < m_meshModels.size(); i++)
		{
			// Mesh does not have animation - render as usual.
			m_meshModels[i]->Render(m_graphics, m_Camera, m_TransformMatrix);

		}
	}

	Present();
}

void DXModel::CreateDeviceResources()
{
	DirectXPanelBase::CreateDeviceResources();

	ComPtr<IDXGIFactory1> dxgiFactory;
	ThrowIfFailed(
		CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))
	);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(
		dxgiFactory->EnumAdapters(0, &dxgiAdapter)
	);

	ThrowIfFailed(
		dxgiAdapter->EnumOutputs(0, &m_dxgiOutput)
	);

	m_graphics.Initialize(m_d3dDevice.Get(), m_d3dContext.Get(), m_d3dFeatureLevel);


	CD3D11_RASTERIZER_DESC d3dRas(D3D11_DEFAULT);
	d3dRas.CullMode = D3D11_CULL_NONE;
	d3dRas.MultisampleEnable = true;
	d3dRas.AntialiasedLineEnable = true;

	ComPtr<ID3D11RasterizerState> p3d3RasState;
	m_d3dDevice->CreateRasterizerState(&d3dRas, &p3d3RasState);
	m_d3dContext->RSSetState(p3d3RasState.Get());

}

void DXModel::CreateSizeDependentResources()
{
	m_renderTargetView = nullptr;
	m_depthStencilView = nullptr;

	DirectXPanelBase::CreateSizeDependentResources();

	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
	);

	// Create render target view.
	ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_renderTargetView)
	);

	// Create and set viewport.
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		m_renderTargetWidth,
		m_renderTargetHeight
	);

	m_d3dContext->RSSetViewports(1, &viewport);

	// Create depth/stencil buffer descriptor.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		static_cast<UINT>(m_renderTargetWidth),
		static_cast<UINT>(m_renderTargetHeight),
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	// Allocate a 2-D surface as the depth/stencil buffer.
	ComPtr<ID3D11Texture2D> depthStencil;
	ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil)
	);

	// Create depth/stencil view based on depth/stencil buffer.
	const CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
	ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_depthStencilView
		)
	);

	m_miscConstants.ViewportHeight = m_renderTargetHeight;
	m_miscConstants.ViewportWidth = m_renderTargetWidth;
	m_graphics.UpdateMiscConstants(m_miscConstants);


	float aspectRatio = m_renderTargetWidth / m_renderTargetHeight;
	float fovAngleY = 70.0f * XM_PI / 180.0f;
	
	float4x4 orientation =
		float4x4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);


	m_Camera->SetLens(fovAngleY, aspectRatio, orientation, 1.00f, 1000.0f);

	m_Camera->UpdateViewMatrix();

	// Setup lightinUpdateViewMatrix();g for our scene.
	static const XMVECTORF32 s_vPos = { 5.0f, 5.0f, -2.5f, 0.f };

	XMFLOAT4 dir;
	DirectX::XMStoreFloat4(&dir, XMVector3Normalize(s_vPos));

	m_lightConstants.ActiveLights = 1;
	m_lightConstants.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_lightConstants.IsPointLight[0] = false;
	m_lightConstants.LightColor[0] = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_lightConstants.LightDirection[0] = dir;
	m_lightConstants.LightSpecularIntensity[0].x = 2;

	m_graphics.UpdateLightConstants(m_lightConstants);
}

void DXModel::Clear(Color color)
{
	// Convert color values.
	const float clearColor[4] = { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f };
	// Clear render target view with given color.
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}