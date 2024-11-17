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
#include "Common/CronoException.h"
#include "Graphics/Renderer.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

namespace CronoEngine::Graphics::CD3D12
{
	constexpr int32_t FrameBufferCount{ 3 };
}

#ifdef _DEBUG
#define NAME_D3D12_OBJECT(obj, name) obj->SetName(name); OutputDebugStringW(L"::D3D12 object created: "); OutputDebugStringW(name);OutputDebugStringW(L"\n");
#define NAME_D3D12_OBJECT_Indexed(obj, n, name)				\
{															\
	wchar_t fullName[128];									\
	if(swprintf_s(fullName, L"%s[%u]", name, n) > 0)		\
	{														\
		obj->SetName(fullName);								\
		OutputDebugStringW(L"::D3D12 object created: ");	\
		OutputDebugStringW(name);							\
		OutputDebugStringW(L"\n");							\
	}														\
}
#else
#define NAME_D3D12_OBJECT(obj, name)
#define NAME_D3D12_OBJECT_Indexed(obj, n, name)
#endif // _DEBUG


