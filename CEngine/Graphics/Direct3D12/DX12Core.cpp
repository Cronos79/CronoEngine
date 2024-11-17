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


using namespace Microsoft::WRL;

namespace CronoEngine::Graphics::CD3D12::Core
{
	namespace
	{
		void FailedInit( HRESULT hr )
		{
			Shutdown();
			throw CHWND_EXCEPT( hr );
		}

		class D3D12_Command
		{
		public:
			D3D12_Command() = default;
			DISABLE_COPY_AND_MOVE( D3D12_Command );
			D3D12_Command( ID3D12Device14* const device, D3D12_COMMAND_LIST_TYPE type )
			{
				HRESULT hr{ S_OK };
				D3D12_COMMAND_QUEUE_DESC desc{};
				desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				desc.NodeMask = 0;
				desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				desc.Type = type;
				hr = device->CreateCommandQueue( &desc, IID_PPV_ARGS( &_cmdQueue ) );
				if (FAILED( hr )) FailedInit( hr );
				NAME_D3D12_OBJECT( _cmdQueue, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command Queue");

				for (int32_t i{ 0 }; i < FrameBufferCount; ++i)
				{
					commandFrame& frame{ _cmdFrames[i] };
					hr = device->CreateCommandAllocator( type, IID_PPV_ARGS( &frame.cmdAllocator ) );
					if (FAILED( hr )) FailedInit( hr );
					NAME_D3D12_OBJECT_Indexed( frame.cmdAllocator, i, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Allocator" :
						type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Allocator" : L"Command Allocator" );
				}
				hr = device->CreateCommandList( 0, type, _cmdFrames[0].cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
				if (FAILED( hr )) FailedInit( hr );
				hr = _cmdList->Close();
				if (FAILED( hr )) FailedInit( hr );
				NAME_D3D12_OBJECT( _cmdList, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command List" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command List" : L"Command List" );

				hr = device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &_fence ) );
				if (FAILED( hr )) FailedInit( hr );
				NAME_D3D12_OBJECT( _fence, L"D3D12 Fence" );

				_fenceEvent = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
				assert( _fenceEvent );

				return;
			}

			~D3D12_Command()
			{
				assert( !_cmdList && !_cmdQueue );
			}

			void BeginFrame()
			{
				commandFrame& frame{ _cmdFrames[_frameIndex] };
				frame.Wait(_fenceEvent, _fence);
				HRESULT hr = frame.cmdAllocator->Reset();
				if (FAILED( hr )) FailedInit( hr );
				hr = _cmdList->Reset( frame.cmdAllocator, nullptr );
				if (FAILED( hr )) FailedInit( hr );

			}

			void EndFrame()
			{
				HRESULT hr = _cmdList->Close();
				if (FAILED( hr )) FailedInit( hr );
				ID3D12CommandList* const cmdLists[]{ _cmdList };
				_cmdQueue->ExecuteCommandLists( _countof( cmdLists ), &cmdLists[0] );

				int64_t& fenceValue{ _fenceValue };
				++fenceValue;
				commandFrame& frame{ _cmdFrames[_frameIndex] };
				frame.fenceValue = fenceValue;
				
				_cmdQueue->Signal( _fence, fenceValue );

				_frameIndex = (_frameIndex + 1) % FrameBufferCount;
			}

			void Flush()
			{
				for (int32_t i{ 0 }; i < FrameBufferCount; ++i)
				{
					_cmdFrames[i].Wait( _fenceEvent, _fence );
				}
			}

			void Release()
			{
				Flush();
				Core::Release( _fence );
				_fenceValue = 0;
				CloseHandle( _fenceEvent );
				_fenceEvent = nullptr;
				Core::Release( _cmdQueue );
				Core::Release( _cmdList );
				for (int32_t i{ 0 }; i < FrameBufferCount; ++i)
				{
					_cmdFrames[i].Release();
				}
			}

			constexpr ID3D12CommandQueue* const CommandQueue() const
			{
				return _cmdQueue;
			}

			constexpr ID3D12GraphicsCommandList10* const CommandList() const
			{
				return _cmdList;
			}

			constexpr int32_t FrameIndex() const
			{
				return _frameIndex;
			}

		private:
			struct commandFrame
			{
				ID3D12CommandAllocator* cmdAllocator{ nullptr };
				int64_t fenceValue{ 0 };

				void Wait(HANDLE fenceEvent, ID3D12Fence* fence)
				{
					assert( fence && fenceEvent );
					if (fence->GetCompletedValue() - fenceValue)
					{
						HRESULT hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
						if (FAILED( hr )) FailedInit( hr );
						// #NOTE: Wait until the gpu has reached the fence value and has finished executing.
						WaitForSingleObject(fenceEvent, INFINITE);
					}
				}

				void Release()
				{
					Core::Release( cmdAllocator );
				}
			};
			ID3D12CommandQueue* _cmdQueue{ nullptr };
			ID3D12GraphicsCommandList10* _cmdList{ nullptr };
			commandFrame _cmdFrames[FrameBufferCount]{};
			int32_t _frameIndex{ 0 };
			ID3D12Fence* _fence{ nullptr };
			int64_t _fenceValue{ 0 };
			HANDLE _fenceEvent{ nullptr };
		};


