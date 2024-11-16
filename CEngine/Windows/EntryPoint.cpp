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
#include "WinInclude.h"
#include "../Application/Application.h"
#include <exception>
#include "../Common/CronoException.h"

extern CronoEngine::Application* CreateEngineApp();

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd )
{
	try
	{
		CronoEngine::Application* app = CreateEngineApp();
		return app->Run();
	}
	catch (const CronoEngine::CronoException& e)
	{
		MessageBox( nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION );
	}
	catch (const std::exception& e)
	{
		MessageBox( nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION );
	}
	catch (...)
	{
		MessageBox( nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION );
	}
	return -1;
}