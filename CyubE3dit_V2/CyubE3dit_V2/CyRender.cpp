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
	auto& weather	   = scene.weathers.Create(CreateEntity());
	weather.fogStart   = 5;
	weather.fogEnd	   = 2000;
	weather.fogHeight  = 0;
	weather.horizon	   = XMFLOAT3(.7f, .5f, .8f);
	weather.zenith	   = XMFLOAT3(0.8f, 0.8f, 1.0f);
	weather.ambient	   = XMFLOAT3(1.0f, 1.0f, 1.0f);
	weather.skyMapName = "images/sky.dds";
	weather.SetRealisticSky(false);
	weather.cloudiness	   = 0.2f;
	weather.cloudSpeed	   = 0.1f;
	weather.windSpeed	   = 1.8f;
	weather.windRandomness = 1.5f;
	weather.windWaveSize   = 2.5f;

	weather.windDirection = XMFLOAT3(0.2, 0, 0.2);
	Entity LightEnt		  = scene.Entity_CreateLight("Sunlight", XMFLOAT3(0, 0, 0), XMFLOAT3(1.0, 1.0, 1.0f), 35, 1000);
	LightComponent* light = scene.lights.GetComponent(LightEnt);
	light->SetType(LightComponent::LightType::DIRECTIONAL);
	//light->color				  = XMFLOAT3(1.0f,0.9f,0.7f);
	TransformComponent& transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(0.67f, 0.f, -0.5f));
	transform.SetDirty();
	transform.UpdateTransform();
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
		position.x += (float)(world->m_playerpos.x / 100);
		position.z += (float)(world->m_playerpos.y / 100);
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
		static int64_t id								 = -1;
		static wiECS::Entity oldMat						 = 0;
		static string blockname							 = "";
		wiScene::Scene& scene							 = wiScene::GetScene();
		static wiScene::MaterialComponent* highlightComp = scene.materials.GetComponent(scene.materials.GetIndex(0));
		wiECS::Entity mat								 = 0;
		if (renderer.hovered.entity) {
			wiScene::MeshComponent* mesh = scene.meshes.GetComponent(scene.objects.GetComponent(renderer.hovered.entity)->meshID);
			mat							 = mesh->subsets[renderer.hovered.subsetIndex].materialID;
			XMFLOAT3 pos				 = mesh->vertex_positions[renderer.hovered.vertexID0];
			pos.x += mesh->vertex_positions[renderer.hovered.vertexID1].x;
			pos.y += mesh->vertex_positions[renderer.hovered.vertexID1].y;
			pos.z += mesh->vertex_positions[renderer.hovered.vertexID1].z;
			pos.x += mesh->vertex_positions[renderer.hovered.vertexID2].x;
			pos.y += mesh->vertex_positions[renderer.hovered.vertexID2].y;
			pos.z += mesh->vertex_positions[renderer.hovered.vertexID2].z;
			pos.x /= 1.5;
			pos.y /= 1.5;
			pos.z /= 1.5;
			pos.x -= mesh->vertex_normals[renderer.hovered.vertexID1].x / 2;
			pos.y -= mesh->vertex_normals[renderer.hovered.vertexID1].y / 2;
			pos.z -= mesh->vertex_normals[renderer.hovered.vertexID1].z / 2;
			XMMATRIX sca = XMMatrixScaling(0.26f, 0.26f, 0.26f);
			XMMATRIX tra = XMMatrixTranslation(roundf(pos.x) / 2, roundf(pos.y) / 2, roundf(pos.z) / 2);
			XMFLOAT4X4 hoverBox;
			XMStoreFloat4x4(&hoverBox, sca * tra);
			wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.8f, 0.8f, 2.f, 1.0f));
		}

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

