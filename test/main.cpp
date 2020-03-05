#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

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
#include <boost/intrusive/list.hpp>
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

struct Map
{
	Grid<RoomTile> RoomGrid;
	WallNavigationGrid NavGrid;
	std::vector<std::unique_ptr<TileObject>> Objects;

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
		NavGrid.SetBlocking(room, WallBlocks::Passage);
		RoomGrid.ForEachInRect(room, [this, bg](ivec2 pos) {
			auto tile = RoomGrid.At(pos);
			tile->Bg = bg;
		});
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

int main()
{
	auto q = Interpolate(InterpolationFunction::Cosine, double{});
	al_init();
	al_init_primitives_addon();
	al_init_image_addon();

	al_install_keyboard();
	al_install_mouse();

	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);

	auto display = al_create_display(1280, 720);
	auto queue = al_create_event_queue();

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	/// al_load_bitmap
	/// al_set_blender
	/*
	AL_FUNC(void, al_set_org_name, (const char *org_name));
	AL_FUNC(void, al_set_app_name, (const char *app_name));
	*/

	IErrorReporter reporter;
	TimingSystem timing{ al_get_time };
	AllegroInput input{ reporter };
	input.Init();
	input.MapKeyAndButton('up', KeyboardKey::W, XboxGamepadButton::Up);
	input.MapKeyAndButton('down', KeyboardKey::S, XboxGamepadButton::Down);
	input.MapKeyAndButton('left', KeyboardKey::A, XboxGamepadButton::Left);
	input.MapKeyAndButton('righ', KeyboardKey::D, XboxGamepadButton::Right);
	ICamera camera{ display };
	PanZoomer pz{ input, camera };

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

	auto player = map.SpawnObject<TileObject>({ 0,0 });
	player->Texture = characters[0];

	timing.Reset();

	constexpr float tile_width = 128;
	constexpr vec2 tile_size = { tile_width, tile_width };

	bool quit = false;
	while (!quit)
	{
		input.Update();

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

			input.ProcessEvent(event);
		}

		timing.Update();

		/// //////////////////////////// ///
		/// TODO: Update
		/// //////////////////////////// ///

		pz.Update(timing.TimeSinceLastFrame());

		Direction move_dir = Direction::None;
		if (input.WasButtonPressed('up'))
			move_dir = Direction::Up;
		else if (input.WasButtonPressed('left'))
			move_dir = Direction::Left;
		else if (input.WasButtonPressed('righ'))
			move_dir = Direction::Right;
		else if (input.WasButtonPressed('down'))
			move_dir = Direction::Down;

		if (move_dir != Direction::None)
		{
			const auto target_pos = player->Position() + ToVector(move_dir);
			if (map.NavGrid.Passable(player->Position(), target_pos))
			{
				player->MoveTo(target_pos);
				camera.SetWorldCenter(map.RoomGrid.TilePositionToWorldPosition(player->Position(), tile_size) + tile_size / 2.0f);
				//map.NavGrid.CalculateFOV(target_pos, 26, false, );
			}
		}

		auto mouse_world_pos = camera.ScreenSpaceToWorldSpace(input.GetMousePosition());
		auto mouse_tile_pos = map.RoomGrid.WorldPositionToTilePosition(mouse_world_pos, tile_size);
		auto mouse_tile = map.RoomGrid.At(mouse_tile_pos);

		auto player_center = vec2{ player->Position() } * tile_size + tile_size / 2.0f;

		auto raycast_result = map.NavGrid.SegmentCast(tile_size, player_center, mouse_world_pos, WallBlocks::Passage);

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
				auto xp = x * tile_width;
				auto tile = map.RoomGrid.At(x, y);
				for (auto obj : tile->Objects)
				{
					al_draw_rotated_bitmap(obj->Texture, half_tile_size, half_tile_size, xp + half_tile_size, yp + half_tile_size, glm::radians((obj->RotationFlags >> 2) * 90.0f), obj->RotationFlags & 3);
				}
			}
		}

		/// Walls
		constexpr auto wall_width = 10;
		constexpr auto shadow_size = 20;
		for (int y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_width;
			for (int x = 0; x < map.RoomGrid.Width(); x++)
			{
				auto xp = x * tile_width;
				auto adj = map.NavGrid.At(x, y)->BlocksWall(WallBlocks::Passage);
				if (adj.is_set(Direction::Left))
				{
					float shadow[] = {
						xp, yp + tile_width,
						xp, yp,
						xp - shadow_size, (yp + shadow_size) ,
						xp - shadow_size, yp + tile_width + shadow_size ,
					};
					al_draw_filled_polygon(shadow, 4, ToAllegro(Colors::GetBlack(0.75f)));
					al_draw_line(xp, yp - wall_width / 2, xp, yp + tile_width + wall_width / 2, ToAllegro(Colors::White), wall_width);
				}
				if (adj.is_set(Direction::Up)) al_draw_line(xp - wall_width / 2, yp, xp + tile_width + wall_width / 2, yp, ToAllegro(Colors::White), wall_width);
				if (adj.is_set(Direction::Right)) al_draw_line(xp + tile_width, yp, xp + tile_width, yp + tile_width, ToAllegro(Colors::White), wall_width);
				if (adj.is_set(Direction::Down))
				{
					float shadow[] = {
						xp, yp + tile_width,
						xp - shadow_size, yp + tile_width + shadow_size,
						xp + tile_width - shadow_size, yp + tile_width + shadow_size,
						xp + tile_width, yp + tile_width,
					};
					al_draw_filled_polygon((float const*)shadow, 4, ToAllegro(Colors::GetBlack(0.75f)));
					al_draw_line(xp - wall_width / 2, yp + tile_width, xp + tile_width + wall_width / 2, yp + tile_width, ToAllegro(Colors::White), wall_width);
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

		al_draw_line(player_center.x, player_center.y, mouse_world_pos.x, mouse_world_pos.y, ToAllegro(Colors::White), 0);
		if (raycast_result.Hit)
		{
			al_draw_line(player_center.x, player_center.y, raycast_result.HitPosition.x, raycast_result.HitPosition.y, ToAllegro(Colors::Red), 0);
		}

		al_flip_display();
	}

	al_destroy_event_queue(queue);
	al_destroy_display(display);

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
