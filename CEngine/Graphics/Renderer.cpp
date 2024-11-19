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
#include "Common/CronoException.h"

namespace CronoEngine::Graphics
{
	
	Renderer::Renderer( int32_t width, int32_t height, std::string title, bool hasBorder )
		: _Width(width), _Height(height)
	{		
		CWindow = CreateCWindow( width, height, title.c_str(), hasBorder );	
		CWindow->Gfx().Init();
	}

	Renderer::~Renderer()
	{

	}

	CronoEngine::Graphics::DX12Core& Renderer::Gfx()
	{
		if (&CWindow->Gfx() == nullptr)
		{
			throw CHWND_NOGFX_EXCEPT();
		}
		return CWindow->Gfx();
	}

	CronoEngine::Window* Renderer::GetWindow()
	{
		return CWindow;
	}

	void Renderer::Resize( int32_t width, int32_t height )
	{
		CWindow->Resize( width, height );
		CWindow->Gfx().Resize( width, height );
	}

}