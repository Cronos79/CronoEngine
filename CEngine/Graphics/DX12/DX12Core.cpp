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
#include "DX12Core.h"
#include <algorithm>

namespace CronoEngine::Graphics
{
	DX12Core::DX12Core( HWND hWnd, uint32_t width, uint32_t height )
		: _HWnd( hWnd ), _Width( width ), _Height( height )
	{

	}

	DX12Core::~DX12Core()
	{
		
	}

	void DX12Core::Init()
	{
#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugController ) ) ))
			{
				debugController->EnableDebugLayer();
			}
		}
#endif
		_TearingSupported = CheckTearingSupport();
		ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter( _UseWarp );
		_Device = CreateDevice( dxgiAdapter4 );
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 info = {};
		if (SUCCEEDED( _Device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS5, &info, sizeof( info ) ) ))
		{
			switch (info.RaytracingTier)
			{
			case D3D12_RAYTRACING_TIER_1_0:
			case D3D12_RAYTRACING_TIER_1_1:
			{
				_rayTracingSupport = true;
			}break;
			case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
			{
				_rayTracingSupport = false;
			}break;
			default:
			{
				_rayTracingSupport = false;
			}break;
			}
		}
		_CommandQueue = CreateCommandQueue( _Device, D3D12_COMMAND_LIST_TYPE_DIRECT );
		_SwapChain = CreateSwapChain( _HWnd, _CommandQueue,	_Width, _Height, NumFrames );
		_CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();
		_RTVDescriptorHeap = CreateDescriptorHeap( _Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NumFrames );
		_RTVDescriptorSize = _Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = 1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (_Device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( _SRVDescriptorHeap.GetAddressOf() ) ) != S_OK)
				return;
		}
		UpdateRenderTargetViews( _Device, _SwapChain, _RTVDescriptorHeap );
		for (int i = 0; i < NumFrames; ++i)
		{
			_CommandAllocators[i] = CreateCommandAllocator( _Device, D3D12_COMMAND_LIST_TYPE_DIRECT );
		}
		_CommandList = CreateCommandList( _Device, _CommandAllocators[_CurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT );
		_Fence = CreateFence( _Device );
		_FenceEvent = CreateEventHandle();

		// Memory stuff
		// Get GPU memory
		DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
		//bool xres = SetVideoMemoryReservation( MEM_GIB( 1 ) );
		dxgiAdapter4->SetVideoMemoryReservation( NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, MEM_GIB( 1 ) );
		dxgiAdapter4->QueryVideoMemoryInfo( NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo );

		ImGui_ImplDX12_Init( _Device.Get(), NumFrames,
			DXGI_FORMAT_R8G8B8A8_UNORM, *_SRVDescriptorHeap.GetAddressOf(),
			_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart() );
		_IsInitialized = true;
	}

	void DX12Core::Shutdown()
	{
		// Make sure the command queue has finished all commands before closing.
		Flush( _CommandQueue, _Fence, _FenceValue, _FenceEvent );

		::CloseHandle( _FenceEvent );
	}

	void DX12Core::Resize( uint32_t width, uint32_t height )
	{
		if (!_IsInitialized) return;

		if (_Width != width || _Height != height)
		{
			// Don't allow 0 size swap chain back buffers.
			_Width = std::max( 1u, width );
			_Height = std::max( 1u, height );

			// Flush the GPU queue to make sure the swap chain's back buffers
			// are not being referenced by an in-flight command list.
			Flush( _CommandQueue, _Fence, _FenceValue, _FenceEvent );
			for (int i = 0; i < NumFrames; ++i)
			{
				// Any references to the back buffers must be released
				// before the swap chain can be resized.
				_BackBuffers[i].Reset();
				_FrameFenceValues[i] = _FrameFenceValues[_CurrentBackBufferIndex];
			}
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			ThrowIfFailed( _SwapChain->GetDesc( &swapChainDesc ) );
			ThrowIfFailed( _SwapChain->ResizeBuffers( NumFrames, _Width, _Height,
				swapChainDesc.BufferDesc.Format, swapChainDesc.Flags ) );

			_CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

			UpdateRenderTargetViews( _Device, _SwapChain, _RTVDescriptorHeap );
		}
	}

	void DX12Core::EnableDebugLayer()
	{
#if defined(_DEBUG)
		// Always enable the debug layer before doing anything DX12 related
		// so all possible errors generated while creating DX12 objects
		// are caught by the debug layer.
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed( D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) ) );
		debugInterface->EnableDebugLayer();
