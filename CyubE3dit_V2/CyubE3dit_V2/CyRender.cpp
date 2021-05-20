
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
wiECS::Entity CyMainComponent::m_posLight  = 0;
wiECS::Entity CyMainComponent::m_probe	   = 0;
wiECS::Entity CyMainComponent::m_dust	   = 0;
wiAudio::Sound CyRender::fireSound;
wiAudio::SoundInstance CyRender::fireSoundinstance[NUM_TORCHSOUNDS];
wiAudio::Sound CyRender::windSound;
wiAudio::SoundInstance CyRender::windSoundinstance;
wiAudio::Sound CyRender::bgSound;
wiAudio::SoundInstance CyRender::bgSoundinstance;
std::vector<wiECS::Entity> CyRender::m_selectedObjects;
bool CyRender::fireSoundIsPlaying[]	 = {false};
bool CyRender::anyfireSoundIsPlaying = false;

void CyMainComponent::Initialize() {
	settings::load();
	MainComponent::Initialize();
	infoDisplay.active = false;
	//renderer.Load();
	//pathRenderer.Load();
	loader.Load();
	renderer.main		   = this;
	renderer.m_soundLoaded = false;
	loader.addLoadingComponent(&renderer, this, 0.5f, 0xFFFFFFFF);
	loader.addLoadingFunction([this](wiJobArgs) {
		LoadSound();
	});
	loader.addLoadingFunction([this](wiJobArgs) {
		cyBlocks::LoadRegBlocks();
		cyBlocks::LoadCustomBlocks();
		cyBlocks::loadMeshes();
		CreateScene();
	});
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

void CyMainComponent::LoadSound(void) {
	renderer.m_soundLoaded = false;
	wiAudio::SetSubmixVolume(wiAudio::SUBMIX_TYPE_MUSIC, settings::musicVol);
	wiAudio::SetSubmixVolume(wiAudio::SUBMIX_TYPE_SOUNDEFFECT, settings::effectVol);
	if (wiAudio::CreateSound("sound/fireloop3.ogg", &renderer.fireSound)) {
		for (uint8_t i = 0; i < renderer.NUM_TORCHSOUNDS; i++) {
			renderer.fireSoundinstance[i].type = wiAudio::SUBMIX_TYPE_SOUNDEFFECT;
			wiAudio::CreateSoundInstance(&renderer.fireSound, &(renderer.fireSoundinstance[i]));
			//fireSoundinstance[i].loop_begin = (float)i ;
			wiAudio::SetVolume(0.1, &(renderer.fireSoundinstance[i]));
		}
	} else {
		settings::sound = false;
		return;
	}

	//wiAudio::CreateSound("sound/background.ogg", &bgSound);

	if (wiAudio::CreateSound("sound/02 - Ingame.mod", &renderer.bgSound)) {
		renderer.bgSoundinstance.type = wiAudio::SUBMIX_TYPE_MUSIC;
		wiAudio::CreateSoundInstance(&renderer.bgSound, &renderer.bgSoundinstance);
		wiAudio::SetVolume(1., &renderer.bgSoundinstance);
	} else {
		settings::sound = false;
		return;
	}

	if (wiAudio::CreateSound("sound/wind2.ogg", &renderer.windSound)) {
		renderer.windSoundinstance.type = wiAudio::SUBMIX_TYPE_SOUNDEFFECT;
		wiAudio::CreateSoundInstance(&renderer.windSound, &renderer.windSoundinstance);
		renderer.windSoundinstance.loop_begin = 32.7;
		wiAudio::SetVolume(0.3, &renderer.windSoundinstance);
	} else {
		settings::sound = false;
		return;
	}
	if (settings::sound) {
		wiAudio::Play(&renderer.bgSoundinstance);
		wiAudio::Play(&renderer.windSoundinstance);
	}
	renderer.m_soundLoaded = true;
}

void CyMainComponent::CreateScene(void) {
	Scene& scene = wiScene::GetScene();
	settings::rendermask |= LAYER_CHUNKMESH;  //enable at least the chunk visibility after restart, otherwise it's weired....
	renderer.setlayerMask(settings::rendermask);
	wiScene::GetScene().weather = WeatherComponent();

	//if (wiLua::GetLuaState() != nullptr) {
	//	wiLua::KillProcesses();
	//}
	// Add some nice weather, not just black:
	auto& weather	   = scene.weathers.Create(CreateEntity());
	weather.fogStart   = 50;
	weather.fogEnd	   = 650;
	weather.fogHeight  = 0;
	weather.horizon	   = XMFLOAT3(0.9f, 0.9f, .9f);	 //XMFLOAT3(.2f, .2f, .3f);	  //XMFLOAT3(0.9f, 0.9f, 1.f);
	weather.zenith	   = XMFLOAT3(0.9f, 0.9f, .8f);	 //XMFLOAT3(0.2f, 0.2f, 0.3f);	 //XMFLOAT3(0.9f, 0.9f, 1.f);
	weather.ambient	   = XMFLOAT3(.9f, .9f, 1.f);	 //XMFLOAT3(.9f, .9f, .9f);  //XMFLOAT3(.5f, .6f, .7f);
	weather.skyMapName = "images/sky.dds";
	weather.skyMap	   = wiResourceManager::Load("images/sky.dds");
	weather.SetRealisticSky(false);
	weather.cloudiness			= 0.3f;
	weather.cloudSpeed			= 0.02f;
	weather.colorGradingMapName = "images/colorGrading.png";
	weather.colorGradingMap		= wiResourceManager::Load("images/colorGrading.png", wiResourceManager::IMPORT_COLORGRADINGLUT | wiResourceManager::IMPORT_RETAIN_FILEDATA);

	weather.windSpeed	   = 2.5f;
	weather.windRandomness = 1.5f;
	weather.windWaveSize   = 0.2f;

	weather.windDirection = XMFLOAT3(0.2, 0, 0.2);
	Entity LightEnt		  = scene.Entity_CreateLight("Sunlight", XMFLOAT3(0, 0, 0), XMFLOAT3(0.9f, 0.9f, .9f), 9, 100);
	LightComponent* light = scene.lights.GetComponent(LightEnt);
	light->SetType(LightComponent::LightType::DIRECTIONAL);
	//light->color				  = XMFLOAT3(1.0f,0.9f,0.7f);
	TransformComponent& transform = *scene.transforms.GetComponent(LightEnt);
	transform.RotateRollPitchYaw(XMFLOAT3(0.67f, 0.f, -0.5f));
	transform.SetDirty();
	transform.UpdateTransform();
	light->SetCastShadow(true);
	//light->SetVolumetricsEnabled(true);
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
	m_posLight															  = wiScene::GetScene().Entity_CreateLight("Poslight", XMFLOAT3(0, 0, 0), XMFLOAT3(0.2f, 1.f, 0.4f), 50, 400, LightComponent::LightType::SPOT);
	light																  = wiScene::GetScene().lights.GetComponent(m_posLight);
	light->SetVolumetricsEnabled(true);
	light->SetStatic(false);
	light->SetCastShadow(false);
	light->fov = 0.2f;

	m_dust							 = wiScene::GetScene().Entity_CreateEmitter("Dust", XMFLOAT3(0, 10, 0));
	wiScene::wiEmittedParticle* dust = wiScene::GetScene().emitters.GetComponent(m_dust);
	dust->SetVolumeEnabled(true);
	dust->shaderType = wiScene::wiEmittedParticle::SOFT;
	dust->SetMaxParticleCount(4096);
	dust->life			= 8.0;
	dust->random_life	= 2.f;
	dust->random_factor = 1.f;
	dust->count			= 2048;
	dust->normal_factor = 0.4f;
	dust->size			= .002f;
	dust->velocity		= XMFLOAT3(weather.windDirection.x * weather.windSpeed / 3, 0, weather.windDirection.z * weather.windSpeed / 3);
	//dust->drag													= 0.91;
	//dust->velocity												= XMFLOAT3(weather.windDirection.x * weather.windSpeed/4 , -1, weather.windDirection.z * weather.windSpeed/4 );
	//dust->gravity												= XMFLOAT3(weather.windDirection.x * weather.windSpeed, -(7.f - weather.windSpeed/6), weather.windDirection.z * weather.windSpeed);
	dust->random_color = 0.1f;
	//dust->motionBlurAmount										= 10;

	/*
	dust->mass													= 1.7;
	dust->SPH_h													= 20.0f;
	dust->SPH_K													= 0.1;
	dust->SPH_p0												= 15;
	dust->SPH_e													= 4;

	dust->SetSPHEnabled(true);
		*/

	dust->SetDepthCollisionEnabled(false);
	wiScene::MaterialComponent* dustmat							= wiScene::GetScene().materials.GetComponent(m_dust);
	dustmat->textures[MaterialComponent::BASECOLORMAP].resource = cyBlocks::emitter_dust_material;

	/*FIREFLIES:
	dustmat->userBlendMode										= BLENDMODE_ADDITIVE;
	dustmat->SetBaseColor(XMFLOAT4(1, 0.5, 0, 1.));
	dustmat->SetEmissiveColor(XMFLOAT4(1, 1, 1, 1));
	dustmat->SetEmissiveStrength(50.f);
	*/
	infoDisplay.active					= false;
	infoDisplay.watermark				= true;
	infoDisplay.resolution				= true;
	infoDisplay.fpsinfo					= true;
	infoDisplay.heap_allocation_counter = true;
	infoDisplay.colorgrading_helper		= true;
	m_probe								= wiScene::GetScene().Entity_CreateEnvironmentProbe("", XMFLOAT3(0.0f, 0.0f, 0.0f));
	cyImportant* world					= settings::getWorld();
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

	GraphicsDevice* device = wiRenderer::GetDevice();

	if (fadeManager.IsActive())
	{
		// display fade rect
		static wiImageParams fx;
		fx.siz.x   = (float)device->GetScreenWidth();
		fx.siz.y   = (float)device->GetScreenHeight();
		fx.opacity = fadeManager.opacity;
		wiImage::Draw(wiTextureHelper::getColor(fadeManager.color), fx, cmd);
	}
	if (cySchematic::m_schematics.size()) {
		wiImageParams fx;
		fx.pos.x = (float)wiRenderer::GetDevice()->GetScreenWidth() - 430;
		fx.pos.y = (float)wiRenderer::GetDevice()->GetScreenHeight() - 200;
		fx.siz.x = (float)400;
		fx.siz.y = (float)150;
		fx.color = wiColor(20, 20, 20, 200);
		wiImage::Draw(wiTextureHelper::getWhite(), fx, cmd);

		stringstream ss("");
		ss << "---- Schematic info: ----" << endl;
		ss << "Size:" << endl;
		ss << "    X:" << cySchematic::m_schematics[0]->size.x << " Y:" << cySchematic::m_schematics[0]->size.y << " Z:" << cySchematic::m_schematics[0]->size.z << " m" << endl;
		ss << "    X:" << cySchematic::m_schematics[0]->size.x * 2 << " Y:" << cySchematic::m_schematics[0]->size.y * 2 << " Z:" << cySchematic::m_schematics[0]->size.z * 2 << " Blocks" << endl;
		ss << "World Position:" << endl;
		ss << "    X:" << cySchematic::m_schematics[0]->pos.x + roundf(settings::getWorld()->m_playerpos.x / 100) << " Y : " << cySchematic::m_schematics[0]->pos.y + roundf(settings::getWorld()->m_playerpos.y / 100) << " Z : " << cySchematic::m_schematics[0]->pos.z << " m" << endl;
		ss.precision(1);
		wiFont::Draw(ss.str(), wiFontParams(wiRenderer::GetDevice()->GetScreenWidth() - 30, wiRenderer::GetDevice()->GetScreenHeight() - 30, 20, WIFALIGN_RIGHT, WIFALIGN_BOTTOM, wiColor(255, 255, 255, 255), wiColor(0, 0, 0, 255)), cmd);
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
			if (dynamic_cast<GraphicsDevice_DX11*>(device))
			{
				ss << "[DX11]";
			}
#endif
#ifdef WICKEDENGINE_BUILD_DX12
			if (dynamic_cast<GraphicsDevice_DX12*>(device))
			{
				ss << "[DX12]";
			}
#endif
#ifdef WICKEDENGINE_BUILD_VULKAN
			if (dynamic_cast<GraphicsDevice_Vulkan*>(device))
			{
				ss << "[Vulkan]";
			}
#endif

#ifdef _DEBUG
			ss << "[DEBUG]";
#endif
			if (device->IsDebugDevice())
			{
				ss << "[debugdevice]";
			}
			ss << endl;
		}
		if (infoDisplay.resolution)
		{
			ss << "Resolution: " << device->GetResolutionWidth() << " x " << device->GetResolutionHeight() << " (" << device->GetDPI() << " dpi)" << endl;
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
		if (infoDisplay.chunkinfo)
		{
			ss << "Entity pool usage: " + to_string((((float)wiECS::next * 100.0f) / UINT32_MAX)) + "%" << endl;
			static int64_t id								 = -1;
			static wiECS::Entity oldMat						 = 0;
			static string blockname							 = "";
			wiScene::Scene& scene							 = wiScene::GetScene();
			static wiScene::MaterialComponent* highlightComp = scene.materials.GetComponent(scene.materials.GetIndex(0));
			wiECS::Entity mat								 = 0;
			if (renderer.hovered.entity) {
				wiScene::ObjectComponent* obj = scene.objects.GetComponent(renderer.hovered.entity);
				if (obj != nullptr && id < 256) {
					wiScene::MeshComponent* mesh = scene.meshes.GetComponent(obj->meshID);
					mat							 = mesh->subsets[renderer.hovered.subsetIndex].materialID;
					if (cyBlocks::m_regBlockTypes[id] != cyBlocks::BLOCKTYPE_BILLBOARD) {
						XMFLOAT3 pos = mesh->vertex_positions[renderer.hovered.vertexID0];
						pos.x		 = roundf(renderer.hovered.position.x * 2) * 0.5;  //mesh->vertex_positions[renderer.hovered.vertexID1].x;
						pos.y		 = roundf(renderer.hovered.position.y * 2) * 0.5;  //mesh->vertex_positions[renderer.hovered.vertexID1].y;
						pos.z		 = roundf(renderer.hovered.position.z * 2) * 0.5;  //mesh->vertex_positions[renderer.hovered.vertexID1].z;
						//pos.x += //mesh->vertex_positions[renderer.hovered.vertexID2].x;
						//pos.y += //mesh->vertex_positions[renderer.hovered.vertexID2].y;
						//pos.z += //mesh->vertex_positions[renderer.hovered.vertexID2].z;
						//pos.x /= 1.5;
						//pos.y /= 1.5;
						//pos.z /= 1.5;
						pos.x -= mesh->vertex_normals[renderer.hovered.vertexID1].x * 0.5;
						pos.y -= mesh->vertex_normals[renderer.hovered.vertexID1].y * 0.5;
						pos.z -= mesh->vertex_normals[renderer.hovered.vertexID1].z * 0.5;
						XMMATRIX sca = XMMatrixScaling(0.26f, 0.26f, 0.26f);
						XMMATRIX tra = XMMatrixTranslation(pos.x, pos.y, pos.z);
						XMFLOAT4X4 hoverBox;
						XMStoreFloat4x4(&hoverBox, sca * tra);
						wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.8f, 0.8f, 2.f, 1.0f));
					}
				} else {
					renderer.hovered.entity = wiECS::INVALID_ENTITY;
				}
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
							}
						}
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
								}
							}
						}
					}
				}
			}
			if (renderer.hovered.entity != wiECS::INVALID_ENTITY) {
				ss << "Hovered Chunk: " + scene.names.GetComponent(renderer.hovered.entity)->name << endl;
				//highlightComp->SetEmissiveColor(XMFLOAT4(0.0f, 0.0f, 1.0f, 0.1f * renderer.sinepulse));
				ss << "Hovered Block: " + blockname + "(ID " + to_string(id) + ")" << endl;
			}

			PROCESS_MEMORY_COUNTERS_EX pmc;
			GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
			ss << "RAM used: " + to_string((pmc.PrivateUsage - pmc.WorkingSetSize) / 1000000) + " MB" << endl;
		}
		ss.precision(2);
		if (infoDisplay.colorgrading_helper)
		{
			wiImage::Draw(wiTextureHelper::getColorGradeDefault(), wiImageParams(0, 0, 256.0f / device->GetDPIScaling(), 16.0f / device->GetDPIScaling()), cmd);
			wiFont::Draw(ss.str(), wiFontParams(4, 40, infoDisplay.size, WIFALIGN_LEFT, WIFALIGN_TOP, wiColor(255, 255, 255, 255), wiColor(0, 0, 0, 255)), cmd);
		} else {
			wiFont::Draw(ss.str(), wiFontParams(4, 4, infoDisplay.size, WIFALIGN_LEFT, WIFALIGN_TOP, wiColor(255, 255, 255, 255), wiColor(0, 0, 0, 255)), cmd);
		}
	}

	wiProfiler::DrawData(4, 180, cmd);

	wiBackLog::Draw(cmd);

	wiProfiler::EndRange(range);  // Compose
}
/*
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
				desc.attachments[0].texture = &rt_selectionOutline[1];	// rendertarget
			} else
			{
				desc.attachments[1].texture = &rt_selectionOutline[1];	// resolve
			}
			hr = device->CreateRenderPass(&desc, &renderpass_selectionOutline[1]);
			assert(hr);
		}
		//{
		//	RenderPassDesc desc;
		//	desc.attachments.push_back(RenderPassAttachment::RenderTarget(&rtGbuffer[GBUFFER_COLOR_ROUGHNESS], RenderPassAttachment::LOADOP_LOAD));
		//	desc.attachments.push_back(
		//		RenderPassAttachment::DepthStencil(
		//			&depthBuffer_Main,
		//			RenderPassAttachment::LOADOP_LOAD,
		//			RenderPassAttachment::STOREOP_STORE,
		//			IMAGE_LAYOUT_DEPTHSTENCIL_READONLY,
		//			IMAGE_LAYOUT_DEPTHSTENCIL,
		//			IMAGE_LAYOUT_DEPTHSTENCIL_READONLY));
		//	if (getMSAASampleCount() > 1)
		//	{
		//		desc.attachments.push_back(RenderPassAttachment::Resolve(&rtGbuffer_resolved[GBUFFER_COLOR_ROUGHNESS]));
		//	}
		//	hr = device->CreateRenderPass(&desc, &renderpass_composeOutline);
		//	assert(hr);
		//}
	}
}
*/

