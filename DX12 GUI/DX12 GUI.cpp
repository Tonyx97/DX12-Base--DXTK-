//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "DX12.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	static bool Debug = true;
	if (Debug)
	{
		AllocConsole();
		freopen("CONOUT$", "w+", stdout);
		ShowWindow(GetConsoleWindow(), SW_SHOW);
		system("color A");
	}
	return DX12Manager::Initialize(300, 300, 1280, 720);
}