#endif
	}

	ComPtr<IDXGIAdapter4> DX12Core::GetAdapter( bool useWarp )
	{
		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		ThrowIfFailed( CreateDXGIFactory2( createFactoryFlags, IID_PPV_ARGS( &dxgiFactory ) ) );
		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		if (useWarp)
		{
			ThrowIfFailed( dxgiFactory->EnumWarpAdapter( IID_PPV_ARGS( &dxgiAdapter1 ) ) );
			ThrowIfFailed( dxgiAdapter1.As( &dxgiAdapter4 ) );
		}
		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1( i, &dxgiAdapter1 ) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1( &dxgiAdapterDesc1 );

				// Check to see if the adapter can create a D3D12 device without actually 
				// creating it. The adapter with the largest dedicated video memory
				// is favored.
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED( D3D12CreateDevice( dxgiAdapter1.Get(),
						D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr ) ) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					ThrowIfFailed( dxgiAdapter1.As( &dxgiAdapter4 ) );
				}
			}
		}

		return dxgiAdapter4;
	}

	ComPtr<ID3D12Device14> DX12Core::CreateDevice( ComPtr<IDXGIAdapter4> adapter )
	{
		ComPtr<ID3D12Device14> d3d12Device14;
		ThrowIfFailed( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &d3d12Device14 ) ) );
		// Enable debug messages in debug mode.
#if defined(_DEBUG)
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED( d3d12Device14.As( &pInfoQueue ) ))
		{
			pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE );
			pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE );
			pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, TRUE );
			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof( Severities );
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof( DenyIds );
			NewFilter.DenyList.pIDList = DenyIds;

			ThrowIfFailed( pInfoQueue->PushStorageFilter( &NewFilter ) );
		}
