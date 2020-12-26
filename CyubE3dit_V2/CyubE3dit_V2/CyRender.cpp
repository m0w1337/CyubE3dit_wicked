#include "stdafx.h"
#include "CyRender.h"
#include "settings.h"
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <cmath>

using namespace std;
using namespace wiECS;
using namespace wiScene;
using namespace wiGraphics;
wiECS::Entity CyMainComponent::m_headLight = 0;
wiECS::Entity CyMainComponent::m_probe = 0;

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
	wiRenderer::ClearWorld(wiScene::GetScene());
	wiScene::GetScene().weather = WeatherComponent();
	if (wiLua::GetLuaState() != nullptr) {
		wiLua::KillProcesses();
	}
	// Add some nice weather, not just black:
	auto& weather	  = scene.weathers.Create(CreateEntity());

	weather.fogStart  = 200;
	weather.fogEnd	  = 2000;
	weather.fogHeight = 0;
	
	weather.horizon = XMFLOAT3(0.4f, 0.4f, 0.9f);
	weather.zenith	= XMFLOAT3(0.5f, 0.7f, 0.9f);
	weather.ambient			  = XMFLOAT3(1.f, 1.f, 1.0f);
	weather.SetRealisticSky(true);
	weather.skyMapName		  = "images/sky.dds";
	weather.cloudiness	  = 0.2f;
	weather.windSpeed	  = 1.3f;
	weather.windRandomness	  = 5.5f;
	weather.windWaveSize	  = 0.05f;

	weather.windDirection	  = XMFLOAT3(0.2, 0, 0.2);
	Entity LightEnt		  = scene.Entity_CreateLight("Sunlight", XMFLOAT3(0, 0, 0), XMFLOAT3(1.0, 0.8, 0.6f), 10, 1000);
	LightComponent* light = scene.lights.GetComponent(LightEnt);
	light->SetType(LightComponent::LightType::DIRECTIONAL);
	//light->color				  = XMFLOAT3(1.0f,0.9f,0.7f);
	TransformComponent& transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(0.47f, -0.579, 0.64f));
	transform.SetDirty();
	transform.UpdateTransform();
	//light->SetStatic(true);
	//light->SetVolumetricsEnabled(true);
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
	m_headLight					   = wiScene::GetScene().Entity_CreateLight("filllight", XMFLOAT3(0, 0, 0), XMFLOAT3(1.0f, 1.f, 0.5f), 10, 5);
	light		   = wiScene::GetScene().lights.GetComponent(m_headLight);
	light->SetType(LightComponent::LightType::SPOT);
	light->SetVolumetricsEnabled(true);
	light->SetVisualizerEnabled(false);
	light->SetCastShadow(true);
	light->fov = 1.f;
	wiScene::GetScene().springs.Create(m_headLight);
	wiScene::GetScene().springs.GetComponent(m_headLight)->wind_affection = 1.0;

	m_probe			 = wiScene::GetScene().Entity_CreateEnvironmentProbe("", XMFLOAT3(0.0f, 0.0f, 0.0f));
	EnvironmentProbeComponent* probe = wiScene::GetScene().probes.GetComponent(m_probe);
	probe->SetRealTime(true);
	probe->SetDirty();
}

