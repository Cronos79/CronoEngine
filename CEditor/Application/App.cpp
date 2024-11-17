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

	void App::Update( float deltaTime )
	{	
		//OutputDebugString( "OnUpdate\n" );
	}

	void App::ShutDown()
	{

	}
}
CronoEngine::Application* CreateEngineApp()
{	
	return new CronoEngine::App(1920,1080, "CronoEditor", false);
}