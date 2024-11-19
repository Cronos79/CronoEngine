/******************************************************************************************
*	CronoGames Game Engine																  *
*	Copyright © 2024 CronoGames <http://www.cronogames.net>								  *
*																						  *
*	This file is part of CronoGames Game Engine.										  *
*																						  *
*	CronoGames Game Engine is free software: you can redistribute it and/or modify		  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The CronoGames Game Engine is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The CronoGames Game Engine.  If not, see <http://www.gnu.org/licenses/>.   *
******************************************************************************************/
#include "Window.h"
#include "Common/CronoException.h"
#include "Graphics/DX12/DX12CommonIncludes.h"

namespace CronoEngine
{
	Window* CreateCWindow( int width, int height, const char* name, bool useAsSurface /*= false */ )
	{
		Window* wnd = new Window( width, height, name, useAsSurface );
		return wnd;
	}

	// Window Class Stuff
	Window::WindowClass Window::WindowClass::wndClass;

	Window::WindowClass::WindowClass() noexcept
		:
		hInst( GetModuleHandle( nullptr ) )
	{
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof( wc );
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();
		wc.hIconSm = LoadIcon( 0, IDI_APPLICATION );
		RegisterClassEx( &wc );
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClass( wndClassName, GetInstance() );
	}

	const char* Window::WindowClass::GetName() noexcept
	{
		return wndClassName;
	}

	HINSTANCE Window::WindowClass::GetInstance() noexcept
	{
		return wndClass.hInst;
	}

	// Window Stuff
	Window::Window( int width, int height, const char* name, bool useAsSurface /*= false */ )
		:
		_width( width ),
		_height( height )
	{
		// calculate window size based on desired client region size
		RECT wr;
		wr.left = 100;
		wr.right = width + wr.left;
		wr.top = 100;
		wr.bottom = height + wr.top;
		DWORD style = useAsSurface ? WS_CHILD : WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
		if (AdjustWindowRect( &wr, style, FALSE ) == 0)
		{
			throw CHWND_LAST_EXCEPT();
		}
		// create window & get hWnd
		_hWnd = CreateWindow(
			WindowClass::GetName(), name,
			style,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			nullptr, nullptr, WindowClass::GetInstance(), this
		);
		// check for error
		if (_hWnd == nullptr)
		{
			throw CHWND_LAST_EXCEPT();
		}
		// newly created windows start off as hidden
		ShowWindow( _hWnd, SW_SHOWDEFAULT );
		pGfx = std::make_unique<Graphics::DX12Core>( _hWnd, width, height );
		// register mouse raw input device
		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01; // mouse page
		rid.usUsage = 0x02; // mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (RegisterRawInputDevices( &rid, 1, sizeof( rid ) ) == FALSE)
		{
			throw CHWND_LAST_EXCEPT();
		}

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& imstyle = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			imstyle.WindowRounding = 0.0f;
			imstyle.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init( _hWnd );
	}

	Window::~Window()
	{
		DestroyWindow( _hWnd );
	}

	void Window::SetTitle( const std::string& title )
	{
		if (SetWindowText( _hWnd, title.c_str() ) == 0)
		{
			throw CHWND_LAST_EXCEPT();
		}
	}

	std::optional<int> Window::ProcessMessages() noexcept
	{
		MSG msg;
		// while queue has messages, remove and dispatch them (but do not block on empty queue)
		while (PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ))
		{
			// check for quit because peekmessage does not signal this via return val
			if (msg.message == WM_QUIT)
			{
				// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
				return (int)msg.wParam;
			}

			// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		// return empty optional when not quitting app
		return {};
	}

	CronoEngine::Graphics::DX12Core& Window::Gfx()
	{
		return *pGfx;
	}