void CyRender::ResizeLayout() {
	RenderPath3D::ResizeLayout();

	float screenW = wiRenderer::GetDevice()->GetScreenWidth();
	float screenH = wiRenderer::GetDevice()->GetScreenHeight();
	worldSelector.SetPos(XMFLOAT2((screenW - worldSelector.scale.x) / 2.f, 15));
	camModeSelector.SetPos(XMFLOAT2(screenW - camModeSelector.scale.x - 20, 15));
	label.SetPos(XMFLOAT2((screenW - label.scale.x) / 2, screenH - label.scale.y - 15));

	float xOffset = 15;
	loadSchBtn.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y * 2 - 20));
	saveSchBtn.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y - 15));
	reposSchBtn.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y - 15));
	xOffset += 10 + max(reposSchBtn.scale.x, loadSchBtn.scale.x);

	treeDelBtn.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y - 15));
	treeDelRstBtn.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y * 2 - 20));
	treeDelGoBtn.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y * 3 - 25));
	xOffset += 10 + max(treeDelGoBtn.scale.x, treeDelBtn.scale.x);

	PauseChunkLoading.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y - 15));
	xOffset += 20 + PauseChunkLoading.scale.x;

	rendererWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y * 3 - 25));
	postprocessWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y * 2 - 20));
	visualsWnd_Toggle.SetPos(XMFLOAT2(xOffset, screenH - saveSchBtn.scale.y - 15));
	xOffset += 10 + max(postprocessWnd_Toggle.scale.x, visualsWnd_Toggle.scale.x);

	//saveButton.SetPos(XMFLOAT2(xOffset, screenH - postprocessWnd_Toggle.scale.y - 15));

	viewDist.SetPos(XMFLOAT2(screenW - viewDist.scale.x - 45, screenH - saveSchBtn.scale.y - 20));
}