void CyRender::ResizeBuffers() {
	RenderPath3D::ResizeBuffers();

	GraphicsDevice* device = wiRenderer::GetDevice();
	bool hr;
	
	if (GetDepthStencil() != nullptr)
	{
		TextureDesc desc;
		desc.Width	= GetInternalResolution().x;
		desc.Height = GetInternalResolution().y;

		desc.Format	   = FORMAT_R8_UNORM;
		desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
		if (getMSAASampleCount() > 1)
		{
			desc.SampleCount = getMSAASampleCount();
			hr				 = device->CreateTexture(&desc, nullptr, &rt_selectionOutline_MSAA);
			assert(hr);
			desc.SampleCount = 1;
		}
		hr = device->CreateTexture(&desc, nullptr, &rt_selectionOutline[0]);
		assert(hr);
		hr = device->CreateTexture(&desc, nullptr, &rt_selectionOutline[1]);
		assert(hr);

		{
			RenderPassDesc desc;
			desc.attachments.push_back(RenderPassAttachment::RenderTarget(&rt_selectionOutline[0], RenderPassAttachment::LOADOP_CLEAR));
			if (getMSAASampleCount() > 1)
			{
				desc.attachments[0].texture = &rt_selectionOutline_MSAA;
				desc.attachments.push_back(RenderPassAttachment::Resolve(&rt_selectionOutline[0]));
			}
			desc.attachments.push_back(
				RenderPassAttachment::DepthStencil(
					GetDepthStencil(),
					RenderPassAttachment::LOADOP_LOAD,
					RenderPassAttachment::STOREOP_STORE,
					IMAGE_LAYOUT_DEPTHSTENCIL_READONLY,
					IMAGE_LAYOUT_DEPTHSTENCIL_READONLY,
					IMAGE_LAYOUT_DEPTHSTENCIL_READONLY));
			hr = device->CreateRenderPass(&desc, &renderpass_selectionOutline[0]);
			assert(hr);

			if (getMSAASampleCount() == 1)
			{
				desc.attachments[0].texture = &rt_selectionOutline[1];  // rendertarget
			} else
			{
				desc.attachments[1].texture = &rt_selectionOutline[1];  // resolve
			}
			hr = device->CreateRenderPass(&desc, &renderpass_selectionOutline[1]);
			assert(hr);
		}
		{
			RenderPassDesc desc;
			desc.attachments.push_back(RenderPassAttachment::RenderTarget(&rtGbuffer[GBUFFER_COLOR_ROUGHNESS], RenderPassAttachment::LOADOP_LOAD));
			desc.attachments.push_back(
				RenderPassAttachment::DepthStencil(
					&depthBuffer_Main,
					RenderPassAttachment::LOADOP_LOAD,
					RenderPassAttachment::STOREOP_STORE,
					IMAGE_LAYOUT_DEPTHSTENCIL_READONLY,
					IMAGE_LAYOUT_DEPTHSTENCIL,
					IMAGE_LAYOUT_DEPTHSTENCIL_READONLY));
			if (getMSAASampleCount() > 1)
			{
				desc.attachments.push_back(RenderPassAttachment::Resolve(&rtGbuffer_resolved[GBUFFER_COLOR_ROUGHNESS]));
			}
			hr = device->CreateRenderPass(&desc, &renderpass_composeOutline);
			assert(hr);
		}
	}
}


void CyRender::ResizeLayout() {
	RenderPath3D::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
	worldSelector.SetPos(XMFLOAT2((screenW - worldSelector.scale.x) / 2.f, 10));
	label.SetPos(XMFLOAT2((screenW - label.scale.x) / 2, screenH - label.scale.y - 15));
	float xOffset = 15;
	postprocessWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));
	xOffset += 10 + postprocessWnd_Toggle.scale.x;
	rendererWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));
	xOffset += 10 + rendererWnd_Toggle.scale.x;
	loadSchBtn.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));
	viewDist.SetPos(XMFLOAT2(screenW - viewDist.scale.x - 45, screenH - postprocessWnd_Toggle.scale.y - 15));
}

