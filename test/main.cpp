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
#include <Utils/PanZoomer.h>
#include <Debug/ImGuiUtils.h>
#include <Debug/AllegroImGuiDebugger.h>
//#include <Resources/Map.h>

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

	void BuildRoom(irec2 const& room, ALLEGRO_BITMAP* bg)
	{
		NavGrid.SetBlocking(room, { WallBlocks::Passage, WallBlocks::Sight }, true);
		RoomGrid.ForEachInRect(room, [this, bg](ivec2 pos) {
			auto tile = RoomGrid.At(pos);
			tile->Bg = bg;
		});
	}

	void DetermineVisibility(vec2 from_position)
	{
		NavGrid.SetAllVisible(false);

		for (int y = 0; y < NavGrid.Height(); y++)
		{
			for (int x = 0; x < NavGrid.Width(); x++)
			{
				NavGrid.BaseNavigationGrid::SegmentCast(tile_size, from_position, vec2{ x,y }*tile_size + tile_size / 2.0f,
					[&](ivec2 from, ivec2 to) {
						return !NavGrid.BlocksPassage(from, to);
					},
					[&](ivec2 entered) {
						NavGrid.SetVisible(entered, true);
						NavGrid.SetWasSeen(entered, true);
					}
				);
			}
		}
	}

	Map()
	{
		RoomGrid.Reset(26, 19);
		NavGrid.Reset(26, 19);
		NavGrid.BuildWalls<ghassanpl::flag_bits(BlockNavigationGrid::IterationFlags::OnlyValid)>([&](ivec2 from, ivec2 to) {
			return NavGrid.IsValid(to) ? enum_flags<WallBlocks>{} : enum_flags<WallBlocks>::all(WallBlocks::Sight);
		});
	}
};

int ToAllegro(HorizontalAlign align)
{
	switch (align)
	{
	case HorizontalAlign::Center:
		return ALLEGRO_ALIGN_CENTER;
	case HorizontalAlign::Right:
		return ALLEGRO_ALIGN_RIGHT;
	default:
		return ALLEGRO_ALIGN_LEFT;
	}
}

ALLEGRO_DISPLAY* mDisplay = nullptr;
ivec2 mScreenSize{};
ALLEGRO_FONT* mFont = nullptr;

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

struct Command
{
	std::string Text;
	InputID Input = InvalidInput;
	std::function<void()> Func;
};

std::vector<Command> commands;

void AddCommand(InputID input, std::string_view text, std::function<void()> func)
{
	commands.push_back(Command{
		.Text = (std::string)text,
		.Input = input,
		.Func = std::move(func)
	});
}

