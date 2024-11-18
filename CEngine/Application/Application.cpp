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

	Application::Application(int width, int height, std::string title, bool useAsSurface /*= false*/ )
	{
		m_Project = new Project();
		m_Surface = new Graphics::RenderSurface();
		m_Surface->CWindow = CreateCWindow( width, height, title.c_str(), useAsSurface );
		CronoEngine::Graphics::Initialize( CronoEngine::Graphics::GraphicsPlatform::Direct3D12 );
	}

	Application::~Application()
	{
		//CronoEngine::Graphics::Shutdown();
	}

	int Application::Run()
	{	
		while (true)
		{
			// process all messages pending, but to not block for new messages
			if (const auto ecode = Window::ProcessMessages())
			{
				// if return optional has value, means we're quitting so return exit code
				return *ecode;
			}
			// execute the game logic
			const auto dt = 0.0f;//timer.Mark() * speed_factor;
			Update( dt );
		}
	}
}
