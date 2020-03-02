#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

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
#include <Navigation/Grid.h>
#include <Navigation/Navigation.h>
#include <Navigation/Maze.h>
#include <Input/AllegroInput.h>
#include <Utils/PanZoomer.h>
//#include <Resources/Map.h>

using namespace gamelib;

int main()
{
	auto q = Interpolate(InterpolationFunction::Cosine, double{});
	al_init();
	al_init_primitives_addon();

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

		al_draw_circle(0, 0, 100, ToAllegro(Colors::Red), 0.0f);
		al_draw_circle(1280, 720, 100, ToAllegro(Colors::Red), 0.0f);
		auto mouse = camera.ScreenSpaceToWorldSpace(input.GetMousePosition());
		al_draw_filled_circle(mouse.x, mouse.y, 3, ToAllegro(Colors::White));
		/// al_hold_bitmap_drawing
		/// al_draw_tinted_scaled_rotated_bitmap_region

		al_flip_display();
	}

	al_destroy_event_queue(queue);
	al_destroy_display(display);

	al_uninstall_mouse();
	al_uninstall_keyboard();
	al_uninstall_system();
}