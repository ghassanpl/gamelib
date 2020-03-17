#pragma once
#define ALLEGRO_UNSTABLE 1
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "../lib/imgui-allegro/imgui-Allegro.h"
#include <imgui.h>

#include <Common.h>
#include <Includes/Allegro.h>
#include <Includes/EnumFlags.h>
#include <Includes/Format.h>
#include <Includes/GLM.h>
#include <Includes/JSON.h>
#include <Includes/MagicEnum.h>
#include <Align.h>
#include <Animations.h>
#include <Camera.h>
#include <Colors.h>
#include <Debugger.h>
#include <ErrorReporter.h>
#include <Random.h>
#include <Timing.h>
#include <Navigation/Squares.h>
#include <Navigation/Grid.h>
#include <Navigation/Navigation.h>
#include <Navigation/Maze.h>
#include <Input/AllegroInput.h>
#include <Text/TextField.h>
#include <Utils/PanZoomer.h>
#include <Debug/ImGuiUtils.h>
#include <Debug/AllegroImGuiDebugger.h>

using namespace gamelib;
using namespace gamelib::squares;

struct Map;

/*
struct ObjectType
{
	std::string Name;
	ObjectClass Class;
	ALLEGRO_BITMAP* Texture = nullptr;
	uvec2 BaseSize{ 1,1 };
};

enum class ObjectFlags
{
	CustomFlags = 32
};
*/
struct TileObject
{
	TileObject(Map* pm, ivec2 at) : mParentMap(pm), mPosition(at) {}
	virtual ~TileObject() = default;

	Color Tint = Colors::White;
	ivec2 Size{ 1,1 };
	Direction WallPosition = Direction::None;
	//enum_flags<ObjectFlags> Flags;
	int RotationFlags = 0;

	ALLEGRO_BITMAP* Texture;

	Map* ParentMap() const { return mParentMap; }
	ivec2 Position() const { return mPosition; }

	void MoveTo(ivec2 pos);

	virtual std::string Name() const { return "[object]"; }
	virtual std::string Image() const { return "error"; }

	virtual bool Visible() const { return true; }
	virtual bool BlocksMovement() const { return false; }

protected:

	Map* mParentMap = nullptr;
	ivec2 mPosition{};
};

struct Furniture : TileObject
{
	using TileObject::TileObject;
	bool Searched = false;
	virtual bool BlocksMovement() const override { return true; }
	virtual std::string Name() const  override{ return "Furniture"; }
	virtual std::string Image() const override { return "obj/furniture/rubble"; }
};

struct Monster : TileObject
{
	using TileObject::TileObject;
	virtual bool BlocksMovement() const override { return true; }
	virtual std::string Name() const  override { return "Monster"; }
	virtual std::string Image() const override { return "obj/monsters/goblin"; }
};

struct Hero : TileObject
{
	using TileObject::TileObject;
	virtual std::string Name() const override { return "Hero"; }
	virtual std::string Image() const override { return "obj/heroes/barbarian"; }
	virtual bool BlocksMovement() const override { return true; }
};

struct Door : TileObject
{
	using TileObject::TileObject;
	uint32_t Locked = 0;
	virtual std::string Name() const  override { return "Door"; }
};

struct Stairs : TileObject
{
	Stairs(Map* pm, ivec2 at) : TileObject(pm, at) { Size = { 2,2 }; }
	virtual std::string Image() const override { return "obj/floorobjs/stairway"; }
	virtual std::string Name() const  override { return "Stairs"; }
};

struct Trap : TileObject
{
	using TileObject::TileObject;
	bool Sprung = false;
	virtual std::string Name() const  override { return "Trap"; }
	virtual std::string Image() const override { return "obj/traps/pit"; }
};

struct Item : TileObject
{
	using TileObject::TileObject;
	virtual std::string Name() const  override { return "Item"; }
	virtual std::string Image() const override { return "obj/treasure"; }
};

struct Trigger : TileObject
{
	using TileObject::TileObject;
	virtual std::string Name() const  override { return "Trigger"; }
};

struct RoomTile
{
	ALLEGRO_BITMAP* Bg = nullptr;
	int RotationFlags = 0;

	std::set<TileObject*> Objects;
};

constexpr float tile_width = 128;
constexpr vec2 tile_size = { tile_width, tile_width };

