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
#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "Direct3D12/DX12Interface.h"

namespace CronoEngine::Graphics
{
	namespace
	{
		PlatformInterface gfx{};

		bool SetPlatformInterface( GraphicsPlatform platform )
		{
			switch (platform)
			{
			case CronoEngine::Graphics::GraphicsPlatform::Direct3D12:
				CD3D12::GetPlatformInterface( gfx );
				break;
			case CronoEngine::Graphics::GraphicsPlatform::OpenGL:
				break;
			default:
				break;
			}
			return true;
		}
	}

	bool Initialize( GraphicsPlatform platform )
	{
		return SetPlatformInterface( platform ) && gfx.Initialize();
	}

	void Shutdown()
	{
		gfx.Shutdown();
	}

	void Render()
	{
		gfx.Render();
	}

}