#endif

		return d3d12Device14;
	}

	ComPtr<ID3D12CommandQueue> DX12Core::CreateCommandQueue( ComPtr<ID3D12Device14> device, D3D12_COMMAND_LIST_TYPE type )
	{
		ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		ThrowIfFailed( device->CreateCommandQueue( &desc, IID_PPV_ARGS( &d3d12CommandQueue ) ) );

		return d3d12CommandQueue;
	}

	bool DX12Core::CheckTearingSupport()
	{
		BOOL allowTearing = FALSE;

		// Rather than create the DXGI 1.5 factory interface directly, we create the
		// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
		// graphics debugging tools which will not support the 1.5 factory interface 
		// until a future update.
		ComPtr<IDXGIFactory4> factory4;
		if (SUCCEEDED( CreateDXGIFactory1( IID_PPV_ARGS( &factory4 ) ) ))
		{
			ComPtr<IDXGIFactory5> factory5;
			if (SUCCEEDED( factory4.As( &factory5 ) ))
			{
				if (FAILED( factory5->CheckFeatureSupport(
					DXGI_FEATURE_PRESENT_ALLOW_TEARING,
					&allowTearing, sizeof( allowTearing ) ) ))
				{
					allowTearing = FALSE;
				}
			}
		}

		return allowTearing == TRUE;
	}

	ComPtr<IDXGISwapChain4> DX12Core::CreateSwapChain( HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount )
	{
		ComPtr<IDXGISwapChain4> dxgiSwapChain4;
		ComPtr<IDXGIFactory4> dxgiFactory4;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		ThrowIfFailed( CreateDXGIFactory2( createFactoryFlags, IID_PPV_ARGS( &dxgiFactory4 ) ) );
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed( dxgiFactory4->CreateSwapChainForHwnd(
			commandQueue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1 ) );

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		ThrowIfFailed( dxgiFactory4->MakeWindowAssociation( hWnd, DXGI_MWA_NO_ALT_ENTER ) );
		ThrowIfFailed( swapChain1.As( &dxgiSwapChain4 ) );
		return dxgiSwapChain4;
	}

	ComPtr<ID3D12DescriptorHeap> DX12Core::CreateDescriptorHeap( ComPtr<ID3D12Device14> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors )
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		ThrowIfFailed( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &descriptorHeap ) ) );
		return descriptorHeap;
	}

	void DX12Core::UpdateRenderTargetViews( ComPtr<ID3D12Device14> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap )
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( descriptorHeap->GetCPUDescriptorHandleForHeapStart() );

		for (int i = 0; i < NumFrames; ++i)
		{
			ComPtr<ID3D12Resource> backBuffer;
			ThrowIfFailed( swapChain->GetBuffer( i, IID_PPV_ARGS( &backBuffer ) ) );
			device->CreateRenderTargetView( backBuffer.Get(), nullptr, rtvHandle );
			_BackBuffers[i] = backBuffer;
			rtvHandle.Offset( rtvDescriptorSize );
		}
	}

	ComPtr<ID3D12CommandAllocator> DX12Core::CreateCommandAllocator( ComPtr<ID3D12Device14> device, D3D12_COMMAND_LIST_TYPE type )
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ThrowIfFailed( device->CreateCommandAllocator( type, IID_PPV_ARGS( &commandAllocator ) ) );
		return commandAllocator;
	}

	ComPtr<ID3D12GraphicsCommandList> DX12Core::CreateCommandList( ComPtr<ID3D12Device14> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type )
	{
		ComPtr<ID3D12GraphicsCommandList> commandList;
		ThrowIfFailed( device->CreateCommandList( 0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS( &commandList ) ) );
		ThrowIfFailed( commandList->Close() );
		return commandList;
	}

	ComPtr<ID3D12Fence> DX12Core::CreateFence( ComPtr<ID3D12Device14> device )
	{
		ComPtr<ID3D12Fence> fence;
		ThrowIfFailed( device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &fence ) ) );
		return fence;
	}

	HANDLE DX12Core::CreateEventHandle()
	{
		HANDLE fenceEvent;
		fenceEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
		assert( fenceEvent && "Failed to create fence event." );
		return fenceEvent;
	}

	uint64_t DX12Core::Signal( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue )
	{
		uint64_t fenceValueForSignal = fenceValue;
		if (_IsInitialized)
		{
			uint64_t fenceValueForSignal = ++fenceValue;
			ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceValueForSignal ) );
		}		
		return fenceValueForSignal;
	}

	void DX12Core::WaitForFenceValue( ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration /*= std::chrono::milliseconds::max() */ )
	{
		if (!_IsInitialized) return;

		if (fence->GetCompletedValue() < fenceValue)
		{
			ThrowIfFailed( fence->SetEventOnCompletion( fenceValue, fenceEvent ) );
			::WaitForSingleObject( fenceEvent, static_cast<DWORD>(duration.count()) );
		}
	}

	void DX12Core::Flush( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent )
	{
		uint64_t fenceValueForSignal = Signal( commandQueue, fence, fenceValue );
		WaitForFenceValue( fence, fenceValueForSignal, fenceEvent );
	}	

	void DX12Core::BeginFrame()
	{
		if (!_IsInitialized) return;

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void DX12Core::EndFrame()
	{
		if (!_IsInitialized) return;
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Rendering
		ImGui::Render();

		auto commandAllocator = _CommandAllocators[_CurrentBackBufferIndex];
		auto backBuffer = _BackBuffers[_CurrentBackBufferIndex];

		commandAllocator->Reset();
		_CommandList->Reset( commandAllocator.Get(), nullptr );

		// Clear the render target.
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				backBuffer.Get(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );

			_CommandList->ResourceBarrier( 1, &barrier );
			FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv( _RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				_CurrentBackBufferIndex, _RTVDescriptorSize );

			_CommandList->ClearRenderTargetView( rtv, clearColor, 0, nullptr );
			_CommandList->OMSetRenderTargets( 1, &rtv, FALSE, nullptr );
			//_CommandList->SetDescriptorHeaps( 1, &g_pd3dSrvDescHeap );
			_CommandList->SetDescriptorHeaps( 1, _SRVDescriptorHeap.GetAddressOf() );
			ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), _CommandList.Get() );
		}
		// Present
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				backBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
			_CommandList->ResourceBarrier( 1, &barrier );
			ThrowIfFailed( _CommandList->Close() );
			//_CommandList->Close();

			ID3D12CommandList* const commandLists[] = {
				_CommandList.Get()
			};
			_CommandQueue->ExecuteCommandLists( _countof( commandLists ), commandLists );

			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			UINT syncInterval = _VSync ? 1 : 0;
			UINT presentFlags = _TearingSupported && !_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			ThrowIfFailed( _SwapChain->Present( syncInterval, presentFlags ) );
			//_SwapChain->Present( syncInterval, presentFlags );

			_FrameFenceValues[_CurrentBackBufferIndex] = Signal( _CommandQueue, _Fence, _FenceValue );
			_CurrentBackBufferIndex = _SwapChain->GetCurrentBackBufferIndex();

			WaitForFenceValue( _Fence, _FrameFenceValues[_CurrentBackBufferIndex], _FenceEvent );
		}
	}

	void DX12Core::SetFullscreen()
	{
		SetFullscreen( !_Fullscreen );
	}

	void DX12Core::SetFullscreen( bool fullscreen )
	{
		if (_Fullscreen != fullscreen)
		{
			_Fullscreen = fullscreen;

			if (_Fullscreen) // Switching to fullscreen.
			{
				// Store the current window dimensions so they can be restored 
				// when switching out of fullscreen state.
				::GetWindowRect( _HWnd, &_WindowRect );
				// Set the window style to a borderless window so the client area fills
				// the entire screen.
				UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

				::SetWindowLongW( _HWnd, GWL_STYLE, windowStyle );
				// Query the name of the nearest display device for the window.
				// This is required to set the fullscreen dimensions of the window
				// when using a multi-monitor setup.
				HMONITOR hMonitor = ::MonitorFromWindow( _HWnd, MONITOR_DEFAULTTONEAREST );
				MONITORINFOEX monitorInfo = {};
				monitorInfo.cbSize = sizeof( MONITORINFOEX );
				::GetMonitorInfo( hMonitor, &monitorInfo );
				::SetWindowPos( _HWnd, HWND_TOP,
					monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.top,
					monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE );

				::ShowWindow( _HWnd, SW_MAXIMIZE );
			}
			else
			{
				// Restore all the window decorators.
				::SetWindowLong( _HWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW );

				::SetWindowPos( _HWnd, HWND_NOTOPMOST,
					_WindowRect.left,
					_WindowRect.top,
					_WindowRect.right - _WindowRect.left,
					_WindowRect.bottom - _WindowRect.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE );

				::ShowWindow( _HWnd, SW_NORMAL );
			}
		}
	}

	void DX12Core::ToggleVSync()
	{
		_VSync = !_VSync;
	}

	void DX12Core::WaitForNextFrameResources()
	{
		UINT nextFrameIndex = _FenceValue + 1;
		_FenceValue = nextFrameIndex;

		HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
		DWORD numWaitableObjects = 1;

		
		UINT64 fenceValue = _FenceValue;
		if (fenceValue != 0) // means no fence was signaled
		{
			_FenceValue = 0;
			_Fence->SetEventOnCompletion( fenceValue, _FenceEvent );
			waitableObjects[1] = _FenceEvent;
			numWaitableObjects = 2;
		}

		WaitForMultipleObjects( numWaitableObjects, waitableObjects, TRUE, INFINITE );

		return;
	}

	bool DX12Core::GetVideoMemory( DXGI_QUERY_VIDEO_MEMORY_INFO* ptrMemory ) noexcept
	{
		ComPtr<IDXGIAdapter4> ptrAdapter4;
		//if (!queryInterface) return false;
		return(SUCCEEDED( ptrAdapter4->QueryVideoMemoryInfo( NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, ptrMemory ) ));
	}

	bool DX12Core::SetVideoMemoryReservation( UINT64 reservationMemoryInBytes ) noexcept
	{
		ComPtr<IDXGIAdapter3> ptrAdapter3;
		//if (!queryInterface) return false;
		return(SUCCEEDED( ptrAdapter3->SetVideoMemoryReservation( NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, reservationMemoryInBytes ) ));
	}

}
