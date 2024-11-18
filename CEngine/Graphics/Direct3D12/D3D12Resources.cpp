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
#include "D3D12Resources.h"
#include "DX12Core.h"

namespace CronoEngine::Graphics::CD3D12
{
	/********************************** DESCRIPTOR HEAP **********************************/
	bool DescriptorHeap::Initialize( int32_t capacity, bool isShaderVisible )
	{
		std::lock_guard lock{ _mutex };
		assert( capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2 );
		assert( !(_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE) );
		if (_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || _type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			isShaderVisible = false;
		}
		Release();
		HRESULT hr{ S_OK };
		ID3D12Device14* const device{ Core::Device() };

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = _type;
		desc.NodeMask = 0;
		device->CreateDescriptorHeap( &desc, IID_PPV_ARGS(&_heap) );
		if (FAILED( hr ))
		{
			Release();
			return false;			
		}

		_freeHandles = std::move( std::make_unique<int32_t[]>( capacity ) );
		_capacity = capacity;
		_size = 0;
		for (int32_t i{ 0 }; i < capacity; ++i) _freeHandles[1] = i;
#ifdef _DEBUG
		for (int32_t i{ 0 }; i < FrameBufferCount; ++i) assert(_deferredFreeIndices[i].empty());
#endif

		_descriptorSize = device->GetDescriptorHandleIncrementSize( _type );
		_cpuStart = _heap->GetCPUDescriptorHandleForHeapStart();
		_gpuStart = isShaderVisible ? _heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	DescriptorHandle DescriptorHeap::Allocate()
	{
		std::lock_guard lock{ _mutex };
		assert( _heap );
		assert( _size < _capacity );
		const int32_t index{ _freeHandles[_size] };
		const int32_t offset{ index * _descriptorSize };
		++_size;

		DescriptorHandle handle{};
		handle.cpu.ptr = _cpuStart.ptr + offset;
		if (isShaderVisible())
		{
			handle.gpu.ptr = _gpuStart.ptr + offset;
		}
#ifdef _DEBUG
		handle.container = this;
		handle.index = index;
#endif // _DEBUG
		return handle;
	}

	void DescriptorHeap::Free( DescriptorHandle& handle )
	{
		if (!handle.isValid()) return;
		std::lock_guard lock{ _mutex };
		assert( _heap && _size );
		assert( handle.cpu.ptr >= _cpuStart.ptr );
		const int32_t index{ (int32_t)(handle.cpu.ptr - _cpuStart.ptr) / _descriptorSize };
		assert( handle.index == index );

		const int32_t frameIndex{ Core::CurrentFrameIndex() };
		_deferredFreeIndices[frameIndex].push_back( index );
		Core::SetDeferredReleasesFlag();
		handle = {};
	}

	void DescriptorHeap::Release()
	{
		assert( !_size );
		Core::DeferredRelease( _heap );
	}

	void DescriptorHeap::ProcessDeferredFree( int32_t frameIdx )
	{
		std::lock_guard lock{ _mutex };
		assert( frameIdx < FrameBufferCount );

		std::vector<int32_t>& indices{ _deferredFreeIndices[frameIdx] };
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				--_size;
				_freeHandles[_size] = index;
			}
			indices.clear();
		}
	}

}