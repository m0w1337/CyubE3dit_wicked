#include "stdafx.h"
#include "CyRender.h"

#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <cmath>

using namespace wiECS;
using namespace wiScene;

void CyMainComponent::Initialize()
{
	MainComponent::Initialize();

	infoDisplay.active = true;
	infoDisplay.watermark = true;
	infoDisplay.fpsinfo = true;
	infoDisplay.resolution = true;
	infoDisplay.heap_allocation_counter = true;

	renderer.Load();
	//pathRenderer.Load();
	ActivatePath(&renderer);
}

void CyMainComponent::CreateScene(void) {
	Scene &scene = wiScene::GetScene();
	wiRenderer::GetDevice()->SetVSyncEnabled(true);
	wiRenderer::SetToDrawGridHelper(false);
	wiRenderer::SetTemporalAAEnabled(false);
	wiRenderer::ClearWorld(wiScene::GetScene());
	wiScene::GetScene().weather = WeatherComponent();
	if (wiLua::GetLuaState() != nullptr) {
		wiLua::KillProcesses();
	}
	// Add some nice weather, not just black:
	auto& weather = scene.weathers.Create(CreateEntity());
	weather.ambient = XMFLOAT3(0.6f, 0.5f, 0.5f);
	weather.fogStart = 200;
	weather.fogEnd = 800;
	weather.fogHeight = 10;

	weather.horizon = XMFLOAT3(0.4f, 0.4f, 0.9f);
	weather.zenith = XMFLOAT3(0.5f, 0.7f, 0.9f);
	weather.cloudiness = 0.50f;
	Entity LightEnt = scene.Entity_CreateLight("Sunlight", XMFLOAT3(1, 0.5, -1), XMFLOAT3(1.0, 1., 1.f), 40, 1000);
	LightComponent* light = scene.lights.GetComponent(LightEnt);
	light->SetType(LightComponent::LightType::DIRECTIONAL);
	TransformComponent& transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(0.5f, 0, 0.6f));
	transform.SetDirty();
	transform.UpdateTransform();
	light->SetCastShadow(true);
	light->lensFlareRimTextures.resize(6);
	light->lensFlareNames.resize(6);
	//light->lensFlareRimTextures[0] = wiResourceManager::Load("images/flare0.jpg");
	//light->lensFlareNames[0] = "flare0";
	light->lensFlareRimTextures[1] = wiResourceManager::Load("images/flare1.jpg");
	light->lensFlareNames[1] = "flare1";
	light->lensFlareRimTextures[2] = wiResourceManager::Load("images/flare2.jpg");
	light->lensFlareNames[2] = "flare2";
	light->lensFlareRimTextures[3] = wiResourceManager::Load("images/flare3.jpg");
	light->lensFlareNames[3] = "flare3";
	light->lensFlareRimTextures[4] = wiResourceManager::Load("images/flare4.jpg");
	light->lensFlareNames[4] = "flare4";
	light->lensFlareRimTextures[5] = wiResourceManager::Load("images/flare5.jpg");
	light->lensFlareNames[5] = "flare5";
	Entity fillLight = scene.Entity_CreateLight("filllight", XMFLOAT3(100, 300, -100), XMFLOAT3(1.0f, 1., 1.f), 5, 1000);
	//LightComponent* lights;
	//for (int i = -5;i < 5;i++) {
	//	for (int y = -5; y < 5; y++) {
	//		lights = scene.lights.GetComponent(scene.Entity_CreateLight("lights"+i+y, XMFLOAT3(i*3, 5, y*3), XMFLOAT3(wiRandom::getRandom(4) / 4.0f, wiRandom::getRandom(4) / 4.0f, wiRandom::getRandom(4) / 4.0f), 1.5, 100));
	//		lights->SetType(LightComponent::LightType::SPOT);
	//	}
	//	//lights->SetVisualizerEnabled(true);
	//}
	meshGen mGen;


	//wiScene::GetScene().Merge(scene); // add lodaded scene to global scene
	/*
	Scene& scene2 = wiScene::GetScene();

	Entity materialID = scene2.Entity_CreateMaterial("terrainMaterial");


	MaterialComponent* material = scene2.materials.GetComponent(materialID);
	material->baseColorMap = wiResourceManager::Load("images/glass.jpg");
	material->userBlendMode = BLENDMODE_ALPHA;
	material->baseColorMapName = "images/glass.jpg";
	material->SetRefractionIndex(0.07f);
	material->SetBaseColor(XMFLOAT4(0, 0, 0, 0.01f));
	material->SetMetalness(0.2f);
	material->SetRoughness(0.03f);
	material->SetCastShadow(false);
	material->SetUVSet_DisplacementMap(0);
	material->SetUVSet_NormalMap(0);
	material->SetReflectance(0.0f);
	material->SetOcclusionEnabled_Primary(false);
	material->SetOcclusionEnabled_Secondary(false);
	//material->SetEmissiveColor(XMFLOAT4(1, 1, 1, 0.1));
	material->SetDirty();

	Entity material2ID = scene2.Entity_CreateMaterial("terrainMaterial2");
	material = scene2.materials.GetComponent(material2ID);
	material->baseColorMapName = "images/grass.jpg";
	material->normalMapName = "images/grass_n.jpg";
	material->occlusionMapName  = "images/grass_o.jpg";
	material->SetParallaxOcclusionMapping(2);
	material->SetReflectance(0.0);
	material->SetMetalness(0.2f);
	material->SetRoughness(1.f);
	material->SetNormalMapStrength(2.0);
	material->SetDirty();


	wiScene::MeshComponent* mesh = mGen.AddMesh(scene2, material2ID);
	
	wiJobSystem::context ctx;

	for (float x = -50; x < 50; x = x + 0.5f) {
		for (float y = -50; y < 50; y = y + 0.5f) {
			mGen.AddFaceTop(mesh, x, y, 0);
		}
	}
	mesh->SetDynamic(false);
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->CreateRenderData();*/
	/*
	mGen.AddFaceTop(mesh, 0, 1, 1);
	mGen.AddFaceBottom(mesh, 0, 1, 1);
	mGen.AddFaceLeft(mesh, 0, 1, 1);
	mGen.AddFaceRight(mesh, 0, 1, 1);
	mGen.AddFaceFront(mesh, 0, 1, 1);
	mGen.AddFaceBack(mesh, 0, 1, 1);

	mGen.AddFaceTop(mesh, 2, 1, 2);
	mGen.AddFaceBottom(mesh, 2, 1, 2);
	mGen.AddFaceLeft(mesh, 2, 1, 2);
	mGen.AddFaceRight(mesh, 2, 1, 2);
	mGen.AddFaceFront(mesh, 2, 1, 2);
	mGen.AddFaceBack(mesh, 2, 1, 2);
	
	mesh = mGen.AddMesh(scene2, materialID);
	mGen.AddFaceTop(mesh, -0.5, 0, 1);
	mGen.AddFaceBottom(mesh, -0.5, 0, 1);
	mGen.AddFaceLeft(mesh, -0.5, 0, 1);
	mGen.AddFaceRight(mesh, -0.5, 0, 1);
	mGen.AddFaceFront(mesh, -0.5, 0, 1);
	mGen.AddFaceBack(mesh, -0.5, 0, 1);

	mGen.AddFaceTop(mesh, 0, 0, 2);
	mGen.AddFaceBottom(mesh, 0, 0, 2);
	mGen.AddFaceLeft(mesh, 0, 0, 2);
	mGen.AddFaceRight(mesh, 0, 0, 2);
	mGen.AddFaceFront(mesh, 0, 0, 2);
	mGen.AddFaceBack(mesh, 0, 0, 2);
	//mesh->ComputeNormals(MeshComponent::COMPUTE_NORMALS_HARD);
	mesh->SetDynamic(false);
	mesh->CreateRenderData();*/
}