struct Map
{
	Grid<RoomTile> RoomGrid;
	WallNavigationGrid NavGrid;
	std::vector<std::unique_ptr<TileObject>> Objects;

	Grid<RoomTile>* operator->() { return &RoomGrid; }
	Grid<RoomTile> const* operator->() const { return &RoomGrid; }

	template <typename T, typename... ARGS>
	T* SpawnObject(ivec2 pos, ARGS&&... args)
	{
		if (!RoomGrid.IsValid(pos)) return nullptr;

		auto obj = std::make_unique<T>(this, pos, std::forward<ARGS>(args)...);
		auto ptr = (TileObject*)obj.get();
		for (int x = 0; x < ptr->Size.x; x++)
			for (int y = 0; y < ptr->Size.y; y++)
				RoomGrid.At(pos + ivec2{x, y})->Objects.insert(ptr);
		Objects.push_back(std::move(obj));
		return (T*)ptr;
	}

	void BuildRoom(irec2 const& room);

	void DetermineVisibility(vec2 from_position);

	Map();
};

struct Game
{
	void Init();

	//ALLEGRO_BITMAP* tiles[7];
	std::map<InputID, ALLEGRO_BITMAP*, std::less<>> input_gfx;
	//ALLEGRO_BITMAP* characters[4]{};

	void Load();

	void Start();

	void Loop();

	void NewFrame();

	void Events();

	void Update();

	void Debug();

	void Render();

	void UpdateCamera();
	
	template <typename... ARGS>
	void DrawText(ALLEGRO_FONT* font, vec2 position, Color const& color, Color const& background_color, HorizontalAlign align, std::string_view format, ARGS&&... args)
	{
		auto str = fmt::format(format, std::forward<ARGS>(args)...);

		ALLEGRO_USTR_INFO info{};
		auto buf = al_ref_buffer(&info, str.data(), str.size());

		int x, y, w, h;
		al_get_ustr_dimensions(font, buf, &x, &y, &w, &h);

		position.x += AlignAxis((float)w, 0.0f, align);

		if (background_color.a != 0)
		{
			al_draw_filled_rectangle(x + position.x - 4, y + position.y - 4, x + position.x + w + 4, y + position.y + h + 4, ToAllegro(background_color));
		}

		al_draw_ustr(font, ToAllegro(Contrasting(color)), position.x + 1, position.y + 1, ALLEGRO_ALIGN_LEFT, buf);
		al_draw_ustr(font, ToAllegro(color), position.x, position.y, ALLEGRO_ALIGN_LEFT, buf);
	}

	void Shutdown();

private:

	bool CanMoveIn(Direction move_dir);
	bool CanMoveTo(ivec2 pos);
	void MovePlayer(Direction move_dir);

	struct Command
	{
		std::string Text;
		InputID Input = InvalidInput;
		std::function<void()> Func;
	};

	std::vector<Command> mCommands;

	void AddCommand(InputID input, std::string_view text, std::function<void()> func);

	auto GetMouseWorldPosition() const { return mCamera.ScreenSpaceToWorldSpace(mInput.GetMousePosition()); }
	auto GetMouseTilePosition() const { return mCurrentMap.RoomGrid.WorldPositionToTilePosition(GetMouseWorldPosition(), tile_size); }
	auto GetMouseTile() { return mCurrentMap.RoomGrid.At(GetMouseTilePosition()); }

	ALLEGRO_DISPLAY* mDisplay = nullptr;
	ALLEGRO_EVENT_QUEUE* mQueue = nullptr;
	rec2 mScreenRect{};
	ALLEGRO_FONT* mFont = nullptr;

	IErrorReporter mReporter;
	TimingSystem mTiming{ al_get_time };
	AllegroInput mInput{ mReporter };
	ICamera mCamera;
	PanZoomer mPanZoomer{ mInput, mCamera };
	AllegroImGuiDebugger mDebugger;

	bool mQuit = false;
	double mDT = 0;

	float mFontHeight;

	ALLEGRO_TRANSFORM mUICamera{};

	std::mt19937_64 RNG;

	Page mTileDescription;

	Map mCurrentMap;
	TileObject* mPlayer = nullptr;

	std::map<std::string, ALLEGRO_BITMAP*, std::less<>> mImages;
};