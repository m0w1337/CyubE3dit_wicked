#pragma once
#include "RenderPath3D_PathTracing.h"
#include "RendererWindow.h"

#include "PostprocessWindow.h"
#include "VisualsWindow.h"
#include "meshGen.h"
#include "cyBlocks.h"

static constexpr float PI = 3.141592653589793238462643383279502884f;

enum LAYERMASKS {
	LAYER_CHUNKMESH = 1,
	LAYER_TREE		= 2,
	LAYER_TORCH		= 4,
	LAYER_SCHEMATIC = 8,
	LAYER_MESH		= 16,
	LAYER_FOILAGE	= 32,
	LAYER_EMITTER	= 64,
	LAYER_GIZMO		= 128
};

enum PICKTYPE {
	PICK_VOID		= 0,
	PICK_CHUNK		= 1,
	PICK_LIGHT		= 8,
	PICK_TREE		= 16,
	PICK_ENVPROBE	= 32,
	PICK_FORCEFIELD = 64,
	PICK_EMITTER	= 128,
	PICK_HAIR		= 256,
	PICK_CAMERA		= 512,
	PICK_ARMATURE	= 1024,
	PICK_SOUND		= 2048,
};

class CyLoadingScreen : public LoadingScreen {
private:
	wiSprite sprite;
	wiSpriteFont font;

public:
	void Load() override;
	void Update(float dt) override;
};
class CyMainComponent;
class CyRender : public RenderPath3D {
private:
	std::unique_ptr<RenderPath3D> renderPath;
	/*
	wiGraphics::RenderPass renderpass_selectionOutline[2];
	wiGraphics::RenderPass renderpass_composeOutline;
	wiGraphics::Texture rt_selectionOutline_MSAA;
	wiGraphics::Texture rt_selectionOutline[2];
	*/


public:
	/*enum EDITORSTENCILREF {
		EDITORSTENCILREF_CLEAR				= 0x00,
		EDITORSTENCILREF_HIGHLIGHT_OBJECT	= 0x01,
		EDITORSTENCILREF_HIGHLIGHT_MATERIAL = 0x02,
		EDITORSTENCILREF_LAST				= 0x0F,
	};
	float selectionOutlineTimer	   = 0;
	const XMFLOAT4 selectionColor  = XMFLOAT4(0.8f, 0.6f, 0, 1.f);
	const XMFLOAT4 selectionColor2 = XMFLOAT4(0, 0.6f, 0.8f, 1.f);
	*/
	float lasttime;
	float sinepulse;
	wiComboBox worldSelector;
	wiComboBox camModeSelector;
	RendererWindow rendererWnd;
	PostprocessWindow postprocessWnd;
	VisualsWindow visualsWnd;
	wiButton rendererWnd_Toggle;
	wiButton postprocessWnd_Toggle;
	wiButton visualsWnd_Toggle;
	wiButton loadSchBtn;
	wiButton saveSchBtn;
	wiButton reposSchBtn;
	wiButton treeDelBtn;
	wiButton treeDelRstBtn;
	wiButton treeDelGoBtn;
	wiButton saveButton;
	wiButton PauseChunkLoading;
	wiSlider viewDist;
	wiLabel label;

	static const uint8_t NUM_TORCHSOUNDS = 4;
	static wiAudio::Sound fireSound;
	static wiAudio::SoundInstance fireSoundinstance[NUM_TORCHSOUNDS];
	static wiAudio::Sound windSound;
	static wiAudio::SoundInstance windSoundinstance;
	static wiAudio::Sound bgSound;
	static wiAudio::SoundInstance bgSoundinstance;
	static std::vector<wiECS::Entity> m_selectedObjects;
	bool m_soundLoaded;
	static bool fireSoundIsPlaying[NUM_TORCHSOUNDS];
	static bool anyfireSoundIsPlaying;

	CyMainComponent* main;
	wiScene::PickResult hovered;
	void Load() override;
	void Update(float dt) override;
	//void ResizeBuffers() override;
	void ResizeLayout() override;
	//void Render() const override;
	//void Compose(wiGraphics::CommandList cmd) const override;
};
/*
class CyPathRender : public RenderPath3D_PathTracing
{
public:
	void Load() override;

	void Update(float dt) override;
	void ResizeLayout() override;
};
*/
class CyMainComponent : public MainComponent {

public:
	/*
	enum EDITORSTENCILREF {
		EDITORSTENCILREF_CLEAR				= 0x00,
		EDITORSTENCILREF_HIGHLIGHT_OBJECT	= 0x01,
		EDITORSTENCILREF_HIGHLIGHT_MATERIAL = 0x02,
		EDITORSTENCILREF_LAST				= 0x0F,
	};

	
	float selectionOutlineTimer	   = 0;
	const XMFLOAT4 selectionColor  = XMFLOAT4(0.4f, 0.6f, 0.1f, 0.5f);
	const XMFLOAT4 selectionColor2 = XMFLOAT4(0.f, 1.f, 0.6f, 0.35f);
	*/
	
	static wiECS::Entity m_headLight;
	static wiECS::Entity m_posLight;
	static wiECS::Entity m_probe;
	static wiECS::Entity m_dust;
	CyRender renderer;
	CyLoadingScreen loader;
	//CyPathRender pathRenderer;
	void Initialize() override;
	void CreateScene(void);
	void LoadSound(void);
	void Compose(wiGraphics::CommandList cmd) override;
};