void CyRender::Load() {
	wiColor uiColor_idle(100, 100, 100, 30);
	wiColor uiColor_focus(100, 100, 100, 0);
	wiColor uiColor_active(100, 130, 130, 0);

	renderPath = std::make_unique<RenderPath3D>();
	hovered	   = wiScene::PickResult();
	setMotionBlurEnabled(false);
	setMotionBlurStrength(10.0f);
	setBloomEnabled(true);
	wiRenderer::SetScreenSpaceShadowsEnabled(false);
	if (settings::volClouds)
		setBloomThreshold(1.9f);
	else
		setBloomThreshold(1.5f);
	setReflectionsEnabled(false);
	setSSREnabled(false);
	wiRenderer::SetGamma(2.4f);
	wiPhysicsEngine::SetEnabled(true);
	setLightShaftsEnabled(true);
	setColorGradingEnabled(true);
	setShadowsEnabled(true);
	setMSAASampleCount(1);
	setDitherEnabled(false);
	setSharpenFilterAmount(0.25f);
	wiRenderer::SetTransparentShadowsEnabled(false);
	//wiRenderer::SetShadowProps2D(2048, 4);

	setVolumetricCloudsEnabled(settings::volClouds);
	if (settings::tempAA) {
		wiRenderer::SetTemporalAAEnabled(true);
		resolutionScale = 1.f;
		setFXAAEnabled(false);
		setSharpenFilterEnabled(true);
	} else {
		wiRenderer::SetTemporalAAEnabled(false);
		resolutionScale = 1.3f;
		setSharpenFilterEnabled(false);
		setFXAAEnabled(true);
	}
	setExposure(1.f);

	wiRenderer::GetDevice()->SetVSyncEnabled(true);
	wiRenderer::SetVoxelRadianceEnabled(false);
	wiRenderer::SetVoxelRadianceNumCones(2);
	wiRenderer::SetVoxelRadianceRayStepSize(1.0f);
	wiRenderer::SetVoxelRadianceMaxDistance(300);
	wiRenderer::SetVoxelRadianceVoxelSize(0.125f);
	wiRenderer::SetVoxelRadianceSecondaryBounceEnabled(true);
	wiRenderer::SetOcclusionCullingEnabled(false);
	wiProfiler::SetEnabled(false);
	setAO(AO_MSAO);
	setAOPower(0.2);
	setAORange(50);
	setEyeAdaptionEnabled(false);
	wiScene::GetCamera().zNearP = 0.15f;
	wiScene::GetCamera().zFarP	= 2000.f;
	label.Create("Label1");
	label.SetText("CyubE3dit Wicked - Beta");
	label.SetColor(uiColor_idle, wiWidget::WIDGETSTATE::IDLE);
	label.SetColor(uiColor_focus, wiWidget::WIDGETSTATE::FOCUS);
	label.SetColor(uiColor_active, wiWidget::WIDGETSTATE::ACTIVE);
	label.font.params.h_align = WIFALIGN_CENTER;
	label.SetSize(XMFLOAT2(240, 20));
	GetGUI().AddWidget(&label);
	cyWorlds::getWorlds();
	worldSelector.Create("WorldSelector");
	worldSelector.SetText("Current World: ");
	worldSelector.SetSize(XMFLOAT2(250.f, 20.f));
	worldSelector.SetPos(XMFLOAT2(300.f, 20.f));
	worldSelector.SetColor(uiColor_idle, wiWidget::WIDGETSTATE::IDLE);
	worldSelector.SetColor(uiColor_focus, wiWidget::WIDGETSTATE::FOCUS);
	worldSelector.SetColor(uiColor_active, wiWidget::WIDGETSTATE::ACTIVE);
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

	camModeSelector.Create("CamMode");
	camModeSelector.SetText("WASD Camera mode: ");
	camModeSelector.SetTooltip("Camera can always be moved up/down with Q/E");
	camModeSelector.SetSize(XMFLOAT2(200.f, 20.f));
	camModeSelector.SetPos(XMFLOAT2(300.f, 20.f));
	camModeSelector.SetColor(uiColor_idle, wiWidget::WIDGETSTATE::IDLE);
	camModeSelector.SetColor(uiColor_focus, wiWidget::WIDGETSTATE::FOCUS);
	camModeSelector.SetColor(uiColor_active, wiWidget::WIDGETSTATE::ACTIVE);
	camModeSelector.AddItem("Move on all axis");
	camModeSelector.AddItem("Hor. plane only");
	camModeSelector.SetSelected(settings::camMode);
	camModeSelector.SetMaxVisibleItemCount(2);
	camModeSelector.OnSelect([=](wiEventArgs args) {
		settings::camMode = args.iValue;
	});
	GetGUI().AddWidget(&camModeSelector);

	rendererWnd = RendererWindow();
	rendererWnd.Create(this);
	GetGUI().AddWidget(&rendererWnd);

	postprocessWnd = PostprocessWindow();
	postprocessWnd.Create(this);
	GetGUI().AddWidget(&postprocessWnd);

	visualsWnd = VisualsWindow();
	visualsWnd.Create(this);
	GetGUI().AddWidget(&visualsWnd);

	viewDist.Create(settings::VIEWDIST_MIN, settings::VIEWDIST_MAX, settings::getViewDist(), settings::VIEWDIST_MAX / settings::VIEWDIST_MIN - 1, "View distance: ");
	viewDist.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	viewDist.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	viewDist.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	viewDist.SetValue(settings::viewDist);
	viewDist.OnSlide([=](wiEventArgs args) { settings::setViewDist((uint32_t)args.fValue); });
	viewDist.SetSize(XMFLOAT2(300, 20));
	GetGUI().AddWidget(&viewDist);
#ifdef PROSETTINGS
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

	saveButton.Create("Save");
	saveButton.SetTooltip("Save the current scene");
	saveButton.SetColor(wiColor(0, 198, 101, 180), wiWidget::WIDGETSTATE::IDLE);
	saveButton.SetColor(wiColor(0, 255, 140, 255), wiWidget::WIDGETSTATE::FOCUS);
	saveButton.OnClick([&](wiEventArgs args) {
		wiHelper::FileDialogParams params;
		params.type		   = wiHelper::FileDialogParams::SAVE;
		params.description = "Wicked Scene";
		params.extensions.push_back("wiscene");
		wiHelper::FileDialog(params, [this](std::string fileName) {
			wiEvent::Subscribe_Once(SYSTEM_EVENT_THREAD_SAFE_POINT, [=](uint64_t userdata) {
				std::string filename = fileName;
				if (filename.substr(filename.length() - 8).compare(".wiscene") != 0)
				{
					filename += ".wiscene";
				}
				wiArchive archive(filename, false);
				if (archive.IsOpen())
				{
					Scene& scene = wiScene::GetScene();

					wiResourceManager::SetMode(wiResourceManager::MODE_ALLOW_RETAIN_FILEDATA);

					scene.Serialize(archive);

				} else
				{
					wiHelper::messageBox("Could not create " + fileName + "!");
				}
			});
		});
	});
	GetGUI().AddWidget(&saveButton);

#endif
	PauseChunkLoading.Create("Pause World loading");
	PauseChunkLoading.SetTooltip("This will stop loading further portions of the world to save performance");
	PauseChunkLoading.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	PauseChunkLoading.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	PauseChunkLoading.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	PauseChunkLoading.SetSize(XMFLOAT2(150, 20));
	PauseChunkLoading.OnClick([&](wiEventArgs args) {
		settings::pauseChunkloader = !settings::pauseChunkloader;
		if (settings::pauseChunkloader)
			PauseChunkLoading.SetText("Resume World loading");
		else
			PauseChunkLoading.SetText("Pause World loading");
	});
	GetGUI().AddWidget(&PauseChunkLoading);

	visualsWnd_Toggle.Create("Settings");
	visualsWnd_Toggle.SetTooltip("Visual/Audio settings");
	visualsWnd_Toggle.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	visualsWnd_Toggle.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	visualsWnd_Toggle.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	visualsWnd_Toggle.SetSize(XMFLOAT2(120, 20));
	visualsWnd_Toggle.OnClick([&](wiEventArgs args) {
		visualsWnd.SetVisible(!visualsWnd.IsVisible());
		if (!m_soundLoaded) {
			visualsWnd.soundCheckBox.SetVisible(false);
			visualsWnd.musicVol.SetVisible(false);
			visualsWnd.effectVol.SetVisible(false);
		}
	});
	GetGUI().AddWidget(&visualsWnd_Toggle);

	loadSchBtn.Create("Insert schematic");
	loadSchBtn.SetTooltip("Load a schematic from disc to place it in the world");
	loadSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	loadSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	loadSchBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	loadSchBtn.SetSize(XMFLOAT2(150, 20));
	loadSchBtn.OnClick([&](wiEventArgs args) {
		loadSchBtn.SetEnabled(false);
		wiHelper::FileDialogParams params;
		params.description = "CyubeVR schematic";
		params.extensions.push_back("cySch");
		params.OPEN;
		wiHelper::FileDialog(params, cySchematic::addSchematic);
		loadSchBtn.SetEnabled(true);
	});
	GetGUI().AddWidget(&loadSchBtn);

	saveSchBtn.Create("Save schematic");
	saveSchBtn.SetTooltip("Select a portion of your world to save as schematic.");
	saveSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	saveSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	saveSchBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	saveSchBtn.SetSize(XMFLOAT2(150, 20));
	saveSchBtn.OnClick([&](wiEventArgs args) {
		cySchematic::addBoxSelector();
	});
	GetGUI().AddWidget(&saveSchBtn);

	reposSchBtn.Create("Bring Schematic in view.");
	reposSchBtn.SetTooltip("This will move the current schematic to your viewport in case you lost it somewhere.");
	reposSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	reposSchBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	reposSchBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	reposSchBtn.SetSize(XMFLOAT2(150, 20));
	reposSchBtn.SetVisible(false);
	reposSchBtn.OnClick([&](wiEventArgs args) {
		cySchematic::reposition();
	});
	GetGUI().AddWidget(&reposSchBtn);

	if (settings::treeDeletion) {
		treeDelBtn.Create("Disable Tree removal.");
	}

	else {
		treeDelBtn.Create("Enable Tree removal.");
	}

	treeDelBtn.SetTooltip("This will enable you to select trees by clicking and deleting them.");
	treeDelBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	treeDelBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	treeDelBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	treeDelBtn.SetSize(XMFLOAT2(160, 20));
	treeDelBtn.SetVisible(true);
	treeDelBtn.OnClick([&](wiEventArgs args) {
		settings::treeDeletion = !settings::treeDeletion;
		m_selectedObjects.clear();
		if (settings::treeDeletion) {
			treeDelBtn.SetText("Disable Tree removal.");
			treeDelRstBtn.SetVisible(true);
			treeDelGoBtn.SetVisible(true);
		} else {
			treeDelRstBtn.SetVisible(false);
			treeDelGoBtn.SetVisible(false);
			treeDelBtn.SetText("Enable Tree removal.");
		}
	});
	GetGUI().AddWidget(&treeDelBtn);

	treeDelRstBtn.Create("Reset Selection.");
	treeDelRstBtn.SetTooltip("This unselects all trees.");
	treeDelRstBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	treeDelRstBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	treeDelRstBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	treeDelRstBtn.SetSize(XMFLOAT2(160, 20));
	if (!settings::treeDeletion)
		treeDelRstBtn.SetVisible(false);
	treeDelRstBtn.OnClick([&](wiEventArgs args) {
		for (size_t i = 0; i < m_selectedObjects.size(); i++) {
			ObjectComponent* object = wiScene::GetScene().objects.GetComponent(m_selectedObjects[i]);
			if (object != nullptr) {
				object->emissiveColor = XMFLOAT4(0, 0, 0, 1);
			}
		}
		m_selectedObjects.clear();
	});
	GetGUI().AddWidget(&treeDelRstBtn);

	treeDelGoBtn.Create("Del. selected trees.");
	treeDelGoBtn.SetTooltip("This deletes all selected trees.");
	treeDelGoBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::IDLE);
	treeDelGoBtn.SetColor(wiColor(100, 100, 100, 150), wiWidget::WIDGETSTATE::FOCUS);
	treeDelGoBtn.SetColor(wiColor(100, 100, 100, 200), wiWidget::WIDGETSTATE::ACTIVE);
	treeDelGoBtn.SetSize(XMFLOAT2(160, 20));
	if (!settings::treeDeletion)
		treeDelGoBtn.SetVisible(false);
	treeDelGoBtn.OnClick([&](wiEventArgs args) {
		if (m_selectedObjects.size()) {
			std::unordered_set<cyImportant::chunkpos_t, cyImportant::chunkposHasher_t> update;
			for (size_t i = 0; i < m_selectedObjects.size(); i++) {
				ObjectComponent* object = wiScene::GetScene().objects.GetComponent(m_selectedObjects[i]);
				TransformComponent* tf	= wiScene::GetScene().transforms.GetComponent(m_selectedObjects[i]);
				if (object != nullptr) {
					object->emissiveColor = XMFLOAT4(0, 0, 0, 1);
					cyImportant::chunkpos_t zero, chunkPos;
					uint32_t chunkID;
					cyImportant* world = settings::getWorld();
					zero.x			   = world->m_playerpos.x / 100;
					zero.y			   = world->m_playerpos.y / 100;
					chunkPos		   = world->getChunkPos(tf->translation_local.x + 0.1, tf->translation_local.z - 0.1);

					if (world->getChunkID(chunkPos.x + zero.x, chunkPos.y + zero.y, &chunkID)) {
						cyChunk chunk;
						chunk.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID);
						cyChunk::blockpos_t pos;
						pos.x	= tf->translation_local.x * 2.f - chunkPos.x * 2.f;
						int txt = tf->translation_local.z;
						pos.y	= 32 - (tf->translation_local.z * 2.f + chunkPos.y * 2.f);
						pos.z	= (tf->translation_local.y + 0.25f) * 2.f;
						chunk.deleteTree(pos);
						update.insert(chunkPos);
					}
				}
			}
			m_selectedObjects.clear();
			for (auto it = update.begin(); it != update.end(); ++it) {
				chunkLoader::reloadChunk(*it);
			}
			update.clear();
		}
	});
	GetGUI().AddWidget(&treeDelGoBtn);

	RenderPath3D::Load();
	//ResizeBuffers();
}

