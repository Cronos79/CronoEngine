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

namespace CronoEngine::Graphics::CD3D12
{
	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu{};

		constexpr bool isValid() const
		{
			return cpu.ptr != 0;
		}

		constexpr bool isShaderVisible() const
		{
			return gpu.ptr != 0;
		}
#ifdef _DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* container{ nullptr };
		int32_t index{ -1 };
#endif // _DEBUG

	};
	class DescriptorHeap
	{
	public:
		DescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE type ) : _type( type ) {	}
		DISABLE_COPY_AND_MOVE( DescriptorHeap );
		~DescriptorHeap() {	/*assert(!_heap);*/ }

		bool Initialize( int32_t capacity, bool isShaderVisible );
		void ProcessDeferredFree( int32_t frameIdx );
		void Release();

		constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() const {	return _type; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE cpuStart() const { return _cpuStart; }
		constexpr D3D12_GPU_DESCRIPTOR_HANDLE gpuStart() const { return _gpuStart; }
		constexpr ID3D12DescriptorHeap* const heap() const { return _heap; }
		constexpr int32_t capacity() const { return _capacity; }
		constexpr int32_t size() const { return _size; }
		constexpr int32_t descriptorSize() const { return _descriptorSize; }
		constexpr bool isShaderVisible() const { return _gpuStart.ptr != 0; }

		[[nodiscard]] DescriptorHandle Allocate();
		void Free( DescriptorHandle& handle );
	private:
		ID3D12DescriptorHeap* _heap{ nullptr };
		D3D12_CPU_DESCRIPTOR_HANDLE _cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE _gpuStart{};
		std::unique_ptr<int32_t[]> _freeHandles{};
		int32_t _capacity{ 0 };
		int32_t _size{ 0 };
		int32_t _descriptorSize{ 0 };
		const D3D12_DESCRIPTOR_HEAP_TYPE _type{};
		std::mutex _mutex{};
		std::vector<int32_t> _deferredFreeIndices[FrameBufferCount]{};
	};
}