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
#include <Serialization/IArchiver.h>
#include <Serialization/IOStreamBuffers.h>
#include <Serialization/CSV.h>
#include <Debug/AllegroImGuiDebugger.h>

using namespace gamelib;
using namespace gamelib::squares;

struct Map;

enum class ObjectZ
{
	UnderFloor = -1,
	Floor = 0,
	Furniture,
	Items,
	Monsters,
	Heroes,
	Walls,

	Ceiling = 100,
};

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

	virtual ObjectZ Z() const { return ObjectZ::Furniture; }

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
	virtual std::string Name() const override { return "Furniture"; }
	virtual std::string Image() const override { return "obj/furniture/rubble"; }
};

struct MonsterClass
{
	std::string Name;
	std::string Image;
	int Health = 1;
	int Attack = 2;
	int Defense = 5;
	int Damage = 1;
	int Speed = 3;
	int XP = 1;
	std::map<std::string, std::string> Abilities;
	std::vector<std::string> Traits;
	//std::vector<std::string> Barks;
	std::string Barks;

	template<class Archive>
	void Archive(Archive& ar)
	{
		ar& ARCHIVE_NVP(Name);
		ar& ARCHIVE_NVP(Image);
		ar& ARCHIVE_NVP(Health);
		ar& ARCHIVE_NVP(Attack);
		ar& ARCHIVE_NVP(Defense);
		ar& ARCHIVE_NVP(Damage);
		ar& ARCHIVE_NVP(Speed);
		ar& ARCHIVE_NVP(XP);
		ar& ARCHIVE_NVP(Abilities);
		ar& ARCHIVE_NVP(Traits);
		ar& ARCHIVE_NVP(Barks);
	}
};

struct Monster : TileObject
{
	Monster(Map* pm, ivec2 at, MonsterClass const& klass) : TileObject(pm, at), Class(&klass) {}

	MonsterClass const* Class = nullptr;

	virtual bool BlocksMovement() const override { return true; }
	virtual std::string Name() const override { return Class->Name; }
	virtual std::string Image() const override { return Class->Image; }

	virtual ObjectZ Z() const override { return ObjectZ::Monsters; }
};

struct HeroClass
{
	std::string Name;
	int Health;
	int Attack;
	int Defense;
	int Speed;
	int MagicPower;
	int Strength;
	std::string Image;
	std::vector<std::string> StartingEquipment;
	std::vector<std::string> StartingSkills;
	std::string Motto;

	template<class Archive>
	void Archive(Archive& ar)
	{
		ar& ARCHIVE_NVP(Name);
		ar& ARCHIVE_NVP(Health);
		ar& ARCHIVE_NVP(Attack);
		ar& ARCHIVE_NVP(Defense);
		ar& ARCHIVE_NVP(Speed);
		ar& ARCHIVE_NVP(MagicPower);
		ar& ARCHIVE_NVP(Strength);
		ar& ARCHIVE_NVP(Image);
		ar& ARCHIVE_NVP(StartingEquipment);
		ar& ARCHIVE_NVP(StartingSkills);
		ar& ARCHIVE_NVP(Motto);
	}
};

struct Hero : TileObject
{
	HeroClass const* Class = nullptr;

	Hero(Map* pm, ivec2 at, HeroClass const& klass) : TileObject(pm, at), Class(&klass) {}

	virtual std::string Name() const override { return Class->Name; }
	virtual std::string Image() const override { return Class->Image; }
	virtual bool BlocksMovement() const override { return true; }
	virtual ObjectZ Z() const override { return ObjectZ::Heroes; }
};

struct Door : TileObject
{
	using TileObject::TileObject;
	uint32_t Locked = 0;
	virtual std::string Name() const override { return "Door"; }
	virtual std::string Image() const override { return "obj/doors/stone_closed"; }
	virtual ObjectZ Z() const override { return ObjectZ::Walls; }
};

struct Stairs : TileObject
{
	Stairs(Map* pm, ivec2 at) : TileObject(pm, at) { Size = { 2,2 }; }
	virtual std::string Image() const override { return "obj/floorobjs/stairway"; }
	virtual std::string Name() const override { return "Stairs"; }
	virtual ObjectZ Z() const override { return ObjectZ::Floor; }
};