		ID3D12Device14* MainDevice{ nullptr };
		IDXGIFactory7* DxgiFactory{ nullptr };
		D3D12_Command* gfxCommand;

		constexpr D3D_FEATURE_LEVEL MinimumFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };	

		IDXGIAdapter4* DetermineMainAdapter()
		{
			IDXGIAdapter4* adapter{ nullptr };
			for (int32_t i{ 0 }; DxgiFactory->EnumAdapterByGpuPreference( i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS( &adapter )) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				if (SUCCEEDED( D3D12CreateDevice( adapter, MinimumFeatureLevel, __uuidof(ID3D12Device), nullptr ) ))
				{
					return adapter;
				}
				Release( adapter );
			}
			return nullptr;
		}

		D3D_FEATURE_LEVEL GetMaxFeatureLevel( IDXGIAdapter4* adapter )
		{
			constexpr D3D_FEATURE_LEVEL featureLevels[5]
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_2,
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
			featureLevelInfo.NumFeatureLevels = _countof( featureLevels );
			featureLevelInfo.pFeatureLevelsRequested = featureLevels;

			ComPtr<ID3D12Device> device;
			HRESULT hr = D3D12CreateDevice( adapter, MinimumFeatureLevel, IID_PPV_ARGS( &device ) );
			if (FAILED( hr )) FailedInit( hr );
			hr = device->CheckFeatureSupport( D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof( featureLevelInfo ) );
			if (FAILED( hr )) FailedInit( hr );
			return featureLevelInfo.MaxSupportedFeatureLevel;
		}
	}

	bool Initialize()
	{
		if (MainDevice) Shutdown();

		int32_t dxgiFactoryFlags{ 0 };
		HRESULT hr{ S_OK };

#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug6> debugInterface;
			hr = D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) );
			if (FAILED( hr )) FailedInit( hr );
			debugInterface->EnableDebugLayer();			
		}
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

		
		hr = CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &DxgiFactory ) );
		if (FAILED( hr )) FailedInit( hr );

		ComPtr<IDXGIAdapter4> mainAdapter;
		mainAdapter.Attach( DetermineMainAdapter() );
		if (!mainAdapter) FailedInit(-1);

		D3D_FEATURE_LEVEL maxFeatureLevel{ GetMaxFeatureLevel( mainAdapter.Get() ) };
		assert( maxFeatureLevel >= MinimumFeatureLevel );
		if (maxFeatureLevel < MinimumFeatureLevel) FailedInit( -1 );

		hr = D3D12CreateDevice( mainAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS( &MainDevice ) );
		if (FAILED( hr )) FailedInit( hr );

		new(&gfxCommand) D3D12_Command( MainDevice, D3D12_COMMAND_LIST_TYPE_DIRECT );
		if(!gfxCommand->CommandQueue()) FailedInit( -1 );

		NAME_D3D12_OBJECT( MainDevice, L"MAIN D3D12 DEVICE" );

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue1> infoQueue;
			hr = MainDevice->QueryInterface( IID_PPV_ARGS( &infoQueue ) );
			if (FAILED( hr )) FailedInit( hr );

			infoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, true );
			infoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, true );
			infoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, true );
		}
#endif // _DEBUG

		return true;
	}

	void Shutdown()
	{
		gfxCommand->Release();
		Release( DxgiFactory );
#ifdef _DEBUG
		{
			HRESULT hr{ S_OK };
			{
				ComPtr<ID3D12InfoQueue1> infoQueue;
				hr = MainDevice->QueryInterface( IID_PPV_ARGS( &infoQueue ) );
				if (FAILED( hr )) FailedInit( hr );

				infoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, false );
				infoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, false );
				infoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, false );
			}
			ComPtr<ID3D12DebugDevice2> debugDevice;
			hr = MainDevice->QueryInterface( IID_PPV_ARGS( &debugDevice ) );
			if (FAILED( hr )) FailedInit( hr );
			Release( MainDevice );
			hr = debugDevice->ReportLiveDeviceObjects(
				D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL );
			if (FAILED( hr )) FailedInit( hr );
		}
#endif // _DEBUG
		Release( MainDevice );
	}

	void Render()
	{
		gfxCommand->BeginFrame();
		ID3D12GraphicsCommandList10* cmdList{ gfxCommand->CommandList() };
		// Record commands
		// ...
		// Done recording commands
		// Signal and increment fence value
		gfxCommand->EndFrame();
	}




}