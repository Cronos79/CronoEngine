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
#include "App.h"
#include "Scene/Entity/Component/TransformComponent.h"
#include "../CEngine/Scene/Scene.h"


namespace CronoEngine
{

	App::App( int width, int height, std::string title, bool useAsSurface /*= false*/ )
		:Application(width, height, title, useAsSurface)
	{
		
	}

	void App::HandleInput( float deltaTime )
	{
		if (CRenderer->GetWindow()->kbd.KeyIsPressed( VK_ESCAPE ))
		{
			PostQuitMessage( 0 );
		}
		if (CRenderer->GetWindow()->kbd.KeyIsPressed( 'V' ))
		{
			CRenderer->Gfx().ToggleVSync();
		}
		if (CRenderer->GetWindow()->kbd.KeyIsPressed( VK_F11 ))
		{
			CRenderer->GetWindow()->SetFullscreen();
		}
	}

	void App::Update( float deltaTimeF )
	{	
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin( "FPS" );
		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
		ImGui::End();
		ImGui::Begin( "FPS1" );
		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
		ImGui::End();
	}

	void App::ShutDown()
	{

	}
}
CronoEngine::Application* CreateEngineApp()
{	
	return new CronoEngine::App(1920,1080, "CronoEditor", false);
}