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
#include <DirectXMath.h>

struct TransformComponent
{
private:
	DirectX::XMFLOAT3A m_Position{};
	DirectX::XMFLOAT3A m_Rotation{};
	DirectX::XMFLOAT3A m_Scale{};
public:
	TransformComponent()
	{
		m_Position = { 0.0f, 0.0f, 0.0f };
		m_Rotation = { 0.0f, 0.0f, 0.0f };
		m_Scale = { 1.0f, 1.0f, 1.0f };
	}

	TransformComponent( DirectX::XMFLOAT3A position, DirectX::XMFLOAT3A rotation, DirectX::XMFLOAT3A scale )
	{
		m_Position = position;
		m_Rotation = rotation;
		m_Scale = scale;
	}

	DirectX::XMFLOAT3A GetPosition()
	{
		return m_Position;
	}
	void SetPosition( float x, float y, float z )
	{
		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
	}

	DirectX::XMFLOAT3A GetRotation()
	{
		return m_Rotation;
	}
	void SetRotation( float x, float y, float z )
	{
		m_Rotation.x = x;
		m_Rotation.y = y;
		m_Rotation.z = z;
	}

	DirectX::XMFLOAT3A GetScale()
	{
		return m_Scale;
	}
	void SetScale( float x, float y, float z )
	{
		m_Scale.x = x;
		m_Scale.y = y;
		m_Scale.z = z;
	}

	DirectX::XMFLOAT4 GetRotationQuaternionFloat4()
	{
		DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw( m_Rotation.x, m_Rotation.y, m_Rotation.z );
		DirectX::XMFLOAT4 quaternionFloat4{};
		DirectX::XMStoreFloat4( &quaternionFloat4, quaternion );
		return quaternionFloat4;
	}
	DirectX::XMVECTOR GetRotationQuaternion()
	{
		return DirectX::XMQuaternionRotationRollPitchYaw( m_Rotation.x, m_Rotation.y, m_Rotation.z );
	}
};