struct Trap : TileObject
{
	using TileObject::TileObject;
	bool Sprung = false;
	virtual std::string Name() const override { return "Trap"; }
	virtual std::string Image() const override { return "obj/traps/pit"; }
	virtual ObjectZ Z() const override { return ObjectZ::Floor; }
};

struct ItemClass
{
	std::string Name;
	int Cost;
	std::string Image;
	std::vector<int> Requirements;
	std::vector<std::string> Powers;
	std::vector<std::string> Traits;
	std::string Fluff;
};

struct WeaponClass : ItemClass
{
	//WeaponType Type;
	//AmmoType Ammo;
	//int AmmoStart;
	int Damage;
	//DamageType DamageType;
	int Range = 1;
	bool Diagonal = false;
};

struct Item : TileObject
{
	using TileObject::TileObject;
	
	ItemClass const* Class = nullptr;

	virtual std::string Name() const override { return "Item"; }
	virtual std::string Image() const override { return "obj/treasure"; }
	virtual ObjectZ Z() const override { return ObjectZ::Items; }
};

struct Trigger : TileObject
{
	using TileObject::TileObject;

	virtual bool Visible() const { return false; }
	virtual std::string Name() const override { return "Trigger"; }
	virtual ObjectZ Z() const override { return ObjectZ::Floor; }
};

struct RoomTile
{
	ALLEGRO_BITMAP* Bg = nullptr;
	int RotationFlags = 0;

	std::set<TileObject*> Objects;
};

constexpr float tile_width = 128;
constexpr vec2 tile_size = { tile_width, tile_width };
constexpr float half_tile_size = tile_width / 2;
constexpr auto wall_width = 10;
constexpr auto shadow_size = 20;

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

	void BuildRoom(irec2 const& room, ivec2 door_pos, Direction door_dir);

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

	void DrawObjects(gsl::span<TileObject* const> objects, ivec2 pos);

	template <typename FUNC>
	void ForEachVisibleTile(FUNC&& func)
	{
		for (int y = 0; y < mCurrentMap.RoomGrid.Height(); y++)
		{
			for (int x = 0; x < mCurrentMap.RoomGrid.Width(); x++)
			{
				auto pos = ivec2{ x, y };
				auto tile = mCurrentMap.RoomGrid.At(pos);
				func(pos, tile, vec2(pos) * tile_width);
			}
		}
	}

	ALLEGRO_DISPLAY* mDisplay = nullptr;
	ALLEGRO_EVENT_QUEUE* mQueue = nullptr;
	rec2 mScreenRect{};
	ALLEGRO_FONT* mFont = nullptr;

	IErrorReporter mReporter;
	TimingSystem mTiming{ al_get_time };
	AllegroInput mInput{ mReporter };
	ICamera mCamera;
	vec2 mCameraTarget{};
	float mCameraSpeed = 15.0f;
	bool mCameraFocus = true;
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

	std::map<std::string, HeroClass, std::less<>> mHeroClasses;
	std::map<std::string, MonsterClass, std::less<>> mMonsterClasses;
	std::map<std::string, ItemClass, std::less<>> mItemClasses;
	std::map<std::string, WeaponClass, std::less<>> mWeaponClasses;

	template <typename T>
	void LoadClasses(path from_file, std::map<std::string, T, std::less<>>& map)
	{
		std::ifstream file{ from_file };
		if (file.peek() == 0xEF) /// FUCK UTF8 BOMS
			file.ignore(3);

		try
		{
			json csv = LoadCSV(file);
			for (auto& row : csv)
			{
				archive::JsonArchiver archiver{ mReporter, row };
				auto& klass = map[(std::string)archiver.CurrentObject->at("Name")];
				klass.Archive(archiver);
			}
		}
		catch (Reporter& e)
		{
			e.AdditionalInfo("InFile", from_file.string());
			e.Perform();
		}
	}

	std::map<std::string, ALLEGRO_BITMAP*, std::less<>> mImages;
};