void CyRender::Update(float dt) {
	static cySchematic::hovertype_t drag	= cySchematic::HOVER_NONE;
	static size_t dragID					= 0;
	static cySchematic::hovertype_t clicked = cySchematic::HOVER_NONE;
	dragID									= 0;
	static XMVECTOR deltaV;
	static wiECS::Entity lastHovered = wiECS::INVALID_ENTITY;
	CameraComponent& camera			 = wiScene::GetCamera();
	// Camera control:
	static XMFLOAT4 originalMouse = XMFLOAT4(0, 0, 0, 0);
	static float Accel			  = 0.0;
	static bool camControlStart	  = true;
	static bool schDraged		  = false;
	static float rotateAni		  = 0.f;

	lasttime += dt * 4;
	sinepulse = std::sinf(lasttime);
	if (lasttime > 100) {
		lasttime -= 100;
	}
	if (camControlStart)
	{
		originalMouse = wiInput::GetPointer();
		deltaV		  = XMVectorZero();
	}

	XMFLOAT4 currentMouse = wiInput::GetPointer();
	float xDif = 0, yDif = 0;

	if (wiInput::Down(wiInput::MOUSE_BUTTON_LEFT) && !GetGUI().GetActiveWidget()) {
		if (drag != cySchematic::HOVER_NONE) {
			schDraged = true;
			switch (drag) {
				case cySchematic::HOVER_CHECK:
					clicked = drag;
					break;
				case cySchematic::HOVER_CROSS:
					clicked = drag;
					break;
				case cySchematic::HOVER_ROTCC:
					clicked = drag;
					break;
				case cySchematic::HOVER_ROTCW:
					clicked = drag;
					break;
				default:
					camControlStart	 = false;
					bool dragX		 = false;
					bool dragY		 = false;
					bool dragZ		 = false;
					XMFLOAT4 pointer = wiInput::GetPointer();
					XMVECTOR plane, planeNormal;
					TransformComponent* transform = wiScene::GetScene().transforms.GetComponent(cySchematic::m_schematics[dragID]->hoverEntities[drag].entity);
					if (transform != nullptr) {
						XMVECTOR pos = transform->GetPositionV();
						XMVECTOR B, axis, wrong;
						switch (drag) {
							case cySchematic::HOVER_X_AXIS:
							case cySchematic::HOVER_SIZEX:
								axis		= XMVectorSet(1, 0, 0, 0);
								B			= pos + XMVectorSet(1, 0, 0, 0);
								wrong		= XMVector3Cross(wiScene::GetCamera().GetAt(), axis);
								planeNormal = XMVector3Cross(wrong, axis);
								dragX		= true;
								break;
							case cySchematic::HOVER_Y_AXIS:
							case cySchematic::HOVER_SIZEY:
								axis		= XMVectorSet(0, 0, 1, 0);
								B			= pos + XMVectorSet(0, 0, 1, 0);
								wrong		= XMVector3Cross(wiScene::GetCamera().GetAt(), axis);
								planeNormal = XMVector3Cross(wrong, axis);
								dragY		= true;
								break;
							case cySchematic::HOVER_Z_AXIS:
							case cySchematic::HOVER_SIZEZ:
								axis		= XMVectorSet(0, 1, 0, 0);
								B			= pos + XMVectorSet(0, 1, 0, 0);
								wrong		= XMVector3Cross(wiScene::GetCamera().GetAt(), axis);
								planeNormal = XMVector3Cross(wrong, axis);
								dragZ		= true;
								break;
							case cySchematic::HOVER_ORIGIN:
								planeNormal = wiScene::GetCamera().GetAt();
								dragX		= true;
								dragY		= true;
								dragZ		= true;
								break;
							case cySchematic::HOVER_XY_PLANE:
							case cySchematic::HOVER_XY2_PLANE:
								planeNormal = XMVectorSet(0, 1, 0, 0);
								dragX		= true;
								dragY		= true;
								break;
							case cySchematic::HOVER_XZ_PLANE:
							case cySchematic::HOVER_XZ2_PLANE:
								planeNormal = XMVectorSet(0, 0, 1, 0);
								dragX		= true;
								dragZ		= true;
								break;
							case cySchematic::HOVER_YZ_PLANE:
							case cySchematic::HOVER_YZ2_PLANE:
								planeNormal = XMVectorSet(1, 0, 0, 0);
								dragY		= true;
								dragZ		= true;
								break;
							default:
								break;
						}
						plane = XMPlaneFromPointNormal(pos, XMVector3Normalize(planeNormal));

						RAY ray				  = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
						XMVECTOR rayOrigin	  = XMLoadFloat3(&ray.origin);
						XMVECTOR rayDir		  = XMLoadFloat3(&ray.direction);
						XMVECTOR intersection = XMPlaneIntersectLine(plane, rayOrigin, rayOrigin + rayDir * wiScene::GetCamera().zFarP);

						ray						  = wiRenderer::GetPickRay((long)originalMouse.x, (long)originalMouse.y);
						rayOrigin				  = XMLoadFloat3(&ray.origin);
						rayDir					  = XMLoadFloat3(&ray.direction);
						XMVECTOR intersectionPrev = XMPlaneIntersectLine(plane, rayOrigin, rayOrigin + rayDir * wiScene::GetCamera().zFarP);

						switch (drag) {
							case cySchematic::HOVER_X_AXIS:
							case cySchematic::HOVER_Y_AXIS:
							case cySchematic::HOVER_Z_AXIS:
							case cySchematic::HOVER_SIZEX:
							case cySchematic::HOVER_SIZEY:
							case cySchematic::HOVER_SIZEZ:
								XMVECTOR P	   = wiMath::GetClosestPointToLine(pos, B, intersection);
								XMVECTOR PPrev = wiMath::GetClosestPointToLine(pos, B, intersectionPrev);
								deltaV += P - PPrev;
								break;
							default:
								deltaV += intersection - intersectionPrev;
						}
						XMFLOAT3 delta(0.f, 0.f, 0.f);

						cySchematic::m_schematics[dragID]->drawGridLines(dragX, dragY, dragZ);

						if (drag < cySchematic::HOVER_SIZEX) {
							if (wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT)) {
								delta.x = roundf(XMVectorGetX(deltaV) / cySchematic::m_schematics[dragID]->size.x) * cySchematic::m_schematics[dragID]->size.x;
								delta.y = roundf(XMVectorGetY(deltaV) / cySchematic::m_schematics[dragID]->size.z) * cySchematic::m_schematics[dragID]->size.z;
								delta.z = roundf(XMVectorGetZ(deltaV) / cySchematic::m_schematics[dragID]->size.y) * cySchematic::m_schematics[dragID]->size.y;
							} else {
								delta.x = roundf(XMVectorGetX(deltaV) * 2.f) / 2.f;
								delta.y = roundf(XMVectorGetY(deltaV) * 2.f) / 2.f;
								delta.z = roundf(XMVectorGetZ(deltaV) * 2.f) / 2.f;
							}

							transform = wiScene::GetScene().transforms.GetComponent(cySchematic::m_schematics[dragID]->mainEntity);
							XMFLOAT3 newpos(transform->GetPosition().x + delta.x, transform->GetPosition().y + delta.y, transform->GetPosition().z + delta.z);
							if (wiMath::DistanceEstimated(camera.Eye, newpos) <= 30.f + 20.f * cySchematic::m_schematics[dragID]->size.w && newpos.y > 0.f && newpos.y + cySchematic::m_schematics[dragID]->size.z < 400.f) {  //limit the area of movement near camera to keep the schematic visible (size.w has to holt the average of all three sizes
								deltaV = XMVectorSubtract(deltaV, XMLoadFloat3(&delta));
								transform->Translate(delta);
								transform->UpdateTransform();
								cySchematic::m_schematics[dragID]->pos.x += delta.x;
								cySchematic::m_schematics[dragID]->pos.z += delta.y;
								cySchematic::m_schematics[dragID]->pos.y += delta.z;
							}
						} else {

							delta.x = roundf(XMVectorGetX(deltaV) * 4.f) / 2.f;
							delta.y = roundf(XMVectorGetY(deltaV) * 4.f) / 2.f;
							delta.z = roundf(XMVectorGetZ(deltaV) * 4.f) / 2.f;

							transform = wiScene::GetScene().transforms.GetComponent(cySchematic::m_schematics[dragID]->mainEntity);
							XMFLOAT3 newsize(cySchematic::m_schematics[dragID]->size.x + delta.x, cySchematic::m_schematics[dragID]->size.z + delta.y, cySchematic::m_schematics[dragID]->size.y + delta.z);
							if (newsize.x > 0.4 && newsize.x < 351.f && newsize.z > 0.4 && newsize.z < 351.f && newsize.y > 0.4 && newsize.y < 350.f && newsize.y + cySchematic::m_schematics[dragID]->pos.z < 400.f) {
								cySchematic::m_schematics[dragID]->size.x += delta.x;
								cySchematic::m_schematics[dragID]->size.z += delta.y;
								cySchematic::m_schematics[dragID]->size.y += delta.z;
								cySchematic::m_schematics[dragID]->size.x  = std::clamp(cySchematic::m_schematics[dragID]->size.x, 0.5f, 350.f);
								cySchematic::m_schematics[dragID]->size.y  = std::clamp(cySchematic::m_schematics[dragID]->size.y, 0.5f, 350.f);
								cySchematic::m_schematics[dragID]->size.z  = std::clamp(cySchematic::m_schematics[dragID]->size.z, 0.5f, 350.f);
								cySchematic::m_schematics[dragID]->size.w  = (cySchematic::m_schematics[dragID]->size.x + cySchematic::m_schematics[dragID]->size.y + cySchematic::m_schematics[dragID]->size.z) * 0.334;
								cySchematic::m_schematics[dragID]->m_dirty = cySchematic::DIRTY_RESIZE;
								delta.x *= 0.5;
								delta.y *= 0.5;
								delta.z *= 0.5;
								deltaV = XMVectorSubtract(deltaV, XMLoadFloat3(&delta));
							}
						}
						originalMouse = pointer;
					} else {  //hovered element was deleted -> clear drag request
						drag = cySchematic::HOVER_NONE;
					}
					break;
			}
		} else {
			if (settings::treeDeletion) {
				RAY pickRay			  = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
				wiECS::Entity treeSel = wiScene::Pick(pickRay, RENDERTYPE_ALL, LAYER_TREE).entity;
				ObjectComponent* obj  = wiScene::GetScene().objects.GetComponent(treeSel);
				if (obj != nullptr) {
					obj->emissiveColor = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
					m_selectedObjects.push_back(treeSel);
				}
			}
			hovered.entity = wiECS::INVALID_ENTITY;
		}

	} else if (wiInput::Down(wiInput::MOUSE_BUTTON_RIGHT))
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
		if (abs(xDif) < 0.00001)
			xDif = 0;
		if (abs(yDif) < 0.00001)
			yDif = 0;
		wiInput::SetPointer(originalMouse);
		wiInput::HidePointer(true);
	} else {
		if (cySchematic::m_schematics.size() > 0) {
			if (saveSchBtn.IsVisible()) {
				loadSchBtn.SetVisible(false);
				saveSchBtn.SetVisible(false);
				reposSchBtn.SetVisible(true);
			}
			for (size_t i = 0; i < cySchematic::m_schematics.size(); i++) {
				wiScene::Scene& scene	   = wiScene::GetScene();
				RAY pickRay				   = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
				wiScene::PickResult SchHov = wiScene::Pick(pickRay, RENDERTYPE_TRANSPARENT | RENDERTYPE_OPAQUE, LAYER_GIZMO);
				drag					   = cySchematic::m_schematics[i]->hoverGizmo(SchHov.entity);
				dragID					   = i;
				XMMATRIX sca			   = XMMatrixScaling(cySchematic::m_schematics[i]->size.x / 2, cySchematic::m_schematics[i]->size.z / 2, cySchematic::m_schematics[i]->size.y / 2);
				XMMATRIX tra			   = XMMatrixTranslation(cySchematic::m_schematics[i]->pos.x + cySchematic::m_schematics[i]->size.x / 2 - 0.25f, cySchematic::m_schematics[i]->pos.z + cySchematic::m_schematics[i]->size.z / 2 - 0.25f, cySchematic::m_schematics[i]->pos.y + cySchematic::m_schematics[i]->size.y / 2 + 0.25f);
				XMFLOAT4X4 hoverBox;
				XMStoreFloat4x4(&hoverBox, sca * tra);
				wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.5f, 1.0f, 1.f, 1.0f));
			}
		} else if (!saveSchBtn.IsVisible()) {
			loadSchBtn.SetVisible(true);
			saveSchBtn.SetVisible(true);
			reposSchBtn.SetVisible(false);
		}
		if (schDraged) {
			schDraged = false;
			switch (clicked) {
				case cySchematic::HOVER_CHECK:
					if (clicked == drag) {
						//save here!!
						cySchematic::m_schematics[dragID]->m_dirty = cySchematic::DIRTY_SAVE;
					}
					break;
				case cySchematic::HOVER_CROSS:
					if (clicked == drag) {
						cySchematic::m_schematics[dragID]->m_dirty = cySchematic::DIRTY_REMOVE;
						//cySchematic::m_schematics[dragID]->clearSchematic();
					}
					break;
				case cySchematic::HOVER_ROTCC:
					if (clicked == drag) {
						cySchematic::m_schematics[dragID]->m_dirty = cySchematic::DIRTY_ROTCC;
						//rotate here!!
					}
					break;
				case cySchematic::HOVER_ROTCW:
					if (clicked == drag) {
						cySchematic::m_schematics[dragID]->m_dirty = cySchematic::DIRTY_ROTCW;
						//rotate here!!
					}
					break;
				default:
					cySchematic::m_schematics[dragID]->m_dirty = cySchematic::DIRTY_DRAG;
					break;
			}
			clicked = cySchematic::HOVER_NONE;
			drag	= cySchematic::HOVER_NONE;	//clear hover state as it won't be updated if no schematic is present
		}

		camControlStart = true;

		wiInput::HidePointer(false);
		if (settings::pickType) {
			RAY pickRay		  = wiRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
			uint32_t pickmask = 0;
			if (settings::pickType & PICK_CHUNK)
				pickmask |= LAYER_CHUNKMESH;
			if (settings::pickType & PICK_TREE)
				pickmask |= LAYER_TREE;
			hovered = wiScene::Pick(pickRay, 1, pickmask);

		} else {
			hovered.entity = wiECS::INVALID_ENTITY;
		}
		if (hovered.entity != wiECS::INVALID_ENTITY)
		{
			ObjectComponent* object = wiScene::GetScene().objects.GetComponent(hovered.entity);
			const AABB& aabb		= *wiScene::GetScene().aabb_objects.GetComponent(hovered.entity);
			object->color			= XMFLOAT4((3.0 - sinepulse) / 4, (3.0 - sinepulse) / 4, (3.0 - sinepulse) / 4, 1);
			object->emissiveColor	= XMFLOAT4((2.0 + sinepulse) * 0.3, (2.0 + sinepulse) * 0.3, (2.0 + sinepulse) * 0.3, 1);
			XMFLOAT4X4 hoverBox;
			XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
			wiRenderer::DrawBox(hoverBox, XMFLOAT4(0.5f, 1.0f, 1.f, 1.0f));
		}
		if (lastHovered != hovered.entity) {
			ObjectComponent* object = wiScene::GetScene().objects.GetComponent(lastHovered);
			if (object != nullptr) {
				object->color		  = XMFLOAT4(1, 1, 1, 1);
				object->emissiveColor = XMFLOAT4(0, 0, 0, 1);
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
	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_ESCAPE)) {
		if (m_selectedObjects.size()) {
			for (size_t i = 0; i < m_selectedObjects.size(); i++) {
				ObjectComponent* object = wiScene::GetScene().objects.GetComponent(m_selectedObjects[i]);
				if (object != nullptr) {
					object->emissiveColor = XMFLOAT4(0, 0, 0, 1);
				}
			}
			m_selectedObjects.clear();
		}
	}
	/*if (wiInput::Down(wiInput::KEYBOARD_BUTTON_DELETE)) {

		
	}*/
	//const XMFLOAT4 leftStick	= wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_L, 0);
	//const XMFLOAT4 rightStick	= wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_THUMBSTICK_R, 0);
	//const XMFLOAT4 rightTrigger = wiInput::GetAnalog(wiInput::GAMEPAD_ANALOG_TRIGGER_R, 0);

	//const float jostickrotspeed = 0.05f;
	//xDif += rightStick.x * jostickrotspeed;
	//yDif += rightStick.y * jostickrotspeed;

	// FPS Camera
	const float clampedDT = min(dt, 0.15f);	 // if dt > 100 millisec, don't allow the camera to jump too far...
	bool btn			  = false;
	XMVECTOR moveNew	  = XMVectorSet(0, 0, 0, 0);
	float moveNewY		  = 0.f;

	Accel += clampedDT * 1.1f;
	if (Accel > 3.5) {
		Accel = 3.5;
	}
	const float speed = ((wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0 : 1.0)) * Accel * settings::camspeed * clampedDT;

	if (wiInput::Down((wiInput::BUTTON)'A')) {
		moveNew += XMVectorSet(-1, 0, 0, 0);
		btn = true;
	} else if (wiInput::Down((wiInput::BUTTON)'D')) {
		moveNew += XMVectorSet(1, 0, 0, 0);
		btn = true;
	}
	if (wiInput::Down((wiInput::BUTTON)'W')) {
		moveNew += XMVectorSet(0, 0, 1, 0);
		btn = true;
	} else if (wiInput::Down((wiInput::BUTTON)'S')) {
		moveNew += XMVectorSet(0, 0, -1, 0);
		btn = true;
	}
	if (wiInput::Down((wiInput::BUTTON)'E')) {
		moveNewY = 1;
		btn		 = true;
	} else if (wiInput::Down((wiInput::BUTTON)'Q')) {
		moveNewY = -1;
		btn		 = true;
	}
	moveNew += XMVector3Normalize(moveNew);
	moveNew *= speed;
	moveNewY *= speed;
	float moveLength = XMVectorGetX(XMVector3Length(moveNew)) + abs(moveNewY);
	if (moveLength < 0.0001f)
	{
		moveLength = 0;
		moveNew	   = XMVectorSet(0, 0, 0, 0);
		moveNewY   = 0.f;
		Accel	   = 0;
	}
	if (!btn) {
		Accel = 0;
	}

	if (abs(xDif) + abs(yDif) > 0 || moveLength > 0.0001f)
	{

		wiScene::TransformComponent camera_transform;
		camera_transform.ClearTransform();
		camera_transform.rotation_local.w = 0;
		camera_transform.rotation_local.x = 0;
		camera_transform.rotation_local.y = 0;
		camera_transform.rotation_local.z = 0;

		TransformComponent* lightT = wiScene::GetScene().transforms.GetComponent(CyMainComponent::m_headLight);

		camera_transform.MatrixTransform(wiScene::GetCamera().GetInvView());

		camera_transform.UpdateTransform();
		XMMATRIX camRot	  = XMMatrixRotationQuaternion(XMLoadFloat4(&camera_transform.rotation_local));
		XMVECTOR move_rot = XMVector3TransformNormal(moveNew, camRot);

		if (settings::camMode && XMVectorGetX(XMVector3Length(moveNew)) > 0.0001) {
			moveLength = XMVectorGetX(XMVector3Length(move_rot));
			move_rot   = XMVectorSetY(move_rot, 0);
			move_rot   = (moveLength / XMVectorGetX(XMVector3Length(move_rot))) * move_rot;
		}
		XMFLOAT3 _move;
		XMStoreFloat3(&_move, move_rot);
		_move.y += moveNewY;
		camera_transform.Translate(_move);
		lightT->Translate(_move);
		lightT->RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
		camera_transform.RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
		if (camera.At.y > 0.95 && yDif < 0) {
			camera_transform.RotateRollPitchYaw(XMFLOAT3(-yDif, 0, 0));
			lightT->RotateRollPitchYaw(XMFLOAT3(-yDif, 0, 0));
		} else if (camera.At.y < -0.95 && yDif > 0) {
			camera_transform.RotateRollPitchYaw(XMFLOAT3(-yDif, 0, 0));
			lightT->RotateRollPitchYaw(XMFLOAT3(-yDif, 0, 0));
		}
		camera_transform.UpdateTransform();
		lightT->UpdateTransform();
		camera.TransformCamera(camera_transform);
		camera.UpdateCamera();
		if (CyMainComponent::m_dust != INVALID_ENTITY) {
			TransformComponent* dust = wiScene::GetScene().transforms.GetComponent(CyMainComponent::m_dust);
			dust->Translate(_move);
			dust->UpdateTransform();
		}
		/*if (CyMainComponent::m_probe != INVALID_ENTITY) {
			TransformComponent* probeT = wiScene::GetScene().transforms.GetComponent(CyMainComponent::m_probe);
			probeT->Translate(_move);
			probeT->SetDirty();
			probeT->UpdateTransform();
		}*/
	} else {
		Accel = 0;
	}
	RenderPath3D::Update(dt);
}
