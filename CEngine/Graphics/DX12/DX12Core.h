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
#include "Windows/WinInclude.h"
#include "DX12CommonIncludes.h"

namespace CronoEngine::Graphics
{
	class DX12Core
	{
	public:
		DX12Core( HWND hWnd, uint32_t width, uint32_t height );
		~DX12Core();

		void Init();
		void Shutdown();
		void Resize( uint32_t width, uint32_t height );
		void BeginFrame();
		void EndFrame();
		void SetFullscreen();
		void SetFullscreen( bool fullscreen );
		void ToggleVSync();
		bool GetVideoMemory( DXGI_QUERY_VIDEO_MEMORY_INFO* ptrMemory ) noexcept;
		bool SetVideoMemoryReservation( UINT64 reservationMemoryInBytes ) noexcept;
	private:
		void EnableDebugLayer();
		ComPtr<IDXGIAdapter4> GetAdapter( bool useWarp );
		ComPtr<ID3D12Device14> CreateDevice( ComPtr<IDXGIAdapter4> adapter );
		ComPtr<ID3D12CommandQueue> CreateCommandQueue( ComPtr<ID3D12Device14> device, D3D12_COMMAND_LIST_TYPE type );
		bool CheckTearingSupport();
		ComPtr<IDXGISwapChain4> CreateSwapChain( HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue,
			uint32_t width, uint32_t height, uint32_t bufferCount );
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap( ComPtr<ID3D12Device14> device,
			D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors );
		void UpdateRenderTargetViews( ComPtr<ID3D12Device14> device,
			ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap );
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator( ComPtr<ID3D12Device14> device,
			D3D12_COMMAND_LIST_TYPE type );
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList( ComPtr<ID3D12Device14> device,
			ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type );
		ComPtr<ID3D12Fence> CreateFence( ComPtr<ID3D12Device14> device );
		HANDLE CreateEventHandle();
		uint64_t Signal( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
			uint64_t& fenceValue );
		void WaitForFenceValue( ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent,
			std::chrono::milliseconds duration = std::chrono::milliseconds::max() );
		void Flush( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence,
			uint64_t& fenceValue, HANDLE fenceEvent );
		
	private:
		// Window handle.
		HWND _HWnd;
		uint32_t _Width = 1280;
		uint32_t _Height = 720;
		// Window rectangle (used to toggle fullscreen state).
		RECT _WindowRect;
		bool _rayTracingSupport = false;
		// The number of swap chain back buffers.
#define NumFrames 3
		// Use WARP adapter
		bool _UseWarp = false;
		// Set to true once the DX12 objects have been initialized.
		bool _IsInitialized = false;
		// DirectX 12 Objects
		ComPtr<ID3D12Device14> _Device;
		ComPtr<ID3D12CommandQueue> _CommandQueue;
		ComPtr<IDXGISwapChain4> _SwapChain;
		ComPtr<ID3D12Resource> _BackBuffers[NumFrames];
		ComPtr<ID3D12GraphicsCommandList> _CommandList;
		ComPtr<ID3D12CommandAllocator> _CommandAllocators[NumFrames];
		ComPtr<ID3D12DescriptorHeap> _SRVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> _RTVDescriptorHeap;
		UINT _RTVDescriptorSize;
		UINT _CurrentBackBufferIndex;
		// Synchronization objects
		ComPtr<ID3D12Fence> _Fence;
		uint64_t _FenceValue = 0;
		uint64_t _FrameFenceValues[NumFrames] = {};
		HANDLE _FenceEvent;
		// By default, enable V-Sync.
		// Can be toggled with the V key.
		bool _VSync = true;
		bool _TearingSupported = false;
		// By default, use windowed mode.
		// Can be toggled with the Alt+Enter or F11
		bool _Fullscreen = false;

		// IMGUI TEST
		
		void WaitForNextFrameResources();
		bool g_SwapChainOccluded = false;
		bool show_demo_window = true;
		bool show_another_window;
		ImVec4 clear_color = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
		HANDLE g_hSwapChainWaitableObject = nullptr;
	};
}

