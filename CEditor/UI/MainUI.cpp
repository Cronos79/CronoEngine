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
#include "MainUI.h"
// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_internal.h"

namespace CronoEngine
{
	void MainUI::RenderUI()
	{		
		FPSDisplay();
		RightSideBar();
		LeftSideBar();
		BottomBar();
		Test();
		TopMenuBar();
	}

	void MainUI::TopMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu( "File" ))
			{
				if (ImGui::MenuItem( "Create" ))
				{
				}
				if (ImGui::MenuItem( "Open", "Ctrl+O" ))
				{
				}
				if (ImGui::MenuItem( "Save", "Ctrl+S" ))
				{
				}
				if (ImGui::MenuItem( "Save as.." ))
				{
				}
				if (ImGui::MenuItem( "Exit" ))
				{
					PostQuitMessage( 0 );
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu( "Edit" ))
			{
				if (ImGui::MenuItem( "Copy" ))
				{
				}
				ImGui::EndMenu();
			}			
			ImGui::EndMainMenuBar();
		}
	}

	void MainUI::FPSDisplay()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin( "FPS" );
		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
		ImGui::End();
	}

	void MainUI::LeftSideBar()
	{
		HWND hwnd = FindWindow( NULL, "CronoEditor" );
		if (hwnd != NULL)
		{
			RECT rect;
			if (GetClientRect( hwnd, &rect ))
			{
				SetNextWinLoc( rect.left + 5, rect.top + 50 );
				ImGui::SetNextWindowSize( ImVec2( 300, rect.bottom - rect.top - 10 ) );
			}
		}	

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin( "LeftBar" );
		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
		ImGui::End();
	}

	void MainUI::RightSideBar()
	{
		HWND hwnd = FindWindow( NULL, "CronoEditor" );
		if (hwnd != NULL)
		{
			RECT rect;
			if (GetClientRect( hwnd, &rect ))
			{
				SetNextWinLoc( rect.right - rect.left - 290, rect.top + 50 );
				ImGui::SetNextWindowSize( ImVec2( 300, rect.bottom - rect.top - 10 ) );
			}
		}

		ImGuiIO& io = ImGui::GetIO(); (void)io;	
		ImGui::Begin( "RightBar" );
		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
		ImGui::End();
	}	

	void MainUI::BottomBar()
	{
		HWND hwnd = FindWindow( NULL, "CronoEditor" );
		if (hwnd != NULL)
		{
			RECT rect;
			if (GetClientRect( hwnd, &rect ))
			{
				SetNextWinLoc( rect.left + 307, rect.bottom - 257 );
				ImGui::SetNextWindowSize( ImVec2( rect.right - rect.left - 600, 300 ) );
			}
		}

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin( "BottomBar" );
		ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
		ImGui::End();
	}

#include <Windows.h>
	void MainUI::Test()
	{
		ImGui::Begin( "Test" );
		HWND hwnd = FindWindow( NULL, "CronoEditor" ); // Replace "Window Title" with the actual title of the window
		if (hwnd == NULL)
		{
			ImGui::Text( "Window not found!" );			
		}
		RECT rect;
		if (GetWindowRect( hwnd, &rect ))
		{
			ImGui::Text( "Top-left corner: (%d) (%d)", rect.left, rect.top );
		}
		else
		{
		ImGui::Text( "Failed to get window rectangle!" );
		}		
		ImGui::End();
	}

	void MainUI::SetNextWinLoc( int32_t x, int32_t y )
	{
		HWND hwnd = FindWindow( NULL, "CronoEditor" ); 
		if (hwnd != NULL)
		{
			RECT rect;
			if (GetWindowRect( hwnd, &rect ))
			{
				ImGui::SetNextWindowPos( ImVec2( (rect.left + x), rect.top + y ) );
			}
		}	
	}
}