void CyRender::Load() {
	renderPath = std::make_unique<RenderPath3D>();
	hovered = wiScene::PickResult();
	setMotionBlurEnabled(true);
	setMotionBlurStrength(10.0f);
	setBloomEnabled(true);
	setBloomThreshold(1.7f);
	setReflectionsEnabled(true);
	setSSREnabled(false);
	wiRenderer::SetGamma(2.4f);
	wiPhysicsEngine::SetEnabled(true);
	setLightShaftsEnabled(true);
	setColorGradingEnabled(true);
	setShadowsEnabled(true);
	setMSAASampleCount(1);
	setSharpenFilterEnabled(true);
	setSharpenFilterAmount(0.17f);
	wiRenderer::SetTransparentShadowsEnabled(false);
	wiRenderer::SetShadowProps2D(2048, 4);
	setVolumetricCloudsEnabled(false);
	setExposure(1.0f);
	wiRenderer::SetTemporalAAEnabled(true);
	wiRenderer::GetDevice()->SetVSyncEnabled(true);
	wiRenderer::SetVoxelRadianceEnabled(false);
	wiRenderer::SetVoxelRadianceNumCones(2);
	wiRenderer::SetVoxelRadianceRayStepSize(1.0f);
	wiRenderer::SetVoxelRadianceMaxDistance(30);
	wiRenderer::SetVoxelRadianceVoxelSize(0.2f);
	wiRenderer::SetVoxelRadianceSecondaryBounceEnabled(true);
	wiRenderer::SetOcclusionCullingEnabled(false);
	wiProfiler::SetEnabled(true);
	setAO(AO_MSAO);
	setAOPower(0.2);
	setFXAAEnabled(false);
	setEyeAdaptionEnabled(true);
	wiScene::GetCamera().zNearP = 0.1f;
	wiScene::GetCamera().zFarP	= 1500.f;
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
	worldSelector.SetSize(XMFLOAT2(250.f, 20.f));
	worldSelector.SetPos(XMFLOAT2(300.f, 20.f));
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
		wiHelper::FileDialog(params, cySchematic::addSchematic);
	});
	GetGUI().AddWidget(&loadSchBtn);

	RenderPath3D::Load();
	ResizeBuffers();
}

void CyRender::Update(float dt) {
	static wiECS::Entity lastHovered = wiECS::INVALID_ENTITY;
	CameraComponent& camera			 = wiScene::GetCamera();
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
		if (rendererWnd.GetPickType()) {
			RAY pickRay		  = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
			uint32_t pickmask = 0;
			if (rendererWnd.GetPickType() & PICK_CHUNK)
				pickmask |= LAYER_CHUNKMESH;
			if (rendererWnd.GetPickType() & PICK_TREE)
				pickmask |= LAYER_TREE;
			hovered = wiScene::Pick(pickRay, 1, pickmask);

		} else {
			hovered.entity = wiECS::INVALID_ENTITY;
		}
		if (hovered.entity != wiECS::INVALID_ENTITY)
		{
			ObjectComponent* object = wiScene::GetScene().objects.GetComponent(hovered.entity);
			const AABB& aabb		= *wiScene::GetScene().aabb_objects.GetComponent(hovered.entity);
			object->color			= XMFLOAT4((3.0 + sinepulse) / 4, (3.0 + sinepulse) / 4, (3.0 + sinepulse) / 4, 1);
			XMFLOAT4X4 hoverBox;
			XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
			wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.5f, 1.0f, 1.f, 1.0f));
		}
		if (lastHovered != hovered.entity) {
			ObjectComponent* object = wiScene::GetScene().objects.GetComponent(lastHovered);
			if (object != nullptr) {
				object->color = XMFLOAT4(1, 1, 1, 1);
			}
			lastHovered = hovered.entity;
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
	const float MoveSpeed = settings::camspeed * Accel;

	// FPS Camera
	const float clampedDT = min(dt, 0.15f);	// if dt > 100 millisec, don't allow the camera to jump too far...

	const float speed	 = ((wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0 : 1.0) + rightTrigger.x * 10.0f) * MoveSpeed * clampedDT;
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
		CAPSULE capsule;
		capsule.radius = 0.6;
		capsule.tip	   = camera.Eye;
		capsule.base   = capsule.tip;
		capsule.base.y -= 1.5f;
		

		camera_transform.MatrixTransform(wiScene::GetCamera().GetInvView());
		camera_transform.UpdateTransform();
		XMMATRIX camRot	  = XMMatrixRotationQuaternion(XMLoadFloat4(&camera_transform.rotation_local));
		XMVECTOR move_rot = XMVector3TransformNormal(move, camRot);
		/*
		if (settings::collision == true && !wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT)) {
			int ccd_max	  = 20;
			XMVECTOR step = XMVectorSet(0, 0, 0, 0);
			SPHERE sphere;
			sphere.center = camera_transform.GetPosition();
			sphere.radius = 0.22;
			for (int ccd = 0; ccd < ccd_max; ++ccd)
			{
				step = move_rot / ccd_max;
				//step = step * 0.016;
				XMFLOAT3 cTransf;
				XMStoreFloat3(&cTransf, step);
				sphere.center.x += cTransf.x;
				sphere.center.y += cTransf.y;
				sphere.center.z += cTransf.z;
				wiScene::SceneIntersectSphereResult intersect = SceneIntersectSphere(sphere, 1);
				if (intersect.entity)
				{
					// Modify player velocity to slide on contact surface:
					XMVECTOR velocity_length	 = XMVector3Length(move_rot);
					XMVECTOR velocity_normalized = XMVector3Normalize(move_rot);
					XMVECTOR collisionNormal	 = XMLoadFloat3(&(intersect.normal));
					XMVECTOR undesired_motion	 = collisionNormal * XMVector3Dot(velocity_normalized, collisionNormal);
					XMVECTOR desired_motion		 = XMVectorSubtract(velocity_normalized, undesired_motion);
					move_rot					 = desired_motion * velocity_length;
					// Remove penetration (penetration epsilon added to handle infinitely small penetration):
					XMStoreFloat3(&cTransf, (collisionNormal * (intersect.depth + 0.0001)));
					sphere.center.x += cTransf.x;
					sphere.center.y += cTransf.y;
					sphere.center.z += cTransf.z;
				}
			}
		}
		*/
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
			probeT->SetDirty();
			probeT->UpdateTransform();
		}
	}
	RenderPath3D::Update(dt);
}

