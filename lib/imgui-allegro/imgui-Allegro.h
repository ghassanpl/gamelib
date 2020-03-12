#pragma once

#include <allegro5/allegro.h>

namespace ImGui::Allegro
{
	
	void Init(ALLEGRO_DISPLAY* display);
	void ProcessEvent(ALLEGRO_EVENT* event);
	void NewFrame(ALLEGRO_DISPLAY* display, double dt);
	void Render(ALLEGRO_DISPLAY* display);
	void Shutdown();

}