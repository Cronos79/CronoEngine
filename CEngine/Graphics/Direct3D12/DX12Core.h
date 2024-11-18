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
#include "DX12CommonHeaders.h"

namespace CronoEngine::Graphics::CD3D12::Core
{
	bool Initialize();
	void Shutdown();
	void Render();

	template<typename T>
	constexpr void Release( T*& resource )
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}		
	}

	namespace detail
	{
		void DeferredRelease( IUnknown* resource );
	}

	template<typename T>
	constexpr void DeferredRelease( T*& resource )
	{
		if (resource)
		{
			detail::DeferredRelease( resource );
			resource->Release();
			resource = nullptr;
		}
	}

	ID3D12Device14* const Device();
	int32_t CurrentFrameIndex();
	void SetDeferredReleasesFlag();
}