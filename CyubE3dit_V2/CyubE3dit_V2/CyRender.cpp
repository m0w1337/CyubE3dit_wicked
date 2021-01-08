#include "stdafx.h"
#include "CyRender.h"
#include "cyWorlds.h"
#include "cySchematic.h"
#include "settings.h"
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <cmath>
#include "windows.h"
#include "psapi.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;
using namespace wiGraphics;
wiECS::Entity CyMainComponent::m_headLight = 0;
wiECS::Entity CyMainComponent::m_probe	   = 0;

void CyMainComponent::Initialize() {
	MainComponent::Initialize();

	infoDisplay.active					= true;
	infoDisplay.watermark				= true;
	infoDisplay.fpsinfo					= true;
	infoDisplay.resolution				= true;
	infoDisplay.heap_allocation_counter = true;
	//renderer.Load();
	//pathRenderer.Load();
	loader.Load();
	renderer.main = this;
	loader.addLoadingComponent(&renderer, this, 0.5f, 0xFFFFFFFF);
	loader.addLoadingFunction([this](wiJobArgs) {
		cyBlocks::LoadRegBlocks();
		cyBlocks::LoadCustomBlocks();
		cyBlocks::loadMeshes();
		CreateScene(); });
	ActivatePath(&loader, 0.2f);
}

void CyLoadingScreen::Load() {
	font = wiSpriteFont("CyubE3dit: Loading block materials...", wiFontParams(wiRenderer::GetDevice()->GetScreenWidth() * 0.5f, wiRenderer::GetDevice()->GetScreenHeight() * 0.5f, 36,
																			  WIFALIGN_CENTER, WIFALIGN_CENTER));
	AddFont(&font);
	sprite					= wiSprite("images/logo_alpha.png");
	sprite.anim.opa			= 1;
	sprite.anim.repeatable	= true;
	sprite.params.pos		= XMFLOAT3(wiRenderer::GetDevice()->GetScreenWidth() * 0.5f, wiRenderer::GetDevice()->GetScreenHeight() * 0.5f, 0);
	sprite.params.siz		= XMFLOAT2(256, 256);
	sprite.params.pivot		= XMFLOAT2(0.5f, 0.9f);
	sprite.params.quality	= QUALITY_LINEAR;
	sprite.params.blendFlag = BLENDMODE_ALPHA;
	AddSprite(&sprite);
	LoadingScreen::Load();
}
void CyLoadingScreen::Update(float dt) {
	font.params.posX  = wiRenderer::GetDevice()->GetScreenWidth() * 0.5f;
	font.params.posY  = wiRenderer::GetDevice()->GetScreenHeight() * 0.5f;
	sprite.params.pos = XMFLOAT3(wiRenderer::GetDevice()->GetScreenWidth() * 0.5f, wiRenderer::GetDevice()->GetScreenHeight() * 0.5f, 0);

	LoadingScreen::Update(dt);
}

