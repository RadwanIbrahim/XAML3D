#pragma once
#include "pch.h"
#include "Common\DirectXPanelBase.h"
#include "Common\StepTimer.h"
#include "Common\Model.h"
#include "VSD3DStarter.h"
#include "Animation\Animation.h"
#include "Transform.h"
#include "Camera.h"
#include <vector>


using namespace VSD3DStarter;

namespace DirectXModel
{

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DXModel sealed : DirectXModel::DirectXPanelBase
	{
	public:
		DXModel();
		property Platform::String^ ModelName
		{
			Platform::String^ get();
			void set(Platform::String^ value);
		}

		property Transform^ Transform
		{
			DirectXModel::Transform^ get();
		}
		property DirectXModel::Camera^ Camera
		{
			DirectXModel::Camera^ get();
		}
		
		void Clear(Windows::UI::Color color);
		event Windows::Foundation::EventHandler<bool>^ ModelLoaded;
	private protected:

		virtual void Render() override;
		virtual void CreateDeviceResources() override;
		virtual void CreateSizeDependentResources() override;

	private:
		DX::StepTimer                                       m_timer;
		DirectXModel::Camera^								m_Camera;
		SkinnedMeshRenderer*								m_skinnedMeshRenderer;
		bool isanimated;
		Platform::String^									m_ModelName;
		DirectXModel::Transform^							m_Transform;
		DirectX::XMMATRIX									m_TransformMatrix;
		std::vector<VSD3DStarter::Mesh*>				    m_meshModels;

		Microsoft::WRL::ComPtr<IDXGIOutput>                 m_dxgiOutput;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;
		Windows::Foundation::IAsyncAction^					m_renderLoopWorker;

		// Members used to keep track of the graphics state.
		VSD3DStarter::Graphics								m_graphics;
		VSD3DStarter::LightConstants						m_lightConstants;
		VSD3DStarter::MiscConstants							m_miscConstants;

		concurrency::task<void> LoadAsync(Graphics& graphics,
			const std::wstring& meshFilename,
			const bool IsAnimated = false,
			const std::wstring& shaderPathLocation = L"",
			const std::wstring& texturePathLocation = L""
		);

		void UpdateAnimation(float TimeDelta);
		void StartRenderLoop();
		void StopRenderLoop();
		void OnUnloaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);

		~DXModel();
	};
}