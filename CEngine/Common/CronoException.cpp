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
#include "CronoException.h"
#include <sstream>

namespace CronoEngine
{
	CronoException::CronoException( int line, const char* file ) noexcept
		:
		line( line ),
		file( file )
	{
	}

	const char* CronoException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
			<< GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* CronoException::GetType() const noexcept
	{
		return "Crono Exception";
	}

	int CronoException::GetLine() const noexcept
	{
		return line;
	}

	const std::string& CronoException::GetFile() const noexcept
	{
		return file;
	}

	std::string CronoException::GetOriginString() const noexcept
	{
		std::ostringstream oss;
		oss << "[File] " << file << std::endl
			<< "[Line] " << line;
		return oss.str();
	}

	HrException::HrException( int line, const char* file, HRESULT hr ) noexcept
		:
		CronoException( line, file ),
		hr( hr )
	{
	}

	const char* HrException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
			<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
			<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
			<< "[Description] " << GetErrorDescription() << std::endl
			<< GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* HrException::GetType() const noexcept
	{
		return "Crono Window Exception";
	}

	HRESULT HrException::GetErrorCode() const noexcept
	{
		return hr;
	}

	std::string HrException::GetErrorDescription() const noexcept
	{
		return TranslateErrorCode( hr );
	}

	std::string HrException::TranslateErrorCode( HRESULT hr ) noexcept
	{
		char* pMsgBuf = nullptr;
		// windows will allocate memory for err string and make our pointer point to it
		const DWORD nMsgLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, hr, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
		);
		// 0 string length returned indicates a failure
		if (nMsgLen == 0)
		{
			return "Unidentified error code";
		}
		// copy error string from windows-allocated buffer to std::string
		std::string errorString = pMsgBuf;
		// free windows buffer
		LocalFree( pMsgBuf );
		return errorString;
	}

	const char* NoGfxException::GetType() const noexcept
	{
		return "No Graphics Exception";
	}

}