// TemplateWindows.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "main.h"
#include <string>
#include <sstream>
#include <thread>
#include "cyImportant.h"
#include "sqlite3.h"
#include <dbghelp.h>
#include "cyVersion.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;




LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo) {
	HANDLE hFile = CreateFile(
		L"CrashDump.dmp",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	MINIDUMP_EXCEPTION_INFORMATION mei;
	mei.ThreadId		  = GetCurrentThreadId();
	mei.ClientPointers	  = TRUE;
	mei.ExceptionPointers = ExceptionInfo;
	MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		&mei,
		NULL,
		NULL);

	return EXCEPTION_EXECUTE_HANDLER;
}


mutex m;



#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;					  // current instance
WCHAR szTitle[MAX_LOADSTRING];		  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];  // the main window class name

CyMainComponent mainComp;  // Wicked Engine Main Runtime Component

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR lpCmdLine,
					  _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	BOOL dpi_success = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	assert(dpi_success);

	wiStartupArguments::Parse(lpCmdLine);  // if you wish to use command line arguments, here is a good place to parse them...

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_TEMPLATEWINDOWS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEMPLATEWINDOWS));

	// just show some basic info:
	
	mainComp.Initialize();

	
	// Reset all state that tests might have modified:
	//file.open("debug.log", ofstream::out);
	//wiBackLog::save(file);
	//mainComp.CreateScene();
	//cyBlocks cyBlocks(wiScene::GetScene());
	//cyBlocks.LoadRegBlocks();
	//cyBlocks.LoadCustomBlocks();
	// Reset camera position:
	
	
	// Scene scene;

	// wiBackLog::save(ofstream & file)
	/*
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Wait(ctx);
    */
	
	MSG msg		= {0};
	uint8_t ran = 0;
	

	chunkLoader loader;
	uint8_t numChunkThreads = wiJobSystem::GetThreadCount();
	if (wiJobSystem::GetThreadCount() > 4)
		numChunkThreads = wiJobSystem::GetThreadCount() - 2;

	loader.spawnThreads(numChunkThreads);
	DWORD lasttick = 0;
	while (msg.message != WM_QUIT)
	{
		
		
		if (wiInput::Press((wiInput::BUTTON)'M')) {
		}
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			m.lock();
			mainComp.Run();	 // run the update - render loop (mandatory)
			m.unlock();
			wiScene::Scene& scn = wiScene::GetScene();
			if (GetTickCount() - lasttick > 50 && settings::torchlights == true) {
				lasttick = GetTickCount();
				for (uint32_t i = 0; i < scn.lights.GetCount(); i++) {
					if (scn.lights[i].GetType() == wiScene::LightComponent::LightType::POINT) {
						scn.lights[i].energy = 6 + ((float)rand() / RAND_MAX) * 4;
					}
				}
			}
			
			if (wiInput::Press((wiInput::BUTTON)'H')) {
				//int msgboxID = MessageBox(NULL, L"test", L"", 0);
				//wiBackLog::Toggle();
			}
			if (wiInput::Press((wiInput::BUTTON)'F')) {
				if (mainComp.m_headLight != INVALID_ENTITY) {
					LightComponent* light = wiScene::GetScene().lights.GetComponent(mainComp.m_headLight);
					if (light->energy > 0.5f) {
						//wiBackLog::post("Light off");
						light->energy = 0.0f;
						light->SetCastShadow(false);
					} else {
						//wiBackLog::post("Light on");
						light->energy = 7.0f;
						light->SetCastShadow(true);
					}
				}
			}
			if (settings::newWorld != settings::thisWorld && mainComp.GetActivePath() == &mainComp.renderer) {
				cyImportant* world = settings::getWorld();
				world->loadWorldInfo(settings::newWorld);
				settings::thisWorld = settings::newWorld;
				TransformComponent ctransform;
				ctransform.Translate(XMFLOAT3(0.f, (float)(world->m_playerpos.z / 100) + 2.0f, 0.f));
				ctransform.RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
				ctransform.SetDirty();
				ctransform.UpdateTransform();
				wiScene::GetCamera().SetDirty();
				wiScene::GetCamera().TransformCamera(ctransform);
				wiScene::GetCamera().UpdateCamera();
				TransformComponent* transform = wiScene::GetScene().transforms.GetComponent(mainComp.m_headLight);
				transform->ClearTransform();
				transform->Translate(XMFLOAT3(0.f, (float)(world->m_playerpos.z / 100) + 2.0f, 0.f));
				transform->RotateRollPitchYaw(XMFLOAT3(-1.5, 0, 0));
				transform->SetDirty();
				transform->UpdateTransform();
				if (CyMainComponent::m_probe != wiECS::INVALID_ENTITY) {
					transform = wiScene::GetScene().transforms.GetComponent(CyMainComponent::m_probe);
					transform->ClearTransform();
					transform->Translate(XMFLOAT3(0.f, (float)(world->m_playerpos.z / 100) + 2.0f, 0.f));
					transform->SetDirty();
					transform->UpdateTransform();
				}
			}
			
		}
	}
	loader.shutdown();
	loader.m_shutdown = 2;
	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style		   = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra	   = 0;
	wcex.cbWndExtra	   = 0;
	wcex.hInstance	   = hInstance;
	wcex.hIcon		   = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEMPLATEWINDOWS));
	wcex.hCursor	   = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_TEMPLATEWINDOWS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm	   = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance;	// Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	mainComp.SetWindow(hWnd);  // assign window handle (mandatory)

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		} break;
		case WM_SIZE:
			wiEvent::FireEvent(SYSTEM_EVENT_CHANGE_RESOLUTION, lParam);
			break;
		case WM_DPICHANGED:
			wiEvent::FireEvent(SYSTEM_EVENT_CHANGE_DPI, wParam);
			break;
		case WM_CHAR:
			switch (wParam)
			{
				case VK_BACK:
					if (wiBackLog::isActive())
						wiBackLog::deletefromInput();
					wiTextInputField::DeleteFromInput();
					break;
				case VK_RETURN:
					break;
				default:
				{
					const char c = (const char)(TCHAR)wParam;
					if (wiBackLog::isActive())
					{
						wiBackLog::input(c);
					}
					wiTextInputField::AddInput(c);
				} break;
			}
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		} break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	string version = string("CyubE3dit - CyubeVR swiss army knife Version ") + cyVersion::GetVersionString();
	HBRUSH brush   = CreateSolidBrush(RGB(255, 255, 255));
	HDC hdc		   = (HDC)wParam;
	switch (message)
	{
		case WM_INITDIALOG:
			SetDlgItemTextA(hDlg, MY_ABOUT_TEXT, version.c_str());
			SetClassLongPtr(hDlg, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		case WM_ERASEBKGND:
			RECT rc;
			GetClientRect(hDlg, &rc);
			FillRect(hdc, &rc, brush); 
			return 1L;
			break;
		case WM_CTLCOLORSTATIC:
		{
			SetTextColor(hdc, RGB(0, 0, 0));
			SetBkColor(hdc, RGB(255, 255, 255));
			return (INT_PTR)brush;
			break;
		}
	}
	return (INT_PTR)FALSE;
}