void CyRender::ResizeLayout()
{
	RenderPath3D::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
}


void CyRender::Load()
{
	setSSREnabled(true);
	setBloomEnabled(false);
	setReflectionsEnabled(false);
	setColorGradingEnabled(false);
	wiPhysicsEngine::SetEnabled(false);
	//setLightShaftsEnabled(true);
	setShadowsEnabled(true);
	setVolumetricCloudsEnabled(false);
	//setRaytracedReflectionsEnabled(true);
	setFXAAEnabled(true);
	setEyeAdaptionEnabled(true);
	RenderPath3D::Load();
}

void CyRender::Update(float dt)
{

	CameraComponent& camera = wiRenderer::GetCamera();
	// Camera control:
	static XMFLOAT4 originalMouse = XMFLOAT4(0, 0, 0, 0);
	static float Accel = 0.0;
	static bool camControlStart = true;
	if (camControlStart)
	{
		originalMouse = wiInput::GetPointer();
	}

	XMFLOAT4 currentMouse = wiInput::GetPointer();
	float xDif = 0, yDif = 0;

	if (wiInput::Down(wiInput::MOUSE_BUTTON_LEFT))
	{
		camControlStart = false;
#if 1
		// Mouse delta from previous frame:
		xDif = currentMouse.x - originalMouse.x;
		yDif = currentMouse.y - originalMouse.y;
#else
		// Mouse delta from hardware read:
		xDif = wiInput::GetMouseState().delta_position.x;
		yDif = wiInput::GetMouseState().delta_position.y;
#endif
		xDif = 0.1f * xDif * (1.0f / 60.0f);
		yDif = 0.1f * yDif * (1.0f / 60.0f);
		wiInput::SetPointer(originalMouse);
		wiInput::HidePointer(true);
	}
	else
	{
		camControlStart = true;
		wiInput::HidePointer(false);
	}

	const float buttonrotSpeed = 2.0f / 60.0f;
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_LEFT))
	{
		xDif -= buttonrotSpeed;
	}
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_RIGHT))
	{
		xDif += buttonrotSpeed;
	}
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_UP))
	{
		yDif -= buttonrotSpeed;
	}
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_DOWN))
	{
		yDif += buttonrotSpeed;
	}

	const XMFLOAT4 leftStick = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_L, 0);
	const XMFLOAT4 rightStick = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_R, 0);
	const XMFLOAT4 rightTrigger = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_TRIGGER_R, 0);

	const float jostickrotspeed = 0.05f;
	xDif += rightStick.x * jostickrotspeed;
	yDif += rightStick.y * jostickrotspeed;

	xDif *= 1.0;
	yDif *= 1.0;
	if (Accel < 4) {
		Accel += dt * 2;
	}
	const float MoveSpeed = 4 * Accel;

	// FPS Camera
	const float clampedDT = dt; // if dt > 100 millisec, don't allow the camera to jump too far...

	const float speed = ((wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0f : 1.0f) + rightTrigger.x * 10.0f) * MoveSpeed * clampedDT;
	static XMVECTOR move = XMVectorSet(0, 0, 0, 0);
	XMVECTOR moveNew = XMVectorSet(leftStick.x, 0, leftStick.y, 0);

	//if (!wiInput::Down(wiInput::KEYBOARD_BUTTON_LCONTROL))
	//{
		// Only move camera if control not pressed
	if (wiInput::Down((wiInput::BUTTON)'A')) { moveNew += XMVectorSet(-1, 0, 0, 0); }
	else if (wiInput::Down((wiInput::BUTTON)'D')) { moveNew += XMVectorSet(1, 0, 0, 0); }
	if (wiInput::Down((wiInput::BUTTON)'W')) { moveNew += XMVectorSet(0, 0, 1, 0); }
	else if (wiInput::Down((wiInput::BUTTON)'S')) { moveNew += XMVectorSet(0, 0, -1, 0); }
	if (wiInput::Down((wiInput::BUTTON)'E')) { moveNew += XMVectorSet(0, 1, 0, 0); }
	else if (wiInput::Down((wiInput::BUTTON)'Q')) { moveNew += XMVectorSet(0, -1, 0, 0); }
	moveNew += XMVector3Normalize(moveNew);
	//}
	moveNew *= speed;


	float moveLength = XMVectorGetX(XMVector3Length(moveNew));
	if (moveLength < 0.0001f)
	{
		Accel = 0.0;
	}
	move = XMVectorLerp(move, moveNew, 0.20f * clampedDT / 0.0166f); // smooth the movement a bit
	moveLength = XMVectorGetX(XMVector3Length(move));
	if (moveLength < 0.0001f)
	{
		move = XMVectorSet(0, 0, 0, 0);
	}
	wiScene::TransformComponent camera_transform;
	
	if (abs(xDif) + abs(yDif) > 0 || moveLength > 0.0001f)
	{

		camera_transform.MatrixTransform(wiRenderer::GetCamera().GetInvView());
		camera_transform.UpdateTransform();
		XMMATRIX camRot = XMMatrixRotationQuaternion(XMLoadFloat4(&camera_transform.rotation_local));
		XMVECTOR move_rot = XMVector3TransformNormal(move, camRot);
		XMFLOAT3 _move;
		XMStoreFloat3(&_move, move_rot);
		camera_transform.Translate(_move);
		camera_transform.RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
		camera_transform.UpdateTransform();
		camera.SetDirty();
		camera.TransformCamera(camera_transform);
		camera.UpdateCamera();
		//camera.CreatePerspective((float)wiRenderer::GetInternalResolution().x, (float)wiRenderer::GetInternalResolution().y, 0.1f,200.0f);
	}
	

	RenderPath3D::Update(dt);
}
/*
void CyPathRender::ResizeLayout()
{
	RenderPath3D_PathTracing::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
}


void CyPathRender::Load()
{
	setSSREnabled(true);
	setReflectionsEnabled(false);
	setColorGradingEnabled(false);
	setLightShaftsEnabled(true);
	setShadowsEnabled(false);
	//setRaytracedReflectionsEnabled(true);
	setFXAAEnabled(true);
	//setEyeAdaptionEnabled(true);
	RenderPath3D_PathTracing::Load();
}

void CyPathRender::Update(float dt)
{

	CameraComponent& camera = wiRenderer::GetCamera();
	// Camera control:
	static XMFLOAT4 originalMouse = XMFLOAT4(0, 0, 0, 0);
	static float Accel = 0.0;
	static bool camControlStart = true;
	if (camControlStart)
	{
		originalMouse = wiInput::GetPointer();
	}

	XMFLOAT4 currentMouse = wiInput::GetPointer();
	float xDif = 0, yDif = 0;

	if (wiInput::Down(wiInput::MOUSE_BUTTON_LEFT))
	{
		camControlStart = false;
#if 1
		// Mouse delta from previous frame:
		xDif = currentMouse.x - originalMouse.x;
		yDif = currentMouse.y - originalMouse.y;
#else
		// Mouse delta from hardware read:
		xDif = wiInput::GetMouseState().delta_position.x;
		yDif = wiInput::GetMouseState().delta_position.y;
#endif
		xDif = 0.1f * xDif * (1.0f / 60.0f);
		yDif = 0.1f * yDif * (1.0f / 60.0f);
		wiInput::SetPointer(originalMouse);
		wiInput::HidePointer(true);
	}
	else
	{
		camControlStart = true;
		wiInput::HidePointer(false);
	}

	const float buttonrotSpeed = 2.0f / 60.0f;
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_LEFT))
	{
		xDif -= buttonrotSpeed;
	}
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_RIGHT))
	{
		xDif += buttonrotSpeed;
	}
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_UP))
	{
		yDif -= buttonrotSpeed;
	}
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_DOWN))
	{
		yDif += buttonrotSpeed;
	}

	const XMFLOAT4 leftStick = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_L, 0);
	const XMFLOAT4 rightStick = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_R, 0);
	const XMFLOAT4 rightTrigger = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_TRIGGER_R, 0);

	const float jostickrotspeed = 0.05f;
	xDif += rightStick.x * jostickrotspeed;
	yDif += rightStick.y * jostickrotspeed;

	xDif *= 1.0;
	yDif *= 1.0;
	if (Accel < 4) {
		Accel += dt * 2;
	}
	const float MoveSpeed = 4 * Accel;

	// FPS Camera
	const float clampedDT = dt; // if dt > 100 millisec, don't allow the camera to jump too far...

	const float speed = ((wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0f : 1.0f) + rightTrigger.x * 10.0f) * MoveSpeed * clampedDT;
	static XMVECTOR move = XMVectorSet(0, 0, 0, 0);
	XMVECTOR moveNew = XMVectorSet(leftStick.x, 0, leftStick.y, 0);

	//if (!wiInput::Down(wiInput::KEYBOARD_BUTTON_LCONTROL))
	//{
		// Only move camera if control not pressed
	if (wiInput::Down((wiInput::BUTTON)'A')) { moveNew += XMVectorSet(-1, 0, 0, 0); }
	else if (wiInput::Down((wiInput::BUTTON)'D')) { moveNew += XMVectorSet(1, 0, 0, 0); }
	if (wiInput::Down((wiInput::BUTTON)'W')) { moveNew += XMVectorSet(0, 0, 1, 0); }
	else if (wiInput::Down((wiInput::BUTTON)'S')) { moveNew += XMVectorSet(0, 0, -1, 0); }
	if (wiInput::Down((wiInput::BUTTON)'E')) { moveNew += XMVectorSet(0, 1, 0, 0); }
	else if (wiInput::Down((wiInput::BUTTON)'Q')) { moveNew += XMVectorSet(0, -1, 0, 0); }
	moveNew += XMVector3Normalize(moveNew);
	//}
	moveNew *= speed;


	float moveLength = XMVectorGetX(XMVector3Length(moveNew));
	if (moveLength < 0.0001f)
	{
		Accel = 0.0;
	}
	move = XMVectorLerp(move, moveNew, 0.20f * clampedDT / 0.0166f); // smooth the movement a bit
	moveLength = XMVectorGetX(XMVector3Length(move));
	if (moveLength < 0.0001f)
	{
		move = XMVectorSet(0, 0, 0, 0);
	}
	wiScene::TransformComponent camera_transform;
	if (abs(xDif) + abs(yDif) > 0 || moveLength > 0.0001f)
	{

		camera_transform.MatrixTransform(wiRenderer::GetCamera().GetInvView());
		camera_transform.UpdateTransform();
		XMMATRIX camRot = XMMatrixRotationQuaternion(XMLoadFloat4(&camera_transform.rotation_local));
		XMVECTOR move_rot = XMVector3TransformNormal(move, camRot);
		XMFLOAT3 _move;
		XMStoreFloat3(&_move, move_rot);
		camera_transform.Translate(_move);
		camera_transform.RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
		camera_transform.UpdateTransform();
		camera.SetDirty();
		camera.TransformCamera(camera_transform);
		camera.UpdateCamera();
	}
	RenderPath3D_PathTracing::Update(dt);
}
*/