void CyMainComponent::CreateScene(void) {
	Scene& scene				= wiScene::GetScene();
	wiScene::GetScene().weather = WeatherComponent();
	if (wiLua::GetLuaState() != nullptr) {
		wiLua::KillProcesses();
	}
	// Add some nice weather, not just black:
	auto& weather = scene.weathers.Create(CreateEntity());

	weather.fogStart  = 5;
	weather.fogEnd	  = 2000;
	weather.fogHeight = 0;
	weather.horizon	  = XMFLOAT3(0.4f, 0.4f, 0.9f);
	weather.zenith	  = XMFLOAT3(0.5f, 0.7f, 0.9f);
	weather.ambient	  = XMFLOAT3(0.8f, 0.8f, 0.9f);
	weather.SetRealisticSky(false);
	weather.skyMapName	   = "images/sky.dds";
	weather.cloudiness	   = 0.2f;
	weather.windSpeed	   = 1.3f;
	weather.windRandomness = 1.5f;
	weather.windWaveSize   = 2.5f;

	weather.windDirection = XMFLOAT3(0.2, 0, 0.2);
	Entity LightEnt		  = scene.Entity_CreateLight("Sunlight", XMFLOAT3(0, 0, 0), XMFLOAT3(1.0, 0.8, 0.6f), 35, 1000);
	LightComponent* light = scene.lights.GetComponent(LightEnt);
	light->SetType(LightComponent::LightType::DIRECTIONAL);
	//light->color				  = XMFLOAT3(1.0f,0.9f,0.7f);
	TransformComponent& transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(0.87f, 0.f, -0.5f));
	transform.SetDirty();
	transform.UpdateTransform();
	//light->SetStatic(true);
	//light->SetVolumetricsEnabled(true);
	light->SetCastShadow(true);
	light->lensFlareRimTextures.resize(6);
	light->lensFlareNames.resize(6);
	light->lensFlareRimTextures[0] = wiResourceManager::Load("images/flare0.jpg");
	light->lensFlareNames[0]	   = "flare0";
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

	//LightEnt = wiScene::GetScene().Entity_CreateLight("filllight", XMFLOAT3(0, 0, 0), XMFLOAT3(0.9f, 0.9f, 1.f), 2, 1000);
	//light	 = scene.lights.GetComponent(LightEnt);
	//light->SetType(LightComponent::LightType::DIRECTIONAL);
	//light->SetStatic(true);
	transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(-0.47f, 0.579, -0.64f));
	transform.SetDirty();
	transform.UpdateTransform();
	m_headLight = wiScene::GetScene().Entity_CreateLight("Headlight", XMFLOAT3(0, 0, 0), XMFLOAT3(1.0f, 1.f, 0.5f), 0, 10);
	light		= wiScene::GetScene().lights.GetComponent(m_headLight);
	light->SetType(LightComponent::LightType::SPOT);
	light->SetVolumetricsEnabled(false);
	light->SetCastShadow(false);
	light->fov = 1.2f;
	wiScene::GetScene().springs.Create(m_headLight);
	wiScene::GetScene().springs.GetComponent(m_headLight)->wind_affection = 1.0;

	infoDisplay.active	   = true;
	infoDisplay.watermark  = true;
	infoDisplay.resolution = true;
	infoDisplay.fpsinfo	   = true;
	m_probe				   = wiScene::GetScene().Entity_CreateEnvironmentProbe("", XMFLOAT3(0.0f, 0.0f, 0.0f));
	cyImportant* world	   = settings::getWorld();
	TransformComponent ctransform;
	settings::newWorld	= "";
	settings::thisWorld = "";
	world->m_stopped	= true;
	float screenW		= wiRenderer::GetDevice()->GetScreenWidth();
	float screenH		= wiRenderer::GetDevice()->GetScreenHeight();
	renderer.ResizeLayout();
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
		ss << "Entity pool usage: " + to_string((((float)wiECS::next * 100.0f) / UINT32_MAX)) + "%" << endl;
		static int64_t id			= -1;
		static wiECS::Entity oldMat = 0;
		static string blockname		= "";
		wiScene::Scene& scene					  = wiScene::GetScene();
		static wiScene::MaterialComponent* highlightComp = scene.materials.GetComponent(scene.materials.GetIndex(0));
		wiECS::Entity mat = 0;
		if (renderer.hovered.entity)
			mat								 = scene.meshes.GetComponent(scene.objects.GetComponent(renderer.hovered.entity)->meshID)->subsets[renderer.hovered.subsetIndex].materialID;
		
		if (oldMat != mat) {
			oldMat = renderer.hovered.entity;
			id	   = -1;
			if (oldMat) {
				for (uint32_t i = 0; i < 256; i++) {
					for (uint8_t ii = 0; ii < 6; ii++) {
						if (cyBlocks::m_regBlockMats[i][ii] == mat) {
							blockname = cyBlocks::m_regBlockNames[i];
							id		  = i;
							break;
							/*for (uint8_t iii = 0; iii < 6; iii++) {
								if (cyBlocks::m_regBlockMats[i][iii]) {
									highlightComp = scene.materials.GetComponent(cyBlocks::m_regBlockMats[i][iii]);
								}
							}*/
						}
					}
					/*if (i != id) {
						for (uint8_t iii = 0; iii < 6; iii++) {
							if (cyBlocks::m_regBlockMats[i][iii]) {
								scene.materials.GetComponent(cyBlocks::m_regBlockMats[i][iii])->SetEmissiveColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
							}
						}
					}*/
				}
				if (id == -1) {
					for (auto& it : cyBlocks::m_cBlockTypes) {
						for (uint8_t ii = 0; ii < 6; ii++) {
							if (it.second.material[ii] == mat) {
								if (it.second.creator.empty()) {
									blockname = it.second.name;
								} else {
									blockname = it.second.name + "( by " + it.second.creator + ")";
								}
								id = it.first;
								break;
								/*for (uint8_t iii = 0; iii < 6; iii++) {
									if (it.second.material[iii]) {
										highlightComp = scene.materials.GetComponent(it.second.material[iii]);
									}
								}*/
							}
						}
						/*if (it.first != id) {
							for (uint8_t iii = 0; iii < 6; iii++) {
								if (it.second.material[iii]) {
									scene.materials.GetComponent(it.second.material[iii])->SetEmissiveColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
								}
							}
						}*/
					}
				}
			} /*else {
				for (uint32_t i = 0; i < 256; i++) {
					for (uint8_t iii = 0; iii < 6; iii++) {
						if (cyBlocks::m_regBlockMats[i][iii]) {
							scene.materials.GetComponent(cyBlocks::m_regBlockMats[i][iii])->SetEmissiveColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
						}
					}
				}
				for (auto& it : cyBlocks::m_cBlockTypes) {
					for (uint8_t iii = 0; iii < 6; iii++) {
						if (it.second.material[iii]) {
							scene.materials.GetComponent(it.second.material[iii])->SetEmissiveColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
						}
					}
				}
			}*/
		}
		if (renderer.hovered.entity != wiECS::INVALID_ENTITY) {
			ss << "Hovered Chunk: " + scene.names.GetComponent(renderer.hovered.entity)->name << endl;
			//highlightComp->SetEmissiveColor(XMFLOAT4(0.0f, 0.0f, 1.0f, 0.1f * renderer.sinepulse));
			ss << "Hovered Block: " + blockname + "(ID " + to_string(id) + ")" << endl;
		}
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		ss << "Physical RAM used: " + to_string(pmc.WorkingSetSize / 1000000) + " MB" << endl;
		ss << "Virtual RAM used: " + to_string(pmc.PrivateUsage / 1000000) + " MB" << endl;
		ss.precision(2);
		wiFont::Draw(ss.str(), wiFontParams(4, 4, infoDisplay.size, WIFALIGN_LEFT, WIFALIGN_TOP, wiColor(255, 255, 255, 255), wiColor(0, 0, 0, 255)), cmd);
	}

	wiProfiler::DrawData(4, 180, cmd);

	wiBackLog::Draw(cmd);

	wiProfiler::EndRange(range);  // Compose
}