void CyMainComponent::Compose(CommandList cmd) {
	auto range = wiProfiler::BeginRangeCPU("Compose");

	if (GetActivePath() != nullptr)
	{
		GetActivePath()->Compose(cmd);
	}

	if (fadeManager.IsActive())
	{
		// display fade rect
		static wiImageParams fx;
		fx.siz.x   = (float)wiRenderer::GetDevice()->GetScreenWidth();
		fx.siz.y   = (float)wiRenderer::GetDevice()->GetScreenHeight();
		fx.opacity = fadeManager.opacity;
		wiImage::Draw(wiTextureHelper::getColor(fadeManager.color), fx, cmd);
	}

	// Draw the information display
	if (infoDisplay.active)
	{
		stringstream ss("");
		if (infoDisplay.watermark)
		{
			ss << "Wicked Engine " << wiVersion::GetVersionString() << " ";

#if defined(_ARM)
			ss << "[ARM]";
#elif defined(_WIN64)
			ss << "[64-bit]";
#elif defined(_WIN32)
			ss << "[32-bit]";
#endif

#ifdef PLATFORM_UWP
			ss << "[UWP]";
#endif

#ifdef WICKEDENGINE_BUILD_DX11
			if (dynamic_cast<GraphicsDevice_DX11*>(wiRenderer::GetDevice()))
			{
				ss << "[DX11]";
			}
#endif
#ifdef WICKEDENGINE_BUILD_DX12
			if (dynamic_cast<GraphicsDevice_DX12*>(wiRenderer::GetDevice()))
			{
				ss << "[DX12]";
			}
#endif
#ifdef WICKEDENGINE_BUILD_VULKAN
			if (dynamic_cast<GraphicsDevice_Vulkan*>(wiRenderer::GetDevice()))
			{
				ss << "[Vulkan]";
			}
#endif

#ifdef _DEBUG
			ss << "[DEBUG]";
#endif
			if (wiRenderer::GetDevice()->IsDebugDevice())
			{
				ss << "[debugdevice]";
			}
			ss << endl;
		}
		if (infoDisplay.resolution)
		{
			ss << "Resolution: " << wiRenderer::GetDevice()->GetResolutionWidth() << " x " << wiRenderer::GetDevice()->GetResolutionHeight() << " (" << wiPlatform::GetDPI() << " dpi)" << endl;
		}
		if (infoDisplay.fpsinfo)
		{
			deltatimes[fps_avg_counter++ % arraysize(deltatimes)] = deltaTime;
			float displaydeltatime								  = deltaTime;
			if (fps_avg_counter > arraysize(deltatimes))
			{
				float avg_time = 0;
				for (int i = 0; i < arraysize(deltatimes); ++i)
				{
					avg_time += deltatimes[i];
				}
				displaydeltatime = avg_time / arraysize(deltatimes);
			}

			ss.precision(2);
			ss << fixed << 1.0f / displaydeltatime << " FPS" << endl;
		}
		ss << settings::numVisChunks << " Chunks visible" << endl;
		cyImportant* world = settings::getWorld();
		XMFLOAT3 position  = wiScene::GetCamera().Eye;
		position.x += world->m_playerpos.x;
		position.z += world->m_playerpos.y;
		ss << "X: " << position.x << " Y: " << position.z << " Z: " << position.y << endl;

		

		/*if (infoDisplay.heap_allocation_counter)
		{
			ss << "Heap allocations per frame: " << number_of_allocs.load() << endl;
			number_of_allocs.store(0);
		}*/

#ifdef _DEBUG
		ss << "Warning: This is a [DEBUG] build, performance will be slow!" << endl;
#endif
		if (wiRenderer::GetDevice()->IsDebugDevice())
		{
			ss << "Warning: Graphics is in [debugdevice] mode, performance will be slow!" << endl;
		}

		if (renderer.hovered.entity != wiECS::INVALID_ENTITY) {
			ss << "Hovered Chunk: " + wiScene::GetScene().names.GetComponent(renderer.hovered.entity)->name << endl;
		}

		ss.precision(2);
		wiFont::Draw(ss.str(), wiFontParams(4, 4, infoDisplay.size, WIFALIGN_LEFT, WIFALIGN_TOP, wiColor(255, 255, 255, 255), wiColor(0, 0, 0, 255)), cmd);

	}
	

	wiProfiler::DrawData(4, 120, cmd);

	wiBackLog::Draw(cmd);

	wiProfiler::EndRange(range);  // Compose
}

void CyRender::ResizeLayout() {
	RenderPath3D::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
	worldSelector.SetPos(XMFLOAT2(screenW / 2.f - worldSelector.scale.x / 2.f, 10));
	label.SetPos(XMFLOAT2(worldSelector.translation.x + worldSelector.scale.x + 10, 10));
}

