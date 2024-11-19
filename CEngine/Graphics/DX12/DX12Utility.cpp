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
#include "DX12Utility.h"
#include "Common/CommonHeaders.h"

using namespace Microsoft::WRL;

namespace CronoEngine::Graphics
{

	void DX12Utility::GetHardwareAdapter( IDXGIFactory7* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter /*= false */ )
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED( pFactory->QueryInterface( IID_PPV_ARGS( &factory6 ) ) ))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED( factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS( &adapter ) ) );
					++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1( &desc );

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ))
				{
					break;
				}
			}
		}

		if (adapter.Get() == nullptr)
		{
			
			for (UINT adapterIndex = 0; SUCCEEDED( pFactory->EnumAdapters1( adapterIndex, &adapter )); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1( &desc );

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}

}