#include <allegro5/allegro.h>

int main()
{
	al_init();
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

	bool quit = false;
	while (!quit)
	{
		ALLEGRO_EVENT event{};
		while (al_get_next_event(queue, &event))
		{
			switch (event.type)
			{
				/// case ALLEGRO_EVENT_KEY_DOWN:
				/// case ALLEGRO_EVENT_KEY_CHAR:
				/// case ALLEGRO_EVENT_KEY_UP:
				/// case ALLEGRO_EVENT_MOUSE_AXES:
				/// case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
				/// case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
				/// ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY, ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				quit = true;
				break;
			}
		}

		double t = al_get_time();

		/// al_get_mouse_state
		/// al_clear_to_color ?
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