void CyRender::Load() {
	hovered = wiScene::PickResult();
	
	setBloomEnabled(true);
	setBloomThreshold(1);
	setReflectionsEnabled(false);
	setSSREnabled(false);
	//setRaytracedReflectionsEnabled(true);
	//wiRenderer::SetRaytracedShadowsEnabled(true);
	//wiRenderer::SetRaytracedShadowsSampleCount(25);
	setMotionBlurEnabled(false);
	setColorGradingEnabled(false);
	setDepthOfFieldEnabled(false);
	setDepthOfFieldFocus(4.0f);
	setDepthOfFieldAspect(1.0f);
	setDepthOfFieldStrength(0.7f);
	setMSAASampleCount(4);
	wiRenderer::SetShadowProps2D(2048, -1);
	//wiRenderer::SetGamma(2.2);
	wiPhysicsEngine::SetEnabled(true);
	setLightShaftsEnabled(true);
	setShadowsEnabled(true);
	wiRenderer::SetTransparentShadowsEnabled(false);
	setVolumetricCloudsEnabled(true);
	//setExposure(1.f);
	wiRenderer::SetTemporalAAEnabled(false);
	
	wiRenderer::GetDevice()->SetVSyncEnabled(true);
	wiRenderer::SetToDrawGridHelper(false);
	wiRenderer::SetToDrawDebugEnvProbes(false);
	wiRenderer::SetVoxelRadianceEnabled(false);
	wiRenderer::SetVoxelRadianceNumCones(2);
	wiRenderer::SetVoxelRadianceRayStepSize(0.75f);
	wiRenderer::SetVoxelRadianceMaxDistance(30);
	wiRenderer::SetVoxelRadianceSecondaryBounceEnabled(true);
	wiRenderer::SetOcclusionCullingEnabled(false);
	wiProfiler::SetEnabled(true);
	setAO(AO_HBAO);
	setAOPower(0.5);
	setAORange(32);
	setAOSampleCount(4);
	setFXAAEnabled(true);
	setEyeAdaptionEnabled(true);
	wiScene::GetCamera().zNearP = 0.1;
	wiScene::GetCamera().zFarP = 2000;
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
		RAY pickRay					  = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
		hovered						  = wiScene::Pick(pickRay);
	}
	if (hovered.entity != wiECS::INVALID_ENTITY)
	{
		const ObjectComponent* object = wiScene::GetScene().objects.GetComponent(hovered.entity);
		const AABB& aabb = *wiScene::GetScene().aabb_objects.GetComponent(hovered.entity);
		XMFLOAT4X4 hoverBox;
		XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
		wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.5f, 0.2f, 5.f, 3.0f));
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
	const float clampedDT = min(dt, 0.1f);	// if dt > 100 millisec, don't allow the camera to jump too far...


	const float speed	 = ((wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0f : 1.2f) + rightTrigger.x * 10.0f) * MoveSpeed * clampedDT;
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
	//if (moveLength < 10) {	//Ignore when movement is too far
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
			camera.TransformCamera(camera_transform);
			camera.UpdateCamera();
			camera.SetDirty();


			if (CyMainComponent::m_headLight != INVALID_ENTITY) {
				TransformComponent* lightT = wiScene::GetScene().transforms.GetComponent(CyMainComponent::m_headLight);
				lightT->Translate(_move);
				lightT->RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
				lightT->SetDirty();
				lightT->UpdateTransform();
			}
			if (CyMainComponent::m_probe != INVALID_ENTITY) {
				TransformComponent* probeT = wiScene::GetScene().transforms.GetComponent(CyMainComponent::m_probe);
				probeT->Translate(_move);
				//probeT->RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
				probeT->SetDirty();
				probeT->UpdateTransform();
			}
			
			//camera.CreatePerspective((float)wiRenderer::GetInternalResolution().x, (float)wiRenderer::GetInternalResolution().y, 0.1f,200.0f);
		}
//	}

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