	void Window::SetFullscreen( bool fullscreen )
	{
		if (_Fullscreen != fullscreen)
		{
			_Fullscreen = fullscreen;

			if (_Fullscreen) // Switching to fullscreen.
			{
				// Store the current window dimensions so they can be restored 
				// when switching out of fullscreen state.
				::GetWindowRect( _hWnd, &_WindowRect );
				// Set the window style to a borderless window so the client area fills
				// the entire screen.
				UINT windowStyle = 0 & ~(WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

				::SetWindowLongW( _hWnd, GWL_STYLE, windowStyle );
				// Query the name of the nearest display device for the window.
				// This is required to set the fullscreen dimensions of the window
				// when using a multi-monitor setup.
				HMONITOR hMonitor = ::MonitorFromWindow( _hWnd, MONITOR_DEFAULTTONEAREST );
				MONITORINFOEX monitorInfo = {};
				monitorInfo.cbSize = sizeof( MONITORINFOEX );
				::GetMonitorInfo( hMonitor, &monitorInfo );
				::SetWindowPos( _hWnd, HWND_TOP,
					monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.top,
					monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE );

				::ShowWindow( _hWnd, SW_MAXIMIZE );
			}
			else
			{
				// Restore all the window decorators.
				::SetWindowLong( _hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW );

				::SetWindowPos( _hWnd, HWND_NOTOPMOST,
					_WindowRect.left,
					_WindowRect.top,
					_WindowRect.right - _WindowRect.left,
					_WindowRect.bottom - _WindowRect.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE );

				::ShowWindow( _hWnd, SW_NORMAL );
			}
		}
	}

	void Window::SetFullscreen()
	{
		SetFullscreen( !_Fullscreen );
	}

	void Window::Resize( int32_t width, int32_t height )
	{
		_width = width;
		_height = height;
		// #TODO: Need to do anything to resize?
	}

	LRESULT CALLBACK Window::HandleMsgSetup( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window instance
			SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd) );
			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr( hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk) );
			// forward message to window instance handler
			return pWnd->HandleMsg( hWnd, msg, wParam, lParam );
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc( hWnd, msg, wParam, lParam );
	}

	LRESULT CALLBACK Window::HandleMsgThunk( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		// retrieve ptr to window instance
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr( hWnd, GWLP_USERDATA ));
		// forward message to window instance handler
		return pWnd->HandleMsg( hWnd, msg, wParam, lParam );
	}

