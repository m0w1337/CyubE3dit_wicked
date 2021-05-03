#pragma once
#include "cyChunk.h"
#include "ChunkLoader.h"

class cySchematic {
public:
	typedef enum hovertype_e{
		HOVER_X_AXIS,
		HOVER_Y_AXIS,
		HOVER_Z_AXIS,
		HOVER_XY_PLANE,
		HOVER_XY2_PLANE,
		HOVER_XZ_PLANE,
		HOVER_XZ2_PLANE,
		HOVER_YZ_PLANE,
		HOVER_YZ2_PLANE,
		HOVER_ORIGIN,
		HOVER_ROTCW,
		HOVER_ROTCC,
		HOVER_CHECK,
		HOVER_CROSS,
		HOVER_SIZEX,
		HOVER_SIZEY,
		HOVER_SIZEZ,
		HOVER_NUMELEMENTS,
		HOVER_NONE
	} hovertype_t;
	typedef enum dirtytype_e {
		NOT_DIRTY = 0,
		DIRTY_NOTRENDERED,
		DIRTY_ROTCC,
		DIRTY_ROTCW,
		DIRTY_DRAG,
		DIRTY_SAVE,
		DIRTY_REMOVE,
		DIRTY_RESIZE
	}dirtytype_t;

	typedef enum schType_e {
		TYPE_SELECTION = 0,
		TYPE_SCHEM_V1,
		NUM_TYPES
	} schType_t;

	
	static const uint32_t HEADER_SIZE = 20;
	static const uint32_t MAGICBYTE = 0x13371337;
	struct blockpos_t {
		uint32_t x;
		uint32_t y;
		uint32_t z;
		blockpos_t() :
		  x(0),
		  y(0),
		  z(0){};
		blockpos_t(const uint32_t& _x, const uint32_t& _y, const uint32_t& _z) :
		  x(_x),
		  y(_y),
		  z(_z){};
		blockpos_t(const blockpos_t& other) {
			x = other.x;
			y = other.y;
			z = other.z;
		};

		blockpos_t& operator=(const blockpos_t& other) {
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		};

		bool operator==(const blockpos_t& other) const {
			if (x == other.x && y == other.y && z == other.z)
				return true;
			return false;
		};

		blockpos_t& operator+(const blockpos_t& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		};
		blockpos_t& operator-(const blockpos_t& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		};

		bool operator<(const blockpos_t& other) {
			if (x < other.x)
				return true;
			return false;
		};
		size_t operator()(const blockpos_t& pointToHash) const noexcept {
			return ((((uint64_t)pointToHash.x) << 24) | (((uint64_t)pointToHash.y) << 16) | ((uint64_t)pointToHash.z));
		};
	};
	struct torch_t {
		torch_t() :
		  ID(0),
		  x(0),
		  y(0),
		  z(0),
		  rotation(0){};
		torch_t(const uint8_t& _ID, const uint16_t& _x, const uint16_t& _y, const uint16_t& _z, uint8_t _rotation) :
		  ID(_ID),
		  x(_x),
		  y(_y),
		  z(_z),
		  rotation(_rotation){};
		uint8_t ID;
		uint16_t x;
		uint16_t y;
		uint16_t z;
		uint8_t rotation;
	};
	struct hoverAttr_s {
		wiECS::Entity entity;
		XMFLOAT4 hovercolor;
		XMFLOAT4 nohovercolor;
	};
	typedef enum chunkstate_e {
		STATE_SAVEABLE,
		STATE_OLD,
		STATE_BROKEN,
		STATE_NOTPRESENT,
		NUM_STATES
	}chunkstate_t;
	XMFLOAT4 size;
	XMFLOAT3 pos;
	schType_t type;
	static void addBoxSelector(void);
	static std::vector<cySchematic*> m_schematics;
	std::vector<chunkLoader::chunkobjects_t> m_chunkPreviews;
	unordered_map<blockpos_t, uint32_t, blockpos_t> m_cBlocks;
	unordered_map<blockpos_t, uint8_t, blockpos_t> m_torches;
	std::vector<cyChunk::meshLoc> meshObjects;
	std::vector<cyChunk::treeLoc> trees;
	wiECS::Entity mainEntity;
	struct hoverAttr_s hoverEntities[HOVER_NUMELEMENTS];
	uint8_t m_activeGizmo;
	bool m_isAirChunk;
	uint8_t* m_chunkdata;
	dirtytype_t m_dirty;
	static bool updating;
	cySchematic(string filename);
	static void addSchematic(std::string filename);
	hovertype_t hoverGizmo(const wiECS::Entity entity);
	void RenderSchematic(void);
	chunkstate_t checkAffectedChunks();
	void generateChunkPreview(void);
	void clearSchematic(void);
	static void clearAllSchematics(void);
	static void updateDirtyPreviews(void);
	void saveToWorld(void);
	void rotate(bool _cclock = false);
	void drawGridLines(const bool x, const bool y, const bool z);

private:
	uint8_t m_rotation;	//1,2,3 for 90,180,270 deg
	void loadCustomBlocks(void);
	void unRenderSchematic(void);
	void rotateMemory(const bool _cc);
	inline void showGizmos(bool _show);
	inline void placeTorches(const std::vector<torch_t>& torches, wiScene::Scene& tmpScene);
	void prepareSchematic(wiScene::Scene& tmpScene);
	void attachGizmos(wiScene::Scene& tmpScene, const bool _toolblock = false);
	void resizeGizmos(void); 
	void positionBeforeCam(void);
	void saveToMem(void);
	void saveToFile(void);
};
