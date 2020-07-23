#include "game.h"
#include <fstream>

ALLEGRO_USTR_INFO ToAllegro(std::string_view str)
{
	ALLEGRO_USTR_INFO result{};
	al_ref_buffer(&result, str.data(), str.size());
	return result;
}

void Game::Init()
{
	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	al_install_keyboard();
	al_install_mouse();

	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);

	mDisplay = al_create_display(1280, 720);
	mQueue = al_create_event_queue();

	ImGui::Allegro::Init(mDisplay);

	mScreenRect = rec2::from_size(0, 0, al_get_display_width(mDisplay), al_get_display_height(mDisplay));

	al_register_event_source(mQueue, al_get_keyboard_event_source());
	al_register_event_source(mQueue, al_get_mouse_event_source());
	al_register_event_source(mQueue, al_get_display_event_source(mDisplay));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	al_add_new_bitmap_flag(ALLEGRO_MIPMAP);
	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_MAG_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	mInput.Init();
	mInput.MapKeyAndButton("up", KeyboardKey::W, XboxGamepadButton::Up);
	mInput.MapKeyAndButton("down", KeyboardKey::S, XboxGamepadButton::Down);
	mInput.MapKeyAndButton("left", KeyboardKey::A, XboxGamepadButton::Left);
	mInput.MapKeyAndButton("right", KeyboardKey::D, XboxGamepadButton::Right);

	al_identity_transform(&mUICamera);
	mCamera.SetFromDisplay(mDisplay);
}

void Game::Load()
{
	mFont = al_load_ttf_font("data/fonts/plantin_regular.ttf", 24, ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	mBigFont = al_load_ttf_font("data/fonts/heroquest.ttf", 60, ALLEGRO_NO_PREMULTIPLIED_ALPHA);
}

void Game::Start()
{
}

void Game::Loop()
{
	while (!mQuit)
	{
		NewFrame();
		Events();
		Update();
		Debug();
		Render();
	}
}

void Game::NewFrame()
{
	mInput.Update();
	mTiming.Update();

	mDT = mTiming.TimeSinceLastFrame();

	ImGui::Allegro::NewFrame(mDisplay, mDT);

	mDebugger.Value("FPS", 1.0 / mDT);
}

void Game::Events()
{
	ALLEGRO_EVENT event{};
	while (al_get_next_event(mQueue, &event))
	{
		switch (event.type)
		{
			/// ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY, ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			mQuit = true;
			break;
		}

		if (!ImGui::IsAnyWindowHovered())
			mInput.ProcessEvent(event);
		ImGui::Allegro::ProcessEvent(&event);
	}
}

void Game::Update()
{
	/// Update animators
	for (auto& anim : mAnimators)
		if (anim(mDT)) anim = {};
	std::erase_if(mAnimators, [](std::function<bool(seconds_t)>& anim) { return !anim; });
}

void Game::Debug()
{
	if (ImGui::BeginTabBar("tabs"))
	{
		if (ImGui::BeginTabItem("Input"))
		{
			mInput.Debug(mDebugger);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void Game::Render()
{
	al_clear_to_color(ToAllegro(Colors::Black));

	al_use_transform(&mCamera.GetTransform());


	///

	ImGui::Allegro::Render(mDisplay);

	al_flip_display();
}

void Game::Shutdown()
{
	ImGui::Allegro::Shutdown();

	al_destroy_event_queue(mQueue);
	al_destroy_display(mDisplay);

	al_uninstall_mouse();
	al_uninstall_keyboard();
	al_uninstall_system();
}
