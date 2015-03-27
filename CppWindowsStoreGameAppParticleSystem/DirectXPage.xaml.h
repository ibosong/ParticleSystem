

#pragma once

#include "DirectXPage.g.h"
#include "ParticleRenderer.h"
#include "BasicTimer.h"

namespace CppWindowsStoreAppParticleSystem
{
	/// <summary>
	/// A DirectX page that can be used on its own.  Note that it may not be used within a Frame.
	/// </summary>
    [Windows::Foundation::Metadata::WebHostHidden]
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();


	private:
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnRendering(Object^ sender, Object^ args);
		void OnHighContrastChanged(Windows::UI::ViewManagement::AccessibilitySettings^ sender, Platform::Object^ args);
		void SwapChainPanel_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void SwapChainPanel_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void Footer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		Windows::Foundation::EventRegistrationToken m_eventToken;
		ParticleRenderer^ m_renderer;
		bool m_renderNeeded;
		float m_renderTargetColor[4];
		Windows::UI::ViewManagement::AccessibilitySettings^ m_accessibilitySettings;
		BasicTimer^ m_timer;
		Windows::Foundation::Point m_lastPoint;
		Windows::Foundation::EventRegistrationToken m_pointerMoveToken;
	};
}