void CyRender::ResizeLayout() {
	RenderPath3D::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
	worldSelector.SetPos(XMFLOAT2((screenW - worldSelector.scale.x) / 2.f, 10));
	label.SetPos(XMFLOAT2((screenW - label.scale.x) / 2, screenH - label.scale.y - 15));
	uint32_t xOffset = 15;
	postprocessWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));
	xOffset += 10 + postprocessWnd_Toggle.scale.x;
	rendererWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));
	xOffset += 10 + rendererWnd_Toggle.scale.x;
	loadSchBtn.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));
	viewDist.SetPos(XMFLOAT2(screenW - viewDist.scale.x - 45, screenH - postprocessWnd_Toggle.scale.y - 15));
}

void CyRender::Load() {
	hovered = wiScene::PickResult();

	setBloomEnabled(true);
	setBloomThreshold(1.5);
	setReflectionsEnabled(true);
	setSSREnabled(false);
	wiRenderer::SetGamma(2.2);
	wiPhysicsEngine::SetEnabled(true);
	setLightShaftsEnabled(true);
	setShadowsEnabled(true);
	setMSAASampleCount(1);
	setSharpenFilterEnabled(true);
	setSharpenFilterAmount(0.17);
	wiRenderer::SetTransparentShadowsEnabled(false);
	//wiRenderer::SetShadowProps2D(2048, 4);
	setVolumetricCloudsEnabled(false);
	setExposure(1.1f);
	wiRenderer::SetTemporalAAEnabled(true);
	wiRenderer::GetDevice()->SetVSyncEnabled(true);
	wiRenderer::SetVoxelRadianceEnabled(true);
	wiRenderer::SetVoxelRadianceNumCones(2);
	wiRenderer::SetVoxelRadianceRayStepSize(1.0f);
	wiRenderer::SetVoxelRadianceMaxDistance(30);
	wiRenderer::SetVoxelRadianceVoxelSize(0.1);
	wiRenderer::SetVoxelRadianceSecondaryBounceEnabled(true);
	wiRenderer::SetOcclusionCullingEnabled(true);
	wiProfiler::SetEnabled(true);
	setAO(AO_MSAO);
	setAOPower(0.2);
	setFXAAEnabled(false);
	setEyeAdaptionEnabled(false);
	wiScene::GetCamera().zNearP = 0.1;
	wiScene::GetCamera().zFarP	= 2000;
	label.Create("Label1");
	label.SetText("CyubE3dit Wicked - sneak peek");
	label.SetColor(wiColor(100, 100, 100, 0), wiWidget::WIDGETSTATE::IDLE);
	label.SetColor(wiColor(100, 100, 100, 0), wiWidget::WIDGETSTATE::FOCUS);
	label.SetColor(wiColor(100, 130, 130, 0), wiWidget::WIDGETSTATE::ACTIVE);
	label.font.params.h_align = WIFALIGN_CENTER;
	label.SetSize(XMFLOAT2(240, 20));
	GetGUI().AddWidget(&label);
	cyWorlds::getWorlds();
	worldSelector.Create("TestSelector");
	worldSelector.SetText("Current World: ");
	worldSelector.SetSize(XMFLOAT2(250, 20));
	worldSelector.SetPos(XMFLOAT2(300, 20));
	worldSelector.SetColor(wiColor(100, 100, 100, 30), wiWidget::WIDGETSTATE::IDLE);
	worldSelector.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	worldSelector.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	worldSelector.AddItem("");
	for (const auto& world : cyWorlds::worlds) {
		worldSelector.AddItem(world);
	}
	worldSelector.SetSelected(0);
	worldSelector.SetMaxVisibleItemCount(15);
	worldSelector.OnSelect([=](wiEventArgs args) {
		settings::newWorld = worldSelector.GetItemText(args.iValue);
	});

	GetGUI().AddWidget(&worldSelector);

	rendererWnd = RendererWindow();
	rendererWnd.Create(this);
	GetGUI().AddWidget(&rendererWnd);

	postprocessWnd = PostprocessWindow();
	postprocessWnd.Create(this);
	GetGUI().AddWidget(&postprocessWnd);

	viewDist.Create(settings::VIEWDIST_MIN, settings::VIEWDIST_MAX, settings::getViewDist(), settings::VIEWDIST_MAX / settings::VIEWDIST_MIN - 1, "View distance: ");
	viewDist.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	viewDist.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	viewDist.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	viewDist.SetValue(settings::viewDist);
	viewDist.OnSlide([=](wiEventArgs args) { settings::setViewDist((uint32_t)args.fValue); });
	viewDist.SetSize(XMFLOAT2(300, 20));
	GetGUI().AddWidget(&viewDist);

	rendererWnd_Toggle.Create("Renderer");
	rendererWnd_Toggle.SetTooltip("Renderer settings");
	rendererWnd_Toggle.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	rendererWnd_Toggle.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	rendererWnd_Toggle.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	rendererWnd_Toggle.SetSize(XMFLOAT2(120, 20));
	rendererWnd_Toggle.OnClick([&](wiEventArgs args) {
		rendererWnd.SetVisible(!rendererWnd.IsVisible());
	});
	GetGUI().AddWidget(&rendererWnd_Toggle);

	postprocessWnd_Toggle.Create("PostProcess");
	postprocessWnd_Toggle.SetTooltip("Postprocess settings");
	postprocessWnd_Toggle.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	postprocessWnd_Toggle.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	postprocessWnd_Toggle.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	postprocessWnd_Toggle.SetSize(XMFLOAT2(120, 20));
	postprocessWnd_Toggle.OnClick([&](wiEventArgs args) {
		postprocessWnd.SetVisible(!postprocessWnd.IsVisible());
	});
	GetGUI().AddWidget(&postprocessWnd_Toggle);

	loadSchBtn.Create("Load schematic");
	loadSchBtn.SetTooltip("Load a schematic from disc to place it in the world");
	loadSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	loadSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	loadSchBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	loadSchBtn.SetSize(XMFLOAT2(120, 20));
	loadSchBtn.OnClick([&](wiEventArgs args) {
		wiHelper::FileDialogParams params;
		params.description = "CyubeVR schematic";
		params.extensions.push_back("cySch");
		params.OPEN;
		wiHelper::FileDialog(params, cySchematic::loadSchematic);
	});
	GetGUI().AddWidget(&loadSchBtn);

	RenderPath3D::Load();
}

