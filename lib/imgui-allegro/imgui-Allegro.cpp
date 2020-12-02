// dear imgui: standalone example application for Allegro 5
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <stdint.h>
#include <allegro5/allegro.h>
#include <imgui.h>
#include "imgui-Allegro.h"
#include "imgui_impl_allegro5.h"

void ImGui::Allegro::Init(ALLEGRO_DISPLAY* display)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplAllegro5_Init(display);

	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();
	ImGui::Render();
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
}

void ImGui::Allegro::ProcessEvent(ALLEGRO_EVENT* event)
{
	ImGui_ImplAllegro5_ProcessEvent(event);
	if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
	{
		ImGui_ImplAllegro5_InvalidateDeviceObjects();
		al_acknowledge_resize(event->display.source);
		ImGui_ImplAllegro5_CreateDeviceObjects();
	}
}

void ImGui::Allegro::NewFrame(ALLEGRO_DISPLAY* display, double dt)
{
	// Start the Dear ImGui frame
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();
}

void ImGui::Allegro::Render(ALLEGRO_DISPLAY* display)
{
	ImGui::Render();
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
}

void ImGui::Allegro::Shutdown()
{
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
}
