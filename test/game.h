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
#include <Support/IOC.h>
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
#include <Resources/Files.h>

#include <rsl/RSL.h>
#include <rsl/ExecutionContext.h>
#include <rsl/CppInterop.h>

using namespace gamelib;
using namespace gamelib::squares;

struct Game;

namespace glm
{
	inline rsl::Value to_value(rsl::ExecutionContext const& exe, ivec2 const& v)
	{
		return exe.ToValue(v.x, v.y);
	}
}

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

	void EndMove();

	template <typename... ARGS>
	void DrawText(ALLEGRO_FONT* font, vec2 position, Color const& color, Color const& background_color, Align align, std::string_view format, ARGS&&... args)
	{
		auto str = fmt::format(format, std::forward<ARGS>(args)...);

		ALLEGRO_USTR_INFO info{};
		auto buf = al_ref_buffer(&info, str.data(), str.size());

		int x, y, w, h;
		al_get_ustr_dimensions(font, buf, &x, &y, &w, &h);

		position.x += AlignAxis((float)w, 0.0f, Horizontal(align));
		position.y += AlignAxis((float)h, 0.0f, Vertical(align));

		if (background_color.a != 0)
		{
			const auto box_offset = std::max(h / 4, 4);
			al_draw_filled_rectangle(x + position.x - box_offset, y + position.y - box_offset, x + position.x + w + box_offset, y + position.y + h + box_offset, ToAllegro(background_color));
		}

		const auto shadow_offset = std::max(h / 16, 1);
		al_draw_ustr(font, ToAllegro(Contrasting(color)), position.x + shadow_offset, position.y + shadow_offset, ALLEGRO_ALIGN_LEFT, buf);
		al_draw_ustr(font, ToAllegro(color), position.x, position.y, ALLEGRO_ALIGN_LEFT, buf);
	}

	void Shutdown();

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

	bool mQuit = false;
	double mDT = 0;

	ALLEGRO_TRANSFORM mUICamera{};

	std::mt19937_64 RNG;

	Page mTileDescription;

	std::map<std::string, ALLEGRO_BITMAP*, std::less<>> mImages;

	ALLEGRO_BITMAP* GetImage(std::string_view name) const;

	std::vector<std::function<bool(seconds_t)>> mAnimators;
};