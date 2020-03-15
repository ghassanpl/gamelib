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

struct TileObject
{
	TileObject(Map* pm, ivec2 at) : mParentMap(pm), mPosition(at) {}

	Color Tint = Colors::White;
	uvec2 Size{ 1,1 };
	Direction WallPosition = Direction::None;
	ALLEGRO_BITMAP* Texture = nullptr;
	int RotationFlags = 0;

	Map* ParentMap() const { return mParentMap; }
	ivec2 Position() const { return mPosition; }

	void MoveTo(ivec2 pos);

protected:

	Map* mParentMap = nullptr;
	ivec2 mPosition{};
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
		auto ptr = obj.get();
		RoomGrid.At(pos)->Objects.insert(ptr);
		Objects.push_back(std::move(obj));
		return ptr;
	}

	void BuildRoom(irec2 const& room, ALLEGRO_BITMAP* bg);

	void DetermineVisibility(vec2 from_position);

	Map();
};

struct Game
{
	void Init();

	ALLEGRO_BITMAP* tiles[7];
	std::map<InputID, ALLEGRO_BITMAP*> input_gfx;
	ALLEGRO_BITMAP* characters[4]{};

	void Load();

	void Start();

	void Loop();

	void NewFrame();

	void Events();

	void Update();

	void Debug();

	void Render();

	void UpdateCamera();;
	
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
	ivec2 mScreenSize{};
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

	Map mCurrentMap;
	TileObject* mPlayer = nullptr;
};