void CyRender::Update(float dt) {

	CameraComponent& camera = wiScene::GetCamera();
	// Camera control:
	static XMFLOAT4 originalMouse = XMFLOAT4(0, 0, 0, 0);
	static float Accel			  = 0.0;
	static bool camControlStart	  = true;
	lasttime += dt * 4;
	sinepulse = std::sinf(lasttime);
	if (lasttime > 100) {
		lasttime -= 100;
	}
	if (camControlStart)
	{
		originalMouse = wiInput::GetPointer();
	}

	XMFLOAT4 currentMouse = wiInput::GetPointer();
	float xDif = 0, yDif = 0;

	if (wiInput::Down(wiInput::MOUSE_BUTTON_LEFT) && !GetGUI().GetActiveWidget())
	{
		hovered.entity	= wiECS::INVALID_ENTITY;
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
	} else {
		camControlStart = true;
		wiInput::HidePointer(false);
		if (rendererWnd.GetPickType() == PICK_CHUNK) {

			RAY pickRay = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
			hovered		= wiScene::Pick(pickRay, 1, LAYER_CHUNKMESH);

		} else {

			hovered.entity = wiECS::INVALID_ENTITY;
		}
		if (hovered.entity != wiECS::INVALID_ENTITY)
		{
			const ObjectComponent* object = wiScene::GetScene().objects.GetComponent(hovered.entity);
			const AABB& aabb			  = *wiScene::GetScene().aabb_objects.GetComponent(hovered.entity);
			XMFLOAT4X4 hoverBox;
			XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
			wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.5f, 0.2f, 5.f, 3.0f));
		}
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