int main()
{
	auto q = Interpolate(InterpolationFunction::Cosine, double{});
	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	al_install_keyboard();
	al_install_mouse();

	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);

	mDisplay = al_create_display(1280, 720);
	auto queue = al_create_event_queue();

	ImGui::Allegro::Init(mDisplay);

	mScreenSize = ivec2{ al_get_display_width(mDisplay), al_get_display_height(mDisplay) };

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(mDisplay));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	/// al_load_bitmap
	/// al_set_blender
	/*
	AL_FUNC(void, al_set_org_name, (const char *org_name));
	AL_FUNC(void, al_set_app_name, (const char *app_name));
	*/

	mFont = al_load_ttf_font("data/fonts/plantin_regular.ttf", 36, 0);
	const auto font_height = al_get_font_line_height(mFont);

	IErrorReporter reporter;
	TimingSystem timing{ al_get_time };
	AllegroInput input{ reporter };
	input.Init();
	input.MapKeyAndButton('up', KeyboardKey::W, XboxGamepadButton::Up);
	input.MapKeyAndButton('down', KeyboardKey::S, XboxGamepadButton::Down);
	input.MapKeyAndButton('left', KeyboardKey::A, XboxGamepadButton::Left);
	input.MapKeyAndButton('righ', KeyboardKey::D, XboxGamepadButton::Right);
	input.MapMouse(MouseButton::Left, 'act');
	input.MapMouse(MouseButton::Right, 'exam');
	input.MapMouse(MouseButton::Middle, 'addi');
	ICamera camera{ mDisplay };
	PanZoomer pz{ input, camera };
	AllegroImGuiDebugger debugger;

	std::mt19937_64 rng;

	ALLEGRO_BITMAP* tiles[7];
	al_add_new_bitmap_flag(ALLEGRO_MIPMAP);
	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_MAG_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	tiles[0] = al_load_bitmap("data/tile.png");
	tiles[1] = al_load_bitmap("data/tile2.png");
	tiles[2] = al_load_bitmap("data/tile3.png");
	tiles[3] = al_load_bitmap("data/tile4.png");
	tiles[4] = al_load_bitmap("data/green.png");
	tiles[5] = al_load_bitmap("data/beige.png");
	tiles[6] = al_load_bitmap("data/yellow.png");

	std::map<InputID, ALLEGRO_BITMAP*> input_gfx;
	input_gfx['up'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_W.png");
	input_gfx['down'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_S.png");
	input_gfx['left'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_A.png");
	input_gfx['righ'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_D.png");
	input_gfx['act'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Left.png");
	input_gfx['exam'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Right.png");
	input_gfx['addi'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Middle.png");

	ALLEGRO_BITMAP* characters[4]{};
	characters[0] = al_load_bitmap("data/barbarian_shadow.png");

	Map map;
	map.RoomGrid.ForEach([&](ivec2 pos) {
		auto tile = map.RoomGrid.At(pos);
		tile->Bg = tiles[random::IntegerRange(rng, 0, 3)];
		tile->RotationFlags = random::IntegerRange(rng, 0, 15);
	});
	map.BuildRoom(irec2::from_size(1, 1, 4, 3), tiles[4]);
	map.BuildRoom(irec2::from_size(5, 1, 4, 3), tiles[5]);
	map.BuildRoom(irec2::from_size(1, 4, 4, 5), tiles[6]);
	map.NavGrid.SetBlocksPassage({ 2,0 }, { 2,1 }, false);

	auto player = map.SpawnObject<TileObject>({ 0,0 });
	player->Texture = characters[0];

	auto UpdateCamera = [&] {
		auto player_world_pos = map.RoomGrid.TilePositionToWorldPosition(player->Position(), tile_size) + tile_size / 2.0f;
		camera.SetWorldCenter(player_world_pos);
		map.DetermineVisibility(player_world_pos);
	};
	UpdateCamera();

	auto MovePlayer = [&](Direction move_dir) {
		const auto target_pos = player->Position() + ToVector(move_dir);
		if (!map.NavGrid.BlocksPassage(player->Position(), target_pos))
		{
			player->MoveTo(target_pos);

			UpdateCamera();
		}
	};

	timing.Reset();

	ALLEGRO_TRANSFORM ui_camera{};
	al_identity_transform(&ui_camera);

	bool quit = false;
	while (!quit)
	{
		input.Update();
		timing.Update();

		auto dt = timing.TimeSinceLastFrame();

		ImGui::Allegro::NewFrame(mDisplay, dt);

		commands.clear();

		debugger.Value("FPS", 1.0/dt);

		ALLEGRO_EVENT event{};
		while (al_get_next_event(queue, &event))
		{
			switch (event.type)
			{
				/// ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY, ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				quit = true;
				break;
			}

			if (!ImGui::IsAnyWindowHovered())
				input.ProcessEvent(event);
			ImGui::Allegro::ProcessEvent(&event);
		}

		/// //////////////////////////// ///
		/// TODO: Update
		/// //////////////////////////// ///

		pz.Update(dt);

		Direction move_dir = Direction::None;
		if (input.WasButtonPressed('up'))
			MovePlayer(Direction::Up);
		else if (input.WasButtonPressed('left'))
			MovePlayer(Direction::Left);
		else if (input.WasButtonPressed('righ'))
			MovePlayer(Direction::Right);
		else if (input.WasButtonPressed('down'))
			MovePlayer(Direction::Down);

		auto mouse_world_pos = camera.ScreenSpaceToWorldSpace(input.GetMousePosition());
		auto mouse_tile_pos = map.RoomGrid.WorldPositionToTilePosition(mouse_world_pos, tile_size);
		auto mouse_tile = map.RoomGrid.At(mouse_tile_pos);

		if (map->IsValid(mouse_tile_pos) && IsNeighbor(mouse_tile_pos, player->Position()) && !map.NavGrid.BlocksPassage(mouse_tile_pos, player->Position()))
		{
			AddCommand('act', "Move", [&] {
				MovePlayer(ToDirection(mouse_tile_pos-player->Position()));
			});
		}

		for (auto& cmd : commands)
		{
			if (input.WasButtonPressed(cmd.Input))
				cmd.Func();
		}

		debugger.Value("Mouse World Pos", mouse_world_pos);
		debugger.Value("Mouse Tile Pos", mouse_tile_pos);

		if (ImGui::BeginTabBar("tabs"))
		{
			if (ImGui::BeginTabItem("Input"))
			{
				input.Debug(debugger);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		/// //////////////////////////// ///
		/// Draw
		/// //////////////////////////// ///

		al_clear_to_color(ToAllegro(Colors::Black));

		al_use_transform(&camera.GetTransform());

		/// Floors
		constexpr float half_tile_size = tile_width / 2;
		for (int y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_width;
			for (int x = 0; x < map.RoomGrid.Width(); x++)
			{
				if (!map.NavGrid.WasSeen({ x, y })) continue;

				auto xp = x * tile_width;
				auto tile = map.RoomGrid.At(x, y);
				al_draw_rotated_bitmap(tile->Bg, half_tile_size, half_tile_size, xp + half_tile_size, yp + half_tile_size, glm::radians((tile->RotationFlags>>2)*90.0f), tile->RotationFlags&3);
			}
		}

		/// Objects
		for (int y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_width;
			for (int x = 0; x < map.RoomGrid.Width(); x++)
			{
				if (!map.NavGrid.WasSeen({ x, y })) continue;

				auto xp = x * tile_width;
				auto tile = map.RoomGrid.At(x, y);

				for (auto obj : tile->Objects)
				{
					al_draw_rotated_bitmap(obj->Texture, half_tile_size, half_tile_size, xp + half_tile_size, yp + half_tile_size, glm::radians((obj->RotationFlags >> 2) * 90.0f), obj->RotationFlags & 3);
				}
			}
		}

		/// Walls
		const auto wall_color = ToAllegro(Colors::LightGray);
		constexpr auto wall_width = 10;
		constexpr auto shadow_size = 20;
		for (int y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_width;
			for (int x = 0; x < map.RoomGrid.Width(); x++)
			{
				if (!map.NavGrid.WasSeen({ x, y })) continue;

				auto xp = x * tile_width;
				auto adj = map.NavGrid.At(x, y)->BlocksWall(WallBlocks::Passage);
				if (adj.is_set(Direction::Left)) al_draw_line(xp, yp - wall_width / 2, xp, yp + tile_width + wall_width / 2, wall_color, wall_width);
				if (adj.is_set(Direction::Up)) al_draw_line(xp - wall_width / 2, yp, xp + tile_width + wall_width / 2, yp, wall_color, wall_width);
				if (adj.is_set(Direction::Right)) al_draw_line(xp + tile_width, yp, xp + tile_width, yp + tile_width, wall_color, wall_width);
				if (adj.is_set(Direction::Down)) al_draw_line(xp - wall_width / 2, yp + tile_width, xp + tile_width + wall_width / 2, yp + tile_width, wall_color, wall_width);

			}
		}

		static constexpr auto half_black = Colors::GetBlack(0.5f);
		for (int y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_width;
			for (int x = 0; x < map.RoomGrid.Width(); x++)
			{
				auto xp = x * tile_width;
				if (!map.NavGrid.Visible({ x, y }))
				{
					al_draw_filled_rectangle(xp, yp, xp + tile_width, yp + tile_width, ToAllegro(half_black));
				}
			}
		}

		/// Mouse selection
		if (mouse_tile)
		{
			auto select = map.RoomGrid.RectForTile(mouse_tile_pos, tile_size);
			//al_draw_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, ToAllegro(Colors::Red), 6);
			al_draw_filled_rounded_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, 4.0f, 4.0f, ToAllegro(Colors::GetWhite(0.5f)));
			al_draw_rounded_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, 4.0f, 4.0f, ToAllegro(Colors::Magenta), 4.0f);
		}

		al_use_transform(&ui_camera);

		rec2 selected_tile_desc_rect = rec2::from_size(16, 16, 300, 200);
		float y = 0;
		al_draw_filled_rounded_rectangle(selected_tile_desc_rect.p1.x, selected_tile_desc_rect.p1.y, selected_tile_desc_rect.p2.x, selected_tile_desc_rect.p2.y, 4.0f, 4.0f, ToAllegro(half_black));
		for (auto& cmd : commands)
		{
			auto glyph = input_gfx[cmd.Input];
			if (glyph) al_draw_scaled_bitmap(glyph, 0, 0, 100, 100, selected_tile_desc_rect.p1.x, selected_tile_desc_rect.p1.y, font_height, font_height, 0);
			DrawText(mFont, { selected_tile_desc_rect.p1.x + font_height, selected_tile_desc_rect.p1.x + y }, Colors::White, Colors::Transparent, HorizontalAlign::Left, "{}", cmd.Text);
			y += font_height + 4;
		}

		ImGui::Allegro::Render(mDisplay);

		al_flip_display();
	}

	ImGui::Allegro::Shutdown();

	al_destroy_event_queue(queue);
	al_destroy_display(mDisplay);

	al_uninstall_mouse();
	al_uninstall_keyboard();
	al_uninstall_system();
}

void TileObject::MoveTo(ivec2 pos)
{
	if (!mParentMap->RoomGrid.IsValid(pos)) return;

	mParentMap->RoomGrid.At(mPosition)->Objects.erase(this);
	mPosition = pos;
	mParentMap->RoomGrid.At(mPosition)->Objects.insert(this);
}
