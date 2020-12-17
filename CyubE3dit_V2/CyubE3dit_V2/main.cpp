// TemplateWindows.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "main.h"
#include <string>
#include <sstream>
#include <thread>
#include "cyImportant.h"
#include "sqlite3.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;
cyImportant* world = settings::getWorld();
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

	// TODO: Place code here.

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
	mainComp.infoDisplay.active		= true;
	mainComp.infoDisplay.watermark	= true;
	mainComp.infoDisplay.resolution = true;
	mainComp.infoDisplay.fpsinfo	= true;
	mainComp.Initialize();

	// Reset all state that tests might have modified:
	ofstream file;
	//file.open("debug.log", ofstream::out);
	//wiBackLog::save(file);
	mainComp.CreateScene();
	cyBlocks cyBlocks(wiScene::GetScene());
	cyBlocks.LoadRegBlocks();
	cyBlocks.LoadCustomBlocks();
	// Reset camera position:
	TransformComponent transform;
	transform.Translate(XMFLOAT3(0.f, 500.f, 0.f));
	transform.RotateRollPitchYaw(XMFLOAT3(1.5, 0, 0));
	transform.SetDirty();
	transform.UpdateTransform();
	wiScene::GetCamera().SetDirty();
	wiScene::GetCamera().TransformCamera(transform);
	wiScene::GetCamera().UpdateCamera();
	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
	wiRenderer::SetTemporalAAEnabled(true);
	// Scene scene;

	// wiBackLog::save(ofstream & file)
	/*
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Execute(ctx, [](wiJobArgs args) { wiHelper::Spin(100); });
    wiJobSystem::Wait(ctx);
    */
	meshGen mGen;
	Scene& scene2 = wiScene::GetScene();
	/*
	Entity materialID = scene2.Entity_CreateMaterial("terrainMaterial");
	MaterialComponent* material = scene2.materials.GetComponent(materialID);
	material->baseColorMap		= wiResourceManager::Load("images/glass.jpg");
	material->baseColorMapName	= "images/2floor.jpg";
	//material->SetEmissiveColor(XMFLOAT4(1, 1, 1, 0.1));
	material->SetDirty();
	wiScene::MeshComponent* mesh = mGen.AddMesh(wiScene::GetScene(),cyBlocks::m_regBlockMats[2][0]);
	mGen.AddFaceTop(mesh, -0.5, 1, 1);
	mGen.AddFaceBottom(mesh, -0.5, 1, 1);
	mGen.AddFaceLeft(mesh, -0.5, 1, 1);
	mGen.AddFaceRight(mesh, -0.5, 1, 1);
	mGen.AddFaceFront(mesh, -0.5, 1, 1);
	mGen.AddFaceBack(mesh, -0.5, 1, 1);
	meshGen::newMaterial(mesh, cyBlocks::m_regBlockMats[1][0]);
	mGen.AddFaceTop(mesh, 1, 0, 2);
	meshGen::newMaterial(mesh, cyBlocks::m_regBlockMats[1][1]);
	mGen.AddFaceBottom(mesh, 1, 0, 2);
	meshGen::newMaterial(mesh, cyBlocks::m_regBlockMats[1][2]);
	mGen.AddFaceLeft(mesh, 1, 0, 2);
	mGen.AddFaceRight(mesh, 1, 0, 2);
	mGen.AddFaceFront(mesh, 1, 0, 2);
	mGen.AddFaceBack(mesh, 1, 0, 2);
	//mesh->ComputeNormals(MeshComponent::COMPUTE_NORMALS_HARD);
	mesh->SetDynamic(false);
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->CreateRenderData();*/

	/*
	if (16 + world.getChunkID(world.m_playerpos.x / 100, world.m_playerpos.y / 100, &chunkID))
		chunkL.loadChunk(db, chunkID, true);
	if (-16 + world.getChunkID(world.m_playerpos.x / 100, world.m_playerpos.y / 100, &chunkID))
		chunkR.loadChunk(db, chunkID, true);
	if (world.getChunkID(world.m_playerpos.x / 100, 16 + world.m_playerpos.y / 100, &chunkID))
		chunkU.loadChunk(db, chunkID, true);
	if (world.getChunkID(world.m_playerpos.x / 100, -16 + world.m_playerpos.y / 100, &chunkID))
		chunkD.loadChunk(db, chunkID,true);
*/

	/*MeshComponent* mesh;
	mesh				  = meshGen::AddMesh(wiScene::GetScene(),cyBlocks::m_regBlockMats[0][0]);
	for (float x = 0; x < 4; x = x + 0.5) {
		for (float y= 0; y< 4; y = y+ 0.5) {
			meshGen::AddFaceTop(mesh, y, x, 0, true);
		}
	}

    meshGen::AddFaceBack(mesh, 0, 0, 0, false);
	meshGen::AddFaceFront(mesh, 0, 0, 0, false);
	meshGen::AddFaceLeft(mesh, 0, 0, 0, false);
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->CreateRenderData();*/
	wiRenderer::SetTemporalAAEnabled(true);

	/*
	std::stringstream ss("");
	ss << "Simple loop took " << time << " milliseconds" << std::endl;
	static wiSpriteFont font;
	font				= wiSpriteFont(ss.str());
	font.params.posX	= wiRenderer::GetDevice()->GetScreenWidth() / 2;
	font.params.posY	= wiRenderer::GetDevice()->GetScreenHeight() / 2;
	font.params.h_align = WIFALIGN_CENTER;
	font.params.v_align = WIFALIGN_CENTER;
	font.params.size	= 24;
	mainComp.renderer.AddFont(&font);
	*/
	MSG msg		= {0};
	uint8_t ran = 0;
	mainComp.Run();	 // run the update - render loop (mandatory)
	
	world->loadWorldInfo(L"My Great World - Kopie(cleaned)");
	Sleep(1000);
	chunkLoader loader;
	loader.spawnThreads(wiJobSystem::GetThreadCount());

	while (msg.message != WM_QUIT)
	{
		if (wiInput::Press((wiInput::BUTTON)'T')) {
			//int msgboxID = MessageBox(NULL, L"test", L"", 0);
			wiBackLog::Toggle();
		}

		if (wiInput::Press((wiInput::BUTTON)'M')) {
		}
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			m.lock();
			mainComp.Run();	 // run the update - render loop (mandatory)
			m.unlock();
		}
	}

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
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
