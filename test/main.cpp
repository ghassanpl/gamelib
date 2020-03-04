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
#include <Navigation/Squares.h>
#include <Navigation/Grid.h>
#include <Navigation/Navigation.h>
#include <Navigation/Maze.h>
#include <Input/AllegroInput.h>
#include <Utils/PanZoomer.h>
//#include <Resources/Map.h>

using namespace gamelib;
using namespace gamelib::squares;

struct RoomTile
{
	ALLEGRO_BITMAP* Bg = nullptr;
	int RotationFlags = 0;
};

struct Map
{
	Grid<RoomTile> RoomGrid;
	NavigationGrid NavGrid;

	void BuildRoom(irec2 const& room, ALLEGRO_BITMAP* bg)
	{
		NavGrid.BreakAdjacency(room);
		RoomGrid.ForEachInRect(room, [this, bg](ivec2 pos) {
			auto tile = RoomGrid.At(pos);
			tile->Bg = bg;
		});
	}

	Map()
	{
		RoomGrid.Reset(26, 19);
		NavGrid.Reset(26, 19);
		NavGrid.BuildAdjacency<ghassanpl::flag_bits(NavigationGrid::IterationFlags::OnlyValid)>();
	}
};

struct TileObject
{
	ivec2 Position{};
	uvec2 Size{ 1,1 };
	Direction WallPosition = Direction::None;
	ALLEGRO_BITMAP* Texture = nullptr;
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

	/// al_load_bitmap
	/// al_set_blender
	/*
	AL_FUNC(void, al_set_org_name, (const char *org_name));
	AL_FUNC(void, al_set_app_name, (const char *app_name));
	*/

	IErrorReporter reporter;

	ICamera camera{display};
	//camera.SetRotation(45.0);

	AllegroInput input{ reporter };
	input.Init();
	PanZoomer pz{ input, camera };

	std::mt19937_64 rng;

	ALLEGRO_BITMAP* tiles[7];
	al_add_new_bitmap_flag(ALLEGRO_MIPMAP);
	tiles[0] = al_load_bitmap("data/tile.png");
	tiles[1] = al_load_bitmap("data/tile2.png");
	tiles[2] = al_load_bitmap("data/tile3.png");
	tiles[3] = al_load_bitmap("data/tile4.png");
	tiles[4] = al_load_bitmap("data/green.png");
	tiles[5] = al_load_bitmap("data/beige.png");
	tiles[6] = al_load_bitmap("data/yellow.png");

	Map map;
	map.RoomGrid.ForEach([&](ivec2 pos) {
		auto tile = map.RoomGrid.At(pos);
		tile->Bg = tiles[random::IntegerRange(rng, 0, 3)];
		tile->RotationFlags = random::IntegerRange(rng, 0, 15);
	});
	map.BuildRoom(irec2::from_size(1, 1, 4, 3), tiles[4]);
	map.BuildRoom(irec2::from_size(5, 1, 4, 3), tiles[5]);
	map.BuildRoom(irec2::from_size(1, 4, 4, 5), tiles[6]);

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

		double t = al_get_time();
		double dt = 1. / 60.;

		/// //////////////////////////// ///
		/// TODO: Update
		/// //////////////////////////// ///

		pz.Update(dt);

		/// //////////////////////////// ///
		/// Draw
		/// //////////////////////////// ///

		al_clear_to_color(ToAllegro(Colors::Black));

		al_use_transform(&camera.GetTransform());

		auto mouse = camera.ScreenSpaceToWorldSpace(input.GetMousePosition());
		/// al_hold_bitmap_drawing
		/// al_draw_tinted_scaled_rotated_bitmap_region
		constexpr float tile_size = 128;
		constexpr float half_tile_size = tile_size / 2;
		for (size_t y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_size;
			for (size_t x = 0; x < map.RoomGrid.Width(); x++)
			{
				auto xp = x * tile_size;
				auto tile = map.RoomGrid.At(x, y);
				al_draw_rotated_bitmap(tile->Bg, half_tile_size, half_tile_size, xp + half_tile_size, yp + half_tile_size, glm::radians((tile->RotationFlags>>2)*90.0f), tile->RotationFlags&3);
			}
		}

		constexpr auto wall_width = 10;
		constexpr auto shadow_size = 20;
		for (size_t y = 0; y < map.RoomGrid.Height(); y++)
		{
			auto yp = y * tile_size;
			for (size_t x = 0; x < map.RoomGrid.Width(); x++)
			{
				auto xp = x * tile_size;
				auto adj = map.NavGrid.At(x, y)->Adjacency;
				if (!adj.is_set(Direction::Left))
				{
					float shadow[] = {
						xp, yp + tile_size,
						xp, yp,
						xp - shadow_size, (yp + shadow_size),
						xp - shadow_size, yp + tile_size + shadow_size,
					};
					al_draw_filled_polygon(shadow, 4, ToAllegro(Colors::Black));
					al_draw_line(xp, yp - wall_width / 2, xp, yp + tile_size + wall_width / 2, ToAllegro(Colors::White), wall_width);
				}
				if (!adj.is_set(Direction::Up)) al_draw_line(xp - wall_width / 2, yp, xp + tile_size + wall_width / 2, yp, ToAllegro(Colors::White), wall_width);
				if (!adj.is_set(Direction::Right)) al_draw_line(xp + tile_size, yp, xp + tile_size, yp + tile_size, ToAllegro(Colors::White), wall_width);
				if (!adj.is_set(Direction::Down))
				{
					float shadow[] = {
						xp, yp + tile_size,
						xp - shadow_size, yp + tile_size + shadow_size,
						(xp + tile_size) - shadow_size, yp + tile_size + shadow_size,
						xp + tile_size, yp + tile_size,
					};
					al_draw_filled_polygon(shadow, 4, ToAllegro(Colors::Black));
					al_draw_line(xp - wall_width / 2, yp + tile_size, xp + tile_size + wall_width / 2, yp + tile_size, ToAllegro(Colors::White), wall_width);
				}
			}
		}

		al_flip_display();
	}

	al_destroy_event_queue(queue);
	al_destroy_display(display);

	al_uninstall_mouse();
	al_uninstall_keyboard();
	al_uninstall_system();
}