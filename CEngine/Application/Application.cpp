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
#include "Application.h"
#include "../Scene/Entity/Component/TransformComponent.h"
#include "Graphics/Renderer.h"

namespace CronoEngine
{

	Application::Application(int width, int height, std::string title, bool hasBorder /*= false*/ )
	{		
		Initialize(width, height, title, hasBorder);
	}

	Application::~Application()
	{
	}

	int Application::Run()
	{			
		while (true)
		{
			// process all messages pending, but to not block for new messages
			if (const auto ecode = Window::ProcessMessages())
			{
				// if return optional has value, means we're quitting so return exit code
				CRenderer->Gfx().Shutdown();
				return *ecode;
			}
			// execute the game logic
			const auto dt = 0.0f;//timer.Mark() * speed_factor;			
			CRenderer->Gfx().BeginFrame();
			HandleInput( dt );
			Update( dt );
			UpdateUI( dt );
			CRenderer->Gfx().EndFrame();
		}
	}

	bool Application::Initialize( int width, int height, std::string title, bool hasBorder )
	{
		if (CRenderer != nullptr)
		{
			ParseCommandLineArguments();
		}
		// Check for DirectX Math library support.
		if (!DirectX::XMVerifyCPUSupport())
		{
			MessageBoxA( NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR );
			return false;
		}
		_CurrentProject = new Project();
		CRenderer = new Graphics::Renderer( width, height, title, hasBorder );

		return true;
	}

	void Application::ShutDown()
	{

	}

	void Application::ParseCommandLineArguments()
	{
		int argc;
		wchar_t** argv = ::CommandLineToArgvW( ::GetCommandLineW(), &argc );

		int32_t clientWidth = 1920;
		int32_t clientHeight = 1080;
		bool useWarp = false;
		bool isDirty = false;

		for (size_t i = 0; i < argc; ++i)
		{
			if (::wcscmp( argv[i], L"-w" ) == 0 || ::wcscmp( argv[i], L"--width" ) == 0)
			{
				clientWidth = ::wcstol( argv[++i], nullptr, 10 );
				isDirty = true;
			}
			if (::wcscmp( argv[i], L"-h" ) == 0 || ::wcscmp( argv[i], L"--height" ) == 0)
			{
				clientHeight = ::wcstol( argv[++i], nullptr, 10 );
				isDirty = true;
			}
			if (::wcscmp( argv[i], L"-warp" ) == 0 || ::wcscmp( argv[i], L"--warp" ) == 0)
			{
				useWarp = true;
				isDirty = true;
			}
		}

		if (isDirty)
		{
			CRenderer->Resize( clientWidth, clientHeight );
		}
		// Free memory allocated by CommandLineToArgvW
		::LocalFree( argv );
	}

}
