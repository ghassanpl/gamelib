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
#include <Align.h>
#include <Input/AllegroInput.h>
#include <Timing.h>
#include <Camera.h>
#include <Random.h>
#include <Text/TextField.h>
#include <Debug/AllegroImGuiDebugger.h>
#include <Navigation/Grid.h>
#include <Includes/Assuming.h>

#include <filesystem>

using namespace gamelib;

static constexpr int TILE_SIZE = 16;

using Bitmap = std::shared_ptr<ALLEGRO_BITMAP>;

enum class TileType
{
	Air,
	Block,
	OneWayPlatform,
	Spikes,
	KeyBlock,
	Bouncy,
	Door,
	Dirt,
	Checkpoint,
	Fence,
	Ladder,
	Disappearing,
	Teleporter,
	Lever,
	Zappy,
	BombableBlock,
	RecursiveBlock,
};

struct Tile
{
	size_t BG = 0;
	size_t Wall = 0;
	TileType Type = TileType::Air;
	size_t Mem = 0;
};

struct Animation;
struct AnimationFrame;
struct AnimationManager;

struct Object
{
	virtual ~Object() noexcept = default;

	bool Alive = true;
	vec2 Position = {};
	ivec2 Size = { TILE_SIZE / 2, TILE_SIZE / 2 };
	vec2 SpriteOffset = {};
	seconds_t Life = {};
	AnimationManager const* AnimSource = nullptr;
	
	virtual void Update(seconds_t dt);
	AnimationFrame const& Frame() const;

	void PlayAnim(std::string_view anim);

protected:

	seconds_t mAnimTime = 0;
	Animation const* mCurrentAnim = nullptr;
};

struct Mob : Object
{
	vec2 Acceleration = {};
	vec2 Velocity = {};

	vec2 PrevPosition = {};
	vec2 PrevVelocity = {};

	virtual void Update(seconds_t dt) override;
};

struct Player : Mob
{

};

struct Enemy : Mob
{
	size_t EnemyIndex = 0;
	int HP = 1;
};

struct ObjectDef
{
	size_t Type = 0;
	ivec2 TilePos = {};
	size_t Mem = 0;
	bool Killed = false;
};

struct Tileset
{
	void Load(std::filesystem::path path, ivec2 frame_counts);

	vec2 PosForID(size_t id) const
	{
		const auto x = id % mFrameCounts.x;
		const auto y = id / mFrameCounts.x;
		return { x * TILE_SIZE, y * TILE_SIZE };
	}

	void DrawTile(size_t id, ivec2 pos) const;
	void DrawTile(size_t id, vec2 pos) const;

	Bitmap const& Image() const { return mImage; }

private:

	Bitmap mImage = {};
	ivec2 mFrameCounts{};

};

struct AnimationFrame
{
	Bitmap Image = {};
	ivec2 Pos{};
	vec2 Offset = {};
};

struct Animation
{
	std::vector<AnimationFrame> Frames;
	seconds_t FrameTime = 0.1;

	AnimationFrame const& FrameAtTime(seconds_t time, seconds_t* out_time_in_frame = nullptr) const;

	template <std::convertible_to<ivec2>... FRAMES>
	void Set(seconds_t frame_time, Bitmap img, FRAMES... pos)
	{
		FrameTime = frame_time;
		(Frames.push_back({ img, pos }), ...);
	}

	template <std::convertible_to<size_t>... FRAMES>
	void Set(seconds_t frame_time, Tileset const& tileset, FRAMES... pos)
	{
		FrameTime = frame_time;
		(Frames.push_back({ tileset.Image(), tileset.PosForID(pos) }), ...);
	}

};

struct AnimationManager
{
	std::map<std::string, Animation, std::less<>> Animations;

	Animation const* FindAnimation(std::string_view animation) const;

	template <std::convertible_to<ivec2>... FRAMES>
	void AddAnimation(std::string name, seconds_t frame_time, Bitmap img, FRAMES... pos)
	{
		Assuming(!Animations.contains(name));
		Animations[name].Set(frame_time, std::move(img), pos...);
	}

	template <std::convertible_to<size_t>... FRAMES>
	void AddAnimation(std::string name, seconds_t frame_time, Tileset const& tileset, FRAMES... pos)
	{
		Assuming(!Animations.contains(name));
		Animations[name].Set(frame_time, tileset, pos...);
	}
};

struct Level
{
	std::string Name;
	gamelib::squares::Grid<Tile> Tiles;
	std::vector<ObjectDef> Objects;
};

struct Game
{
	std::map<InputID, Bitmap, std::less<>> input_gfx;

	Tileset LevelTiles;
	Tileset PlayerImages;
	Level CurrentLevel;
	std::vector<std::unique_ptr<Object>> LevelObjects;
	Player* Gostek = nullptr;

	AnimationManager PlayerAnimations;

	void Init();

	void Load();

	void Start();

	void Loop();

	void NewFrame();

	void Events();

	void Update();

	void Debug();

	void Render();

	void EndMove();

	Bitmap LoadBitmap(std::filesystem::path path);

	template <typename... ARGS>
	void DrawTextV(ALLEGRO_FONT* font, vec2 position, Color const& color, Color const& background_color, Align align, std::string_view format, ARGS&&... args)
	{
		auto str = fmt::format(format, std::forward<ARGS>(args)...);
		DrawText(font, position, color, background_color, align, str);
	}

	void DrawText(ALLEGRO_FONT* font, vec2 position, Color const& color, Color const& background_color, Align align, std::string_view str);

	void Shutdown();

	IErrorReporter& ErrorReporter() const { return *mReporter; }

	/// debug vars

	float gravity = 500.0f;
	float zoom = 5.0f;
	ivec2 start_pos = vec2{ 3,3 };
	float jump_velocity = -200.0f;
	float move_accel = 120.0f;
	float move_decel = 20.0f;

	bool draw_tiles = false;
	bool draw_objects = false;

private:

	ALLEGRO_DISPLAY* mDisplay = nullptr;
	ALLEGRO_EVENT_QUEUE* mQueue = nullptr;
	rec2 mScreenRect{};
	ALLEGRO_FONT* mFont = nullptr;
	ALLEGRO_FONT* mBigFont = nullptr;

	TimingSystem mTiming{ al_get_time };
	std::shared_ptr<IErrorReporter> mReporter;
	std::shared_ptr<AllegroInput> mInput;
	std::shared_ptr<IDebugger> mDebugger;

	ICamera mCamera;
	vec2 mCameraTarget{};
	float mCameraSpeed = 15.0f;
	bool mCameraFocus = true;

	bool mCanJump = false;
	bool mJumping = false;

	bool mQuit = false;
	double mDT = 0;

	ALLEGRO_TRANSFORM mUICamera{};

	std::mt19937_64 RNG;

	std::vector<std::function<bool(seconds_t)>> mAnimators;

	std::map<std::filesystem::path, Bitmap> mBitmaps;
};