#include "imgui_impl_win32.h"
	// Forward declare message handler from imgui_impl_win32.cpp
	//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	LRESULT Window::HandleMsg( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		if (ImGui_ImplWin32_WndProcHandler( hWnd, msg, wParam, lParam ))
		{
			return true;
		}
		//const auto& imio = ImGui::GetIO();

		switch (msg)
		{
		case WM_SIZE:
		{
			RECT clientRect = {};
			::GetClientRect( _hWnd, &clientRect );

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			if (_width != width || _height != height)
			{
				Resize( width, height );
				pGfx->Resize( width, height );
			}		
		}
		break;
		case WM_PAINT:
		{
			//pGfx->Render();
		}
		break;
			// we don't want the DefProc to handle this message because
			// we want our destructor to destroy the window, so return 0 instead of break
		case WM_CLOSE:
			PostQuitMessage( 0 );
			return 0;
			// clear keystate when window loses focus to prevent input getting "stuck"
		case WM_KILLFOCUS:
			kbd.ClearState();
			break;
			/*********** KEYBOARD MESSAGES ***********/
		case WM_KEYDOWN:
			// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		case WM_SYSKEYDOWN:
			// stifle this keyboard message if imgui wants to capture
		/*	if (imio.WantCaptureKeyboard)
			{
				break;
			}*/
			if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled()) // filter autorepeat
			{
				kbd.OnKeyPressed( static_cast<unsigned char>(wParam) );
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			// stifle this keyboard message if imgui wants to capture
		/*	if (imio.WantCaptureKeyboard)
			{
				break;
			}*/
			kbd.OnKeyReleased( static_cast<unsigned char>(wParam) );
			break;
		case WM_CHAR:
			// stifle this keyboard message if imgui wants to capture
			/*if (imio.WantCaptureKeyboard)
			{
				break;
			}*/
			kbd.OnChar( static_cast<unsigned char>(wParam) );
			break;
			/*********** END KEYBOARD MESSAGES ***********/

			/************* MOUSE MESSAGES ****************/
		case WM_MOUSEMOVE:
		{
			const POINTS pt = MAKEPOINTS( lParam );		
			// stifle this mouse message if imgui wants to capture
		/*	if (imio.WantCaptureMouse)
			{
				break;
			}*/
			// in client region -> log move, and log enter + capture mouse (if not previously in window)
			if (pt.x >= 0 && pt.x < _width && pt.y >= 0 && pt.y < _height)
			{
				mouse.OnMouseMove( pt.x, pt.y );
				if (!mouse.IsInWindow())
				{
					SetCapture( hWnd );
					mouse.OnMouseEnter();
				}
			}
			// not in client -> log move / maintain capture if button down
			else
			{
				if (wParam & (MK_LBUTTON | MK_RBUTTON))
				{
					mouse.OnMouseMove( pt.x, pt.y );
				}
				// button up -> release capture / log event for leaving
				else
				{
					ReleaseCapture();
					mouse.OnMouseLeave();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			SetForegroundWindow( hWnd );
			// stifle this mouse message if imgui wants to capture
			/*if (imio.WantCaptureMouse)
			{
				break;
			}*/
			const POINTS pt = MAKEPOINTS( lParam );
			mouse.OnLeftPressed( pt.x, pt.y );
			break;
		}
		case WM_RBUTTONDOWN:
		{
			// stifle this mouse message if imgui wants to capture
			/*if (imio.WantCaptureMouse)
			{
				break;
			}*/
			const POINTS pt = MAKEPOINTS( lParam );
			mouse.OnRightPressed( pt.x, pt.y );
			break;
		}
		case WM_LBUTTONUP:
		{
			// stifle this mouse message if imgui wants to capture
			/*if (imio.WantCaptureMouse)
			{
				break;
			}*/
			const POINTS pt = MAKEPOINTS( lParam );
			mouse.OnLeftReleased( pt.x, pt.y );
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= _width || pt.y < 0 || pt.y >= _height)
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			// stifle this mouse message if imgui wants to capture
			/*if (imio.WantCaptureMouse)
			{
				break;
			}*/
			const POINTS pt = MAKEPOINTS( lParam );
			mouse.OnRightReleased( pt.x, pt.y );
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= _width || pt.y < 0 || pt.y >= _height)
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			// stifle this mouse message if imgui wants to capture
			/*if (imio.WantCaptureMouse)
			{
				break;
			}*/
			const POINTS pt = MAKEPOINTS( lParam );
			const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
			mouse.OnWheelDelta( pt.x, pt.y, delta );
			break;
		}
		/************** END MOUSE MESSAGES **************/

		/************** RAW MOUSE MESSAGES **************/
		case WM_INPUT:
		{
			if (!mouse.RawEnabled())
			{
				break;
			}
			UINT size;
			// first get the size of the input data
			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				nullptr,
				&size,
				sizeof( RAWINPUTHEADER ) ) == -1)
			{
				// bail msg processing if error
				break;
			}
			rawBuffer.resize( size );
			// read in the input data
			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				rawBuffer.data(),
				&size,
				sizeof( RAWINPUTHEADER ) ) != size)
			{
				// bail msg processing if error
				break;
			}
			// process the raw input data
			auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (ri.header.dwType == RIM_TYPEMOUSE &&
				(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
			{
				mouse.OnRawDelta( ri.data.mouse.lLastX, ri.data.mouse.lLastY );
			}
			break;
		}
		/************** END RAW MOUSE MESSAGES **************/
		}

		return DefWindowProc( hWnd, msg, wParam, lParam );
	}
}