void CyRender::Compose(wiGraphics::CommandList cmd) const {
	RenderPath3D::Compose(cmd);
}

void CyRender::Render() const {

	GraphicsDevice* device = wiRenderer::GetDevice();
	wiJobSystem::context ctx;
	CommandList cmd;

	// Preparing the frame:
	cmd = device->BeginCommandList();
	wiJobSystem::Execute(ctx, [this, cmd](wiJobArgs args) {
		RenderFrameSetUp(cmd);
	});

	static const uint32_t drawscene_flags =
		wiRenderer::DRAWSCENE_OPAQUE |
		wiRenderer::DRAWSCENE_HAIRPARTICLE |
		wiRenderer::DRAWSCENE_TESSELLATION |
		wiRenderer::DRAWSCENE_OCCLUSIONCULLING;
	static const uint32_t drawscene_flags_reflections =
		wiRenderer::DRAWSCENE_OPAQUE;

	// Depth prepass + Occlusion culling + AO:
	cmd = device->BeginCommandList();
	wiJobSystem::Execute(ctx, [this, cmd](wiJobArgs args) {
		GraphicsDevice* device = wiRenderer::GetDevice();

		wiRenderer::UpdateCameraCB(
			*camera,
			camera_previous,
			camera_reflection,
			cmd);

		device->RenderPassBegin(&renderpass_depthprepass, cmd);

		device->EventBegin("Opaque Z-prepass", cmd);
		auto range = wiProfiler::BeginRangeGPU("Z-Prepass", cmd);

		Viewport vp;
		vp.Width  = (float)depthBuffer_Main.GetDesc().Width;
		vp.Height = (float)depthBuffer_Main.GetDesc().Height;
		device->BindViewports(1, &vp, cmd);
		wiRenderer::DrawScene(visibility_main, RENDERPASS_DEPTHONLY, cmd, drawscene_flags);

		wiProfiler::EndRange(range);
		device->EventEnd(cmd);

		wiRenderer::OcclusionCulling_Render(*camera, visibility_main, cmd);

		device->RenderPassEnd(cmd);

		// Make a readable copy for depth buffer:
		if (getMSAASampleCount() > 1)
		{
			{
				GPUBarrier barriers[] = {
					GPUBarrier::Image(&depthBuffer_Main, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY, IMAGE_LAYOUT_SHADER_RESOURCE),
					GPUBarrier::Image(&depthBuffer_Copy, IMAGE_LAYOUT_SHADER_RESOURCE, IMAGE_LAYOUT_UNORDERED_ACCESS)};
				device->Barrier(barriers, arraysize(barriers), cmd);
			}

			wiRenderer::ResolveMSAADepthBuffer(depthBuffer_Copy, depthBuffer_Main, cmd);

			{
				GPUBarrier barriers[] = {
					GPUBarrier::Image(&depthBuffer_Main, IMAGE_LAYOUT_SHADER_RESOURCE, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY),
					GPUBarrier::Image(&depthBuffer_Copy, IMAGE_LAYOUT_UNORDERED_ACCESS, IMAGE_LAYOUT_SHADER_RESOURCE)};
				device->Barrier(barriers, arraysize(barriers), cmd);
			}
		} else
		{
			{
				GPUBarrier barriers[] = {
					GPUBarrier::Image(&depthBuffer_Main, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY, IMAGE_LAYOUT_COPY_SRC),
					GPUBarrier::Image(&depthBuffer_Copy, IMAGE_LAYOUT_SHADER_RESOURCE, IMAGE_LAYOUT_COPY_DST)};
				device->Barrier(barriers, arraysize(barriers), cmd);
			}

			device->CopyResource(&depthBuffer_Copy, &depthBuffer_Main, cmd);

			{
				GPUBarrier barriers[] = {
					GPUBarrier::Image(&depthBuffer_Main, IMAGE_LAYOUT_COPY_SRC, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY),
					GPUBarrier::Image(&depthBuffer_Copy, IMAGE_LAYOUT_COPY_DST, IMAGE_LAYOUT_SHADER_RESOURCE)};
				device->Barrier(barriers, arraysize(barriers), cmd);
			}
		}

		wiRenderer::Postprocess_Lineardepth(depthBuffer_Copy, rtLinearDepth, cmd);

		RenderAO(cmd);

		if (wiRenderer::GetVariableRateShadingClassification() && device->CheckCapability(GRAPHICSDEVICE_CAPABILITY_VARIABLE_RATE_SHADING_TIER2))
		{
			wiRenderer::ComputeShadingRateClassification(
				GetGbuffer_Read(),
				rtLinearDepth,
				rtShadingRate,
				debugUAV,
				cmd);
		}
	});

	// Planar reflections depth prepass + Light culling:
	if (visibility_main.IsRequestedPlanarReflections())
	{
		cmd = device->BeginCommandList();
		wiJobSystem::Execute(ctx, [cmd, this](wiJobArgs args) {
			GraphicsDevice* device = wiRenderer::GetDevice();

			wiRenderer::UpdateCameraCB(
				camera_reflection,
				camera_reflection,
				camera_reflection,
				cmd);

			device->EventBegin("Planar reflections Z-Prepass", cmd);
			auto range = wiProfiler::BeginRangeGPU("Planar Reflections Z-Prepass", cmd);

			Viewport vp;
			vp.Width  = (float)depthBuffer_Reflection.GetDesc().Width;
			vp.Height = (float)depthBuffer_Reflection.GetDesc().Height;
			device->BindViewports(1, &vp, cmd);

			device->RenderPassBegin(&renderpass_reflection_depthprepass, cmd);

			wiRenderer::DrawScene(visibility_reflection, RENDERPASS_DEPTHONLY, cmd, drawscene_flags_reflections);

			device->RenderPassEnd(cmd);

			wiProfiler::EndRange(range);  // Planar Reflections
			device->EventEnd(cmd);

			wiRenderer::ComputeTiledLightCulling(
				depthBuffer_Reflection,
				tileFrustums,
				entityTiles_Opaque,
				entityTiles_Transparent,
				debugUAV,
				cmd);
		});
	}

	// Shadow maps:
	if (getShadowsEnabled() && !wiRenderer::GetRaytracedShadowsEnabled())
	{
		cmd = device->BeginCommandList();
		wiJobSystem::Execute(ctx, [this, cmd](wiJobArgs args) {
			wiRenderer::DrawShadowmaps(visibility_main, cmd);
		});
	}

	// Updating textures:
	cmd = device->BeginCommandList();
	wiJobSystem::Execute(ctx, [cmd, this](wiJobArgs args) {
		wiRenderer::BindCommonResources(cmd);
		wiRenderer::RefreshDecalAtlas(*scene, cmd);
		wiRenderer::RefreshLightmapAtlas(*scene, cmd);
		wiRenderer::RefreshEnvProbes(visibility_main, cmd);
		wiRenderer::RefreshImpostors(*scene, cmd);
	});

	// Voxel GI:
	if (wiRenderer::GetVoxelRadianceEnabled())
	{
		cmd = device->BeginCommandList();
		wiJobSystem::Execute(ctx, [cmd, this](wiJobArgs args) {
			wiRenderer::VoxelRadiance(visibility_main, cmd);
		});
	}

	// Planar reflections:
	if (visibility_main.IsRequestedPlanarReflections())
	{
		cmd = device->BeginCommandList();
		wiJobSystem::Execute(ctx, [cmd, this](wiJobArgs args) {
			GraphicsDevice* device = wiRenderer::GetDevice();

			wiRenderer::UpdateCameraCB(
				camera_reflection,
				camera_reflection,
				camera_reflection,
				cmd);

			device->EventBegin("Planar reflections", cmd);
			auto range = wiProfiler::BeginRangeGPU("Planar Reflections", cmd);

			Viewport vp;
			vp.Width  = (float)depthBuffer_Reflection.GetDesc().Width;
			vp.Height = (float)depthBuffer_Reflection.GetDesc().Height;
			device->BindViewports(1, &vp, cmd);

			GPUBarrier barriers[] = {
				GPUBarrier::Memory(&entityTiles_Opaque),
			};
			device->Barrier(barriers, arraysize(barriers), cmd);
			device->UnbindResources(TEXSLOT_DEPTH, 1, cmd);

			device->RenderPassBegin(&renderpass_reflection, cmd);

			device->BindResource(PS, &entityTiles_Opaque, TEXSLOT_RENDERPATH_ENTITYTILES, cmd);
			device->BindResource(PS, wiTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_REFLECTION, cmd);
			device->BindResource(PS, wiTextureHelper::getWhite(), TEXSLOT_RENDERPATH_AO, cmd);
			device->BindResource(PS, wiTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_SSR, cmd);
			wiRenderer::DrawScene(visibility_reflection, RENDERPASS_MAIN, cmd, drawscene_flags_reflections);
			wiRenderer::DrawSky(*scene, cmd);

			device->RenderPassEnd(cmd);

			wiProfiler::EndRange(range);  // Planar Reflections
			device->EventEnd(cmd);
		});
	}

	// Opaque scene + Light culling:
	cmd = device->BeginCommandList();
	wiJobSystem::Execute(ctx, [this, cmd](wiJobArgs args) {
		GraphicsDevice* device = wiRenderer::GetDevice();
		device->EventBegin("Opaque Scene", cmd);

		wiRenderer::UpdateCameraCB(
			*camera,
			camera_previous,
			camera_reflection,
			cmd);

		device->UnbindResources(TEXSLOT_ONDEMAND0, 1, cmd);

		{
			auto range = wiProfiler::BeginRangeGPU("Entity Culling", cmd);
			wiRenderer::ComputeTiledLightCulling(
				depthBuffer_Copy,
				tileFrustums,
				entityTiles_Opaque,
				entityTiles_Transparent,
				debugUAV,
				cmd);
			GPUBarrier barriers[] = {
				GPUBarrier::Memory(&entityTiles_Opaque),
			};
			device->Barrier(barriers, arraysize(barriers), cmd);
			wiProfiler::EndRange(range);
		}

		device->RenderPassBegin(&renderpass_main, cmd);

		auto range = wiProfiler::BeginRangeGPU("Opaque Scene", cmd);

		Viewport vp;
		vp.Width  = (float)depthBuffer_Main.GetDesc().Width;
		vp.Height = (float)depthBuffer_Main.GetDesc().Height;
		device->BindViewports(1, &vp, cmd);

		device->BindResource(PS, &entityTiles_Opaque, TEXSLOT_RENDERPATH_ENTITYTILES, cmd);
		device->BindResource(PS, getReflectionsEnabled() ? &rtReflection : wiTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_REFLECTION, cmd);
		device->BindResource(PS, getAOEnabled() ? &rtAO : wiTextureHelper::getWhite(), TEXSLOT_RENDERPATH_AO, cmd);
		device->BindResource(PS, getSSREnabled() || getRaytracedReflectionEnabled() ? &rtSSR : wiTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_SSR, cmd);
		wiRenderer::DrawScene(visibility_main, RENDERPASS_MAIN, cmd, drawscene_flags);
		wiRenderer::DrawSky(*scene, cmd);

		wiProfiler::EndRange(range);  // Opaque Scene

		RenderOutline(cmd);

		

		device->RenderPassEnd(cmd);

		device->EventEnd(cmd);

		// Selection outline:

		if (GetDepthStencil() != nullptr)
		{
			//Viewport vp;
			// We will specify the stencil ref in user-space, don't care about engine stencil refs here:
			//	Otherwise would need to take into account engine ref and draw multiple permutations of stencil refs.
			vp.Width  = (float)rt_selectionOutline[0].GetDesc().Width;
			vp.Height = (float)rt_selectionOutline[0].GetDesc().Height;
			device->BindViewports(1, &vp, cmd);

			wiImageParams fx;
			fx.enableFullScreen();
			fx.stencilComp	  = STENCILMODE::STENCILMODE_EQUAL;
			fx.stencilRefMode = STENCILREFMODE_USER;

			// Materials outline:
			{
				device->RenderPassBegin(&renderpass_selectionOutline[0], cmd);

				// Draw solid blocks of selected materials
				fx.stencilRef = EDITORSTENCILREF_HIGHLIGHT_MATERIAL;
				wiImage::Draw(wiTextureHelper::getWhite(), fx, cmd);

				device->RenderPassEnd(cmd);
			}

			// Objects outline:
			{
				device->UnbindResources(TEXSLOT_ONDEMAND0, 1, cmd);
				device->RenderPassBegin(&renderpass_selectionOutline[1], cmd);

				// Draw solid blocks of selected objects
				fx.stencilRef = EDITORSTENCILREF_HIGHLIGHT_OBJECT;
				wiImage::Draw(wiTextureHelper::getWhite(), fx, cmd);

				device->RenderPassEnd(cmd);
			}
			device->RenderPassBegin(&renderpass_composeOutline, cmd);
			vp.Width  = (float)depthBuffer_Main.GetDesc().Width;
			vp.Height = (float)depthBuffer_Main.GetDesc().Height;
			device->BindViewports(1, &vp, cmd);

			const float selectionColorIntensity = sinepulse * 0.5f + 0.5f;
			float opacity						= wiMath::Lerp(0.0f, 0.3f, selectionColorIntensity);
			XMFLOAT4 col						= selectionColor2;
			col.w *= opacity;
			wiRenderer::Postprocess_Outline(rt_selectionOutline[0], cmd, 0.1f, 0.5f, col);
			col = selectionColor;
			col.w *= opacity;
			wiRenderer::Postprocess_Outline(rt_selectionOutline[1], cmd, 0.1f, 0.5f, col);
			device->RenderPassEnd(cmd);
		}
		//END selection_outline
		
	});

	// Transparents, post processes, etc:
	cmd = device->BeginCommandList();
	wiJobSystem::Execute(ctx, [this, cmd](wiJobArgs args) {
		GraphicsDevice* device = wiRenderer::GetDevice();

		wiRenderer::UpdateCameraCB(
			*camera,
			camera_previous,
			camera_reflection,
			cmd);
		wiRenderer::BindCommonResources(cmd);

		RenderLightShafts(cmd);

		RenderVolumetrics(cmd);

		RenderSceneMIPChain(cmd);

		RenderSSR(cmd);

		RenderTransparents(cmd);


		RenderPostprocessChain(cmd);
	});

	RenderPath2D::Render();

	wiJobSystem::Wait(ctx);
}
