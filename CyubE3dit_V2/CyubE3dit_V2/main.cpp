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
#include <set>
#include "cyVersion.h"
#include "cySchematic.h"
#include "SimplexNoise.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;
/*
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
*/
mutex m;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;					  // current instance
WCHAR szTitle[MAX_LOADSTRING];		  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];  // the main window class name

CyMainComponent mainComp;  // Wicked Engine Main Runtime Component

//Sort function for XMFLOAT4 containing position and distance values (for the torch sounds)
bool partSortDist(XMFLOAT4 i, XMFLOAT4 j) {
	return (i.w < j.w);
}

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

	//SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

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
		numChunkThreads = wiJobSystem::GetThreadCount() - 3;

	loader.spawnThreads(numChunkThreads);
	DWORD lasttick = 0, lasttickEmitter = 0;
	SimplexNoise noise;
	std::set<float, greater<float>> nearestTorches;
	XMFLOAT3 minPos		 = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rainbowColor(1.f,0,0);
	uint8_t fadestate	 = 0;
	XMFLOAT3 leastMinPos = XMFLOAT3(0, 0, 0);
	float minDist[CyRender::NUM_TORCHSOUNDS];
	std::vector<XMFLOAT4> neartorches;

	while (msg.message != WM_QUIT)
	{

		if (wiInput::Press(wiInput::KEYBOARD_BUTTON_ESCAPE)) {
			cySchematic::clearAllSchematics();
		}
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			m.lock();
			mainComp.Run();	 // run the update - render loop (mandatory)
			m.unlock();
			if (cySchematic::m_schematics.size() > 0) {
				if (!cySchematic::updating) {
					cySchematic::updating = true;
					wiJobSystem::context ctx;
					wiJobSystem::Execute(ctx, [](wiJobArgs args) { cySchematic::updateDirtyPreviews(); });
				}
			}
			m.lock();
			wiScene::Scene& scn = wiScene::GetScene();
			if (GetTickCount() - lasttick > 100 && settings::torchlights == true) {
				lasttick = GetTickCount();
				switch (fadestate){
					case 0:
						rainbowColor.x += 0.01f;
						rainbowColor.y -= 0.01f;
						if (rainbowColor.x > 1.f) {
							rainbowColor.x = 1;
							rainbowColor.y = 0;
							++fadestate;
						}
						break;
					case 1:
						rainbowColor.z += 0.01f;
						rainbowColor.x -= 0.01f;
						if (rainbowColor.z > 1.f) {
							rainbowColor.z = 1;
							rainbowColor.x = 0;
							++fadestate;
						}
						break;
					case 2:
						rainbowColor.y += 0.01f;
						rainbowColor.z -= 0.01f;
						if (rainbowColor.y > 1.f) {
							rainbowColor.y = 1;
							rainbowColor.z = 0;
							fadestate = 0;
						}
						break;
				}
				for (uint32_t i = 0; i < scn.lights.GetCount(); i++) {
					if (scn.lights[i].GetType() == wiScene::LightComponent::LightType::POINT) {
						if (scn.lights[i]._flags & wiScene::LightComponent::RAINBOW) {
							scn.lights[i].color = rainbowColor;
						}
						scn.lights[i].energy = 6 + ((float)rand() / RAND_MAX) * 4;
					}
				}
				//wiScene::WeatherComponent* weather = wiScene::GetScene().weathers.GetComponent(wiScene::GetScene().weathers.GetEntity(0));
				//weather->windSpeed				   = 2.5 + noise.noise((float)lasttick/100.0);
			} else if (GetTickCount() - lasttickEmitter > 100) {
				//if (settings::sound) {
					lasttickEmitter = GetTickCount();
					neartorches.clear();
					if (settings::rendermask & LAYER_EMITTER) {
						for (uint32_t i = 2; i < scn.emitters.GetCount(); i++) {
							float dist = wiMath::DistanceEstimated(wiScene::GetCamera().Eye, scn.emitters[i].center);
							if (scn.emitters[i]._flags & wiScene::wiEmittedParticle::FLAG_RAINBOW)
								wiScene::GetScene().materials.GetComponent(scn.emitters.GetEntity(i))->SetBaseColor(XMFLOAT4(rainbowColor.x, rainbowColor.y, rainbowColor.z, 0.4f));
							if (dist < 10) {
								if(settings::sound)
									neartorches.push_back(XMFLOAT4(scn.emitters[i].center.x, scn.emitters[i].center.y, scn.emitters[i].center.z, dist));
								if (scn.emitters[i].IsPaused()) {
									scn.emitters[i].SetPaused(false);
								}
							} else if (dist > 30) {
								if (!scn.emitters[i].IsPaused()) {
									scn.emitters[i].SetPaused(true);
								}
							}
							//wiScene::WeatherComponent* weather = wiScene::GetScene().weathers.GetComponent(wiScene::GetScene().weathers.GetEntity(0));
							//weather->windSpeed				   = 2.5 + noise.noise((float)lasttick/100.0);
						}
					} else if (settings::rendermask & LAYER_TORCH && settings::sound) {
						for (uint32_t i = 2; i < scn.lights.GetCount(); i++) {
							float dist = wiMath::DistanceEstimated(wiScene::GetCamera().Eye, scn.lights[i].position);
							if (dist < 10) {
								neartorches.push_back(XMFLOAT4(scn.lights[i].position.x, scn.lights[i].position.y, scn.lights[i].position.z, dist));
							}
						}
					} else {
						if (neartorches.size())
							neartorches.clear();
					}
				//}
			}
			m.unlock();

			if (neartorches.size()) {
				std::partial_sort(neartorches.begin(), neartorches.begin() + min(4, (int)neartorches.size()), neartorches.end(), partSortDist);
				wiAudio::SoundInstance3D snd3d;
				snd3d.listenerFront = wiScene::GetCamera().At;
				snd3d.listenerPos	= wiScene::GetCamera().Eye;
				snd3d.emitterRadius = 0.2;
				snd3d.emitterFront	= wiScene::GetCamera().At;
				for (uint8_t i = 0; i < CyRender::NUM_TORCHSOUNDS; i++) {
					if (i < neartorches.size()) {
						if (!mainComp.renderer.fireSoundIsPlaying[i]) {
							wiAudio::Play(&(mainComp.renderer.fireSoundinstance[i]));
							mainComp.renderer.fireSoundIsPlaying[i] = true;
							mainComp.renderer.anyfireSoundIsPlaying = true;
						}
						snd3d.emitterPos = XMFLOAT3(neartorches[i].x, neartorches[i].y, neartorches[i].z);
						wiAudio::Update3D(&(mainComp.renderer.fireSoundinstance[i]), snd3d);
					} else if (mainComp.renderer.fireSoundIsPlaying[i]) {
						wiAudio::Stop(&(mainComp.renderer.fireSoundinstance[i]));
						mainComp.renderer.fireSoundIsPlaying[i] = false;
					}
				}
			} else if (mainComp.renderer.anyfireSoundIsPlaying) {
				for (uint8_t i = 0; i < CyRender::NUM_TORCHSOUNDS; i++) {
					if (mainComp.renderer.fireSoundIsPlaying[i]) {
						wiAudio::Stop(&(mainComp.renderer.fireSoundinstance[i]));
						mainComp.renderer.fireSoundIsPlaying[i] = false;
					}
				}
				mainComp.renderer.anyfireSoundIsPlaying = false;
			}

			if (wiInput::Down(wiInput::KEYBOARD_BUTTON_LCONTROL)) {
				if (wiInput::Press((wiInput::BUTTON)'H')) {
					//int msgboxID = MessageBox(NULL, L"test", L"", 0);
					wiBackLog::Toggle();
				}
				if (wiInput::Press((wiInput::BUTTON)'T')) {
					double x, y, z;
					x = (double)wiScene::GetCamera().Eye.x * 100;
					y = (double)wiScene::GetCamera().Eye.z * 100;
					z = (double)wiScene::GetCamera().Eye.y * 100;
					settings::getWorld()->setPlayerPos(x, y, z);
				}
				if (wiInput::Press((wiInput::BUTTON)'P')) {
					wiProfiler::SetEnabled(!wiProfiler::IsEnabled());
				}
				if (wiInput::Press((wiInput::BUTTON)'I')) {
					if (mainComp.infoDisplay.active == false) {
						mainComp.infoDisplay.active	   = true;
						mainComp.infoDisplay.chunkinfo = false;
					} else if (mainComp.infoDisplay.chunkinfo == false) {
						mainComp.infoDisplay.chunkinfo = true;
					} else {
						mainComp.infoDisplay.active = false;
					}
				}
				if (wiInput::Press((wiInput::BUTTON)'S')) {
					if (cySchematic::m_schematics.size() > 0) {
						cySchematic::m_schematics[0]->m_dirty = cySchematic::DIRTY_SAVE;
					}
				}
				if (wiInput::Press((wiInput::BUTTON)'Y') || wiInput::Press((wiInput::BUTTON)'Z')) {
					if (cySchematic::m_schematics.size() > 0) {
						cySchematic::m_schematics[0]->m_dirty = cySchematic::DIRTY_ROTCC;
					}
				}
				if (wiInput::Press((wiInput::BUTTON)'X')) {
					if (cySchematic::m_schematics.size() > 0) {
						cySchematic::m_schematics[0]->m_dirty = cySchematic::DIRTY_ROTCW;
					}
				}
				if (wiInput::Press((wiInput::BUTTON)'V')) {
					mainComp.renderer.loadSchBtn.SetEnabled(false);
					wiHelper::FileDialogParams params;
					params.description = "CyubeVR schematic";
					params.extensions.push_back("cySch");
					params.OPEN;
					wiHelper::FileDialog(params, cySchematic::addSchematic);
					mainComp.renderer.loadSchBtn.SetEnabled(true);
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
			}

			if (settings::newWorld != settings::thisWorld && mainComp.GetActivePath() == &mainComp.renderer) {
				/*for (size_t i = 0;  i < cyBlocks::m_treeMeshes.size(); i++) {
					if(wiScene::GetScene().impostors.Contains(cyBlocks::m_treeMeshes[i]) == false)
						wiScene::GetScene().impostors.Create(cyBlocks::m_treeMeshes[i]).swapInDistance = settings::viewDist * 4;
				}*/
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
				transform = wiScene::GetScene().transforms.GetComponent(mainComp.m_posLight);
				transform->ClearTransform();
				transform->Translate(XMFLOAT3(0.f, (float)(world->m_playerpos.z / 100) + 8.0f, 0.f));
				transform->RotateRollPitchYaw(XMFLOAT3(-PI, 0, 0));
				transform->SetDirty();
				transform->UpdateTransform();
				transform = wiScene::GetScene().transforms.GetComponent(mainComp.m_dust);
				transform->ClearTransform();
				transform->Scale(XMFLOAT3(10, 7, 10));
				transform->Translate(XMFLOAT3(0.f, (float)(world->m_playerpos.z / 100) + 4.0f, 0.f));
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
	settings::save();
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
		case WM_KILLFOCUS:
			mainComp.is_window_active = false;
			break;
		case WM_SETFOCUS:
			mainComp.is_window_active = true;
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
