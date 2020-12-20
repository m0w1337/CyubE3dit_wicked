#include "stdafx.h"
#include "CyRender.h"

#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <cmath>

using namespace wiECS;
using namespace wiScene;

void CyMainComponent::Initialize() {
	MainComponent::Initialize();

	infoDisplay.active					= true;
	infoDisplay.watermark				= true;
	infoDisplay.fpsinfo					= true;
	infoDisplay.resolution				= true;
	infoDisplay.heap_allocation_counter = true;

	renderer.Load();
	//pathRenderer.Load();
	ActivatePath(&renderer);
}

void CyMainComponent::CreateScene(void) {
	Scene& scene = wiScene::GetScene();
	wiRenderer::GetDevice()->SetVSyncEnabled(true);
	wiRenderer::SetToDrawGridHelper(false);
	wiRenderer::SetTemporalAAEnabled(true);
	wiRenderer::ClearWorld(wiScene::GetScene());
	wiScene::GetScene().weather = WeatherComponent();
	if (wiLua::GetLuaState() != nullptr) {
		wiLua::KillProcesses();
	}
	// Add some nice weather, not just black:
	auto& weather	  = scene.weathers.Create(CreateEntity());
	weather.ambient	  = XMFLOAT3(0.6f, 0.5f, 0.5f);
	weather.fogStart  = 300;
	weather.fogEnd	  = 2000;
	weather.fogHeight = 0;
	weather.SetRealisticSky(false);
	weather.skyMapName	  = "images/sky.dds";
	weather.skyMap		  = wiResourceManager::Load("images/sky.dds");
	weather.horizon		  = XMFLOAT3(0.4f, 0.4f, 0.9f);
	weather.zenith		  = XMFLOAT3(0.5f, 0.7f, 0.9f);
	weather.cloudiness	  = 0.2f;
	weather.windSpeed	  = 1.3f;
	weather.windRandomness	  = 5.5f;
	weather.windWaveSize	  = 0.05f;
	weather.windDirection	  = XMFLOAT3(0.2, 0, 0.2);
	Entity LightEnt		  = scene.Entity_CreateLight("Sunlight", XMFLOAT3(1, 0.5, -1), XMFLOAT3(1.0, 1., 1.f), 20, 1000);
	LightComponent* light = scene.lights.GetComponent(LightEnt);
	light->SetType(LightComponent::LightType::DIRECTIONAL);
	TransformComponent& transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(0.5f, 0, 0.6f));
	transform.SetDirty();
	transform.UpdateTransform();
	light->SetCastShadow(true);
	light->lensFlareRimTextures.resize(6);
	light->lensFlareNames.resize(6);
	light->lensFlareRimTextures[0] = wiResourceManager::Load("images/flare0.jpg");
	light->lensFlareNames[0] = "flare0";
	light->lensFlareRimTextures[1] = wiResourceManager::Load("images/flare1.jpg");
	light->lensFlareNames[1]	   = "flare1";
	light->lensFlareRimTextures[2] = wiResourceManager::Load("images/flare2.jpg");
	light->lensFlareNames[2]	   = "flare2";
	light->lensFlareRimTextures[3] = wiResourceManager::Load("images/flare3.jpg");
	light->lensFlareNames[3]	   = "flare3";
	light->lensFlareRimTextures[4] = wiResourceManager::Load("images/flare4.jpg");
	light->lensFlareNames[4]	   = "flare4";
	light->lensFlareRimTextures[5] = wiResourceManager::Load("images/flare5.jpg");
	light->lensFlareNames[5]	   = "flare5";
	Entity fillLight			   = scene.Entity_CreateLight("filllight", XMFLOAT3(100, 300, -100), XMFLOAT3(1.0f, 1., 1.f), 10, 1000);
}

void CyRender::ResizeLayout() {
	RenderPath3D::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
	worldSelector.SetPos(XMFLOAT2(screenW / 2.f - worldSelector.scale.x / 2.f, 10));
	label.SetPos(XMFLOAT2(worldSelector.translation.x + worldSelector.scale.x + 10, 10));
}

