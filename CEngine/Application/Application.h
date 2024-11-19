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
#pragma once
#include "Common/CommonHeaders.h"
#include "Windows/Window.h"
#include "Project/Project.h"

namespace CronoEngine
{
	class Application
	{
	public:
		Application( int width, int height, std::string title, bool hasBorder = false );
		~Application();
		int Run();
		/**
		 *  Initialize the DirectX Runtime.
		*/
		virtual bool Initialize( int width, int height, std::string title, bool hasBorder = false );

		/**
		 *  Load content required for the demo.
		 */
		virtual bool LoadContent() = 0;

		/**
		 *  Unload demo specific content that was loaded in LoadContent.
		 */
		virtual void UnloadContent() = 0;

		/**
		 * Destroy any resource that are used by the game.
		 */
		virtual void ShutDown();
	protected:
		virtual void HandleInput( float deltaTime ) = 0;
		virtual void Update( float deltaTime ) = 0;
	private:
		void ParseCommandLineArguments();
	protected:
		Project* m_Project;
		Graphics::Renderer* CRenderer = nullptr;
	};
}
// To be defined in CLIENT
CronoEngine::Application* CreateEngineApp();