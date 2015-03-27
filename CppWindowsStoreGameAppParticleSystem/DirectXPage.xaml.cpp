
#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace CppWindowsStoreAppParticleSystem;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

DirectXPage::DirectXPage() :
	m_renderNeeded(true)
{
	InitializeComponent();

	m_renderer = ref new ParticleRenderer();

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();	
	m_renderer->Initialize(
		Window::Current->CoreWindow,
		SwapChainPanel,
		currentDisplayInformation->LogicalDpi
		);

	/*if (!m_renderer->CheckSupportGS())
	{
		ErrorText->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
*/
	m_accessibilitySettings =
		ref new Windows::UI::ViewManagement::AccessibilitySettings;

	m_accessibilitySettings->HighContrastChanged +=
		ref new TypedEventHandler<Windows::UI::ViewManagement::AccessibilitySettings^, Object^>(this, &DirectXPage::OnHighContrastChanged);

	/*if (m_accessibilitySettings->HighContrast)
	{
		LogoImage->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	}*/

	Window::Current->CoreWindow->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXPage::OnWindowSizeChanged);
	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);
	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);
	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);	
	m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));

	m_timer = ref new BasicTimer();

	//Windows::UI::Xaml::Media::SolidColorBrush^ br =
	//	(Windows::UI::Xaml::Media::SolidColorBrush^)Windows::UI::Xaml::Application::Current->Resources->
	//	Lookup(
	//	"ApplicationPageBackgroundThemeBrush"
	//	);
	//Windows::UI::Color Color = br->Color;
	/*m_renderTargetColor[0] = Color.R;
	m_renderTargetColor[1] = Color.G;
	m_renderTargetColor[2] = Color.B;
	m_renderTargetColor[3] = Color.A;*/
	m_renderTargetColor[0] = 0.f;
	m_renderTargetColor[1] = 0.f;
	m_renderTargetColor[2] = 0.f;
	m_renderTargetColor[3] = 1.0f;
}

void DirectXPage::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();

	if (args->Size.Width <= 500)
	{
		VisualStateManager::GoToState(this, "MinimalLayout", true);
	}
	else if (args->Size.Width < args->Size.Height)
	{
		VisualStateManager::GoToState(this, "PortraitLayout", true);
	}
	else
	{
		VisualStateManager::GoToState(this, "DefaultLayout", true);
	}
	
}

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	m_renderer->SetDpi(sender->LogicalDpi);
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	m_renderer->UpdateForWindowSizeChange();
}


void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	m_renderer->ValidateDevice();
}

void DirectXPage::OnHighContrastChanged(Windows::UI::ViewManagement::AccessibilitySettings^ sender, Object^ args)
{
	//// Temporarily hide the logo image in high contrast mode.
	//if (sender->HighContrast)
	//{
	//	LogoImage->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	//}
	//else
	//{
	//	LogoImage->Visibility = Windows::UI::Xaml::Visibility::Visible;
	//}

	// Update render target color.
	Windows::UI::Xaml::Media::SolidColorBrush^ br =
		(Windows::UI::Xaml::Media::SolidColorBrush^)Windows::UI::Xaml::Application::Current->Resources->
		Lookup(
		"ApplicationPageBackgroundThemeBrush"
		);
	Windows::UI::Color Color = br->Color;
	m_renderTargetColor[0] = Color.R;
	m_renderTargetColor[1] = Color.G;
	m_renderTargetColor[2] = Color.B;
	m_renderTargetColor[3] = Color.A;
}

void DirectXPage::OnRendering(Object^ sender, Object^ args)
{	
	m_timer->Update();
	m_renderer->Update(m_timer->Total, m_timer->Delta);

	// Clear the render target to the color of ApplicationPageBackgroundThemeBrush, so the app works fine
	// in high contrast mode.
	m_renderer->Render(m_renderTargetColor);
	m_renderer->Present();	
}

// Handle pointer pressed event.
void CppWindowsStoreAppParticleSystem::DirectXPage::SwapChainPanel_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_lastPoint.X = e->GetCurrentPoint(nullptr)->Position.X;
	m_lastPoint.Y = e->GetCurrentPoint(nullptr)->Position.Y;
	m_pointerMoveToken = Window::Current->CoreWindow->PointerMoved += 
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);

}

// Handle pointer moved event to rotate the view space martrix.
void CppWindowsStoreAppParticleSystem::DirectXPage::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	Point pt = args->CurrentPoint->Position;
	m_renderer->RotateWithMouse(m_lastPoint.X, m_lastPoint.Y, pt.X, pt.Y);
	m_lastPoint = args->CurrentPoint->Position;
}

void CppWindowsStoreAppParticleSystem::DirectXPage::SwapChainPanel_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Window::Current->CoreWindow->PointerMoved -= m_pointerMoveToken;
}

void CppWindowsStoreAppParticleSystem::DirectXPage::Footer_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Windows::System::Launcher::LaunchUriAsync(ref new Uri((String^)((HyperlinkButton^)sender)->Tag));
}