void CyRender::Load() {
	setSSREnabled(true);
	setBloomEnabled(false);
	setBloomThreshold(10);
	setMotionBlurEnabled(false);
	setReflectionsEnabled(true);
	setColorGradingEnabled(false);
	wiPhysicsEngine::SetEnabled(true);
	setLightShaftsEnabled(true);
	setShadowsEnabled(true);
	setVolumetricCloudsEnabled(true);
	wiRenderer::SetTransparentShadowsEnabled(false);
	wiRenderer::SetVoxelRadianceEnabled(false);
	wiRenderer::SetVoxelRadianceNumCones(8);
	wiRenderer::SetVoxelRadianceRayStepSize(0.25f);
	wiRenderer::SetVoxelRadianceMaxDistance(20);
	wiRenderer::SetVoxelRadianceSecondaryBounceEnabled(true);
	wiRenderer::SetOcclusionCullingEnabled(false);
	wiProfiler::SetEnabled(true);
	setAO(AO_MSAO);
	setAOPower(1);
	setAORange(400);
	setAOSampleCount(2);
	setFXAAEnabled(true);
	setEyeAdaptionEnabled(false);

	label.Create("Label1");
	label.SetText("Wicked Engine Test Framework");
	label.font.params.h_align = WIFALIGN_CENTER;
	label.SetSize(XMFLOAT2(240, 20));
	GetGUI().AddWidget(&label);

	worldSelector.Create("TestSelector");
	worldSelector.SetText("Current World: ");
	worldSelector.SetSize(XMFLOAT2(250, 30));
	worldSelector.SetPos(XMFLOAT2(300, 20));
	worldSelector.SetColor(wiColor(100, 100, 100, 50), wiWidget::WIDGETSTATE::IDLE);
	worldSelector.SetColor(wiColor(100, 100, 100, 255), wiWidget::WIDGETSTATE::FOCUS);
	worldSelector.AddItem("HelloWorld");
	worldSelector.AddItem("Model");
	worldSelector.AddItem("EmittedParticle 1");
	worldSelector.SetMaxVisibleItemCount(10);
	worldSelector.OnSelect([=](wiEventArgs args) {
		switch (args.iValue)
		{
			case 0:
			{
				break;
			}
			default:
				break;
		}
	});
	worldSelector.SetSelected(0);
	GetGUI().AddWidget(&worldSelector);
	RenderPath3D::Load();
}

void CyRender::Update(float dt) {

	CameraComponent& camera = wiScene::GetCamera();
	// Camera control:
	static XMFLOAT4 originalMouse = XMFLOAT4(0, 0, 0, 0);
	static float Accel			  = 0.0;
	static bool camControlStart	  = true;
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
	} else
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

	const XMFLOAT4 leftStick	= wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_L, 0);
	const XMFLOAT4 rightStick	= wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_R, 0);
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
	const float clampedDT = dt;	 // if dt > 100 millisec, don't allow the camera to jump too far...

	const float speed	 = ((wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0f : 1.0f) + rightTrigger.x * 10.0f) * MoveSpeed * clampedDT;
	static XMVECTOR move = XMVectorSet(0, 0, 0, 0);
	XMVECTOR moveNew	 = XMVectorSet(leftStick.x, 0, leftStick.y, 0);

	//if (!wiInput::Down(wiInput::KEYBOARD_BUTTON_LCONTROL))
	//{
	// Only move camera if control not pressed
	if (wiInput::Down((wiInput::BUTTON)'A')) {
		moveNew += XMVectorSet(-1, 0, 0, 0);
	} else if (wiInput::Down((wiInput::BUTTON)'D')) {
		moveNew += XMVectorSet(1, 0, 0, 0);
	}
	if (wiInput::Down((wiInput::BUTTON)'W')) {
		moveNew += XMVectorSet(0, 0, 1, 0);
	} else if (wiInput::Down((wiInput::BUTTON)'S')) {
		moveNew += XMVectorSet(0, 0, -1, 0);
	}
	if (wiInput::Down((wiInput::BUTTON)'E')) {
		moveNew += XMVectorSet(0, 1, 0, 0);
	} else if (wiInput::Down((wiInput::BUTTON)'Q')) {
		moveNew += XMVectorSet(0, -1, 0, 0);
	}
	moveNew += XMVector3Normalize(moveNew);
	//}
	moveNew *= speed;

	float moveLength = XMVectorGetX(XMVector3Length(moveNew));
	if (moveLength < 0.0001f)
	{
		Accel = 0.0;
	}
	move	   = XMVectorLerp(move, moveNew, 0.20f * clampedDT / 0.0166f);	// smooth the movement a bit
	moveLength = XMVectorGetX(XMVector3Length(move));
	if (moveLength < 0.0001f)
	{
		move = XMVectorSet(0, 0, 0, 0);
	}
	wiScene::TransformComponent camera_transform;
	if (moveLength < 32) {	//Ignore when movement is too far
		if (abs(xDif) + abs(yDif) > 0 || moveLength > 0.0001f)
		{

			camera_transform.MatrixTransform(wiScene::GetCamera().GetInvView());
			camera_transform.UpdateTransform();
			XMMATRIX camRot	  = XMMatrixRotationQuaternion(XMLoadFloat4(&camera_transform.rotation_local));
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
