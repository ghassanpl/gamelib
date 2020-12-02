#include "game.h"
#include <fstream>

#include "Geometry/Collision.h"
#include "Utils/MemberSpan.h"

static Game* game = nullptr;

ALLEGRO_USTR_INFO ToAllegro(std::string_view str)
{
	ALLEGRO_USTR_INFO result{};
	al_ref_buffer(&result, str.data(), str.size());
	return result;
}

void DrawTile(ALLEGRO_BITMAP* img, vec2 src, ivec2 pos)
{
	al_draw_bitmap_region(img, src.x, src.y, TILE_SIZE, TILE_SIZE, pos.x * TILE_SIZE, pos.y * TILE_SIZE, 0);
}

void DrawTile(ALLEGRO_BITMAP* img, vec2 src, vec2 pos)
{
	al_draw_bitmap_region(img, src.x, src.y, TILE_SIZE, TILE_SIZE, pos.x, pos.y, 0);
}

void ghassanpl::ReportAssumptionFailure(ghassanpl::detail::source_location where, std::string_view expectation, std::initializer_list<std::pair<std::string_view, std::string>> values, std::string data)
{
	using namespace gamelib;
	Reporter reporter{ game->ErrorReporter(), ReportType::AssumptionFailure };
	reporter.MessageLine("I assumed that {}", expectation);
	for (auto& p : values)
		reporter.Value(p.first, p.second);
	reporter.AdditionalInfo("File", "{}", where.file_name);
	reporter.AdditionalInfo("Function", "{}", where.function_name);
	reporter.AdditionalInfo("Line", "{}", where.line);
	reporter.AdditionalInfo("Column", "{}", where.column);
	reporter.AdditionalInfo("Additional Assumption Info", "{}", data);
	reporter.Perform();
}

void Game::Init()
{
	game = this;

	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	al_install_keyboard();
	al_install_mouse();
	al_install_joystick();

	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);

	mDisplay = al_create_display(1280, 720);
	mQueue = al_create_event_queue();

	ImGui::Allegro::Init(mDisplay);

	mScreenRect = rec2::from_size(0, 0, al_get_display_width(mDisplay), al_get_display_height(mDisplay));

	al_register_event_source(mQueue, al_get_keyboard_event_source());
	al_register_event_source(mQueue, al_get_mouse_event_source());
	al_register_event_source(mQueue, al_get_joystick_event_source());
	al_register_event_source(mQueue, al_get_display_event_source(mDisplay));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	//al_add_new_bitmap_flag(ALLEGRO_MIPMAP);
	//al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR);
	//al_add_new_bitmap_flag(ALLEGRO_MAG_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	mReporter = std::make_shared<IErrorReporter>();
	mDebugger = std::make_shared<AllegroImGuiDebugger>();
	mInput = std::make_shared<AllegroInput>(mReporter, mDebugger);

	mInput->Init();
	mInput->MapKeyAndButton("up", KeyboardKey::W, XboxGamepadButton::Up);
	mInput->MapKeyAndButton("down", KeyboardKey::S, XboxGamepadButton::Down);
	mInput->MapKeyAndButton("left", KeyboardKey::A, XboxGamepadButton::Left);
	mInput->MapKeyAndButton("right", KeyboardKey::D, XboxGamepadButton::Right);
	mInput->MapKeyAndButton("jump", KeyboardKey::Space, XboxGamepadButton::A);

	al_identity_transform(&mUICamera);
	mCamera.SetFrom(mDisplay);

	al_register_assert_handler([](const char* expr, const char* file, int line, const char* func) {
		game->ErrorReporter().Error("Allegro assert failed: {}\nFile: {}\nLine: {}\nFunc: {}\nErrno: {}", expr, file, line, func, al_get_errno());
	});

	mTiming.SetMaxTimeSinceLastFrame(1.0/60.0);
	mTiming.SetRecordLastFrames(60);
}

void Game::Load()
{
	mFont = al_load_ttf_font("data/fonts/plantin_regular.ttf", 24, ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	mBigFont = al_load_ttf_font("data/fonts/heroquest.ttf", 60, ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	LevelTiles.Load("data/gfx/tiles.png", { 32, 2 });
	PlayerImages.Load("data/gfx/character.png", { 16, 2 });
	
	CurrentLevel.Tiles.Reset(16, 16, Tile{ .BG = 17 });
	CurrentLevel.Tiles.ForEachInPerimeter(CurrentLevel.Tiles.Perimeter(), [this](ivec2 pos) {
		CurrentLevel.Tiles.At(pos)->Wall = 16;
		CurrentLevel.Tiles.At(pos)->Type = TileType::Block;
	});
	CurrentLevel.Tiles.ForEachInRect(CurrentLevel.Tiles.Perimeter(), [this](ivec2 pos) {
		if ((rand() % 6) == 0)
		{
			CurrentLevel.Tiles.At(pos)->Wall = 16;
			CurrentLevel.Tiles.At(pos)->Type = TileType::Block;
		}
	});

	PlayerAnimations.AddAnimation("walking", 0.1, PlayerImages, 0, 16);

	LevelObjects.push_back(std::make_unique<Player>());
	Gostek = static_cast<Player*>(LevelObjects.front().get());
	Gostek->AnimSource = &PlayerAnimations;
	Gostek->PlayAnim("walking");
	Gostek->Position = start_pos * TILE_SIZE;
	Gostek->SpriteOffset = vec2{ -4,-8 };

	mCamera.SetWorldSize(mCamera.WorldSize() / zoom);
}

void Game::Start()
{
	mTiming.Reset();
	mTiming.Update();
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
	mInput->Update();
	mTiming.Update();

	mDT = mTiming.FixedFrameTime();

	ImGui::Allegro::NewFrame(mDisplay, mDT);

	mDebugger->Value("FPS", 1.0 / mDT);
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
			mInput->ProcessEvent(event);
		ImGui::Allegro::ProcessEvent(&event);
	}
}

irec2 RectForTileType(TileType type)
{
	return { 0, TILE_SIZE/2, TILE_SIZE, TILE_SIZE };
}

void Game::Update()
{
	const auto dt = mDT;
	const auto fdt = (float)dt;

	/// Update animators
	for (auto& anim : mAnimators)
		if (anim(dt)) anim = {};
	std::erase_if(mAnimators, [](std::function<bool(seconds_t)>& anim) { return !anim; });

	for (auto& obj : LevelObjects)
	{
		if (obj->Alive)
			obj->Update(dt);
	}

	if (mInput->WasButtonPressed("jump"))
	{
		if (mCanJump)
		{
			mJumping = true;
			mCanJump = false;
			Gostek->Velocity.y = jump_velocity;
		}
	}
	
	if (mJumping && Gostek->Velocity.y >= 0)
	{
		mJumping = false;
	}

	if (mInput->WasButtonReleased("jump"))
	{
		if (mJumping)
		{
			//Gostek->Velocity.y = -Gostek->Velocity.y;
			Gostek->Velocity.y = 0;
		}
		mJumping = false;
	}

	if (mInput->IsButtonPressed("left"))
	{
		Gostek->Velocity.x = -move_accel;
	}
	else if (mInput->IsButtonPressed("right"))
	{
		Gostek->Velocity.x = move_accel;
	}
	
	Gostek->Velocity.x -= (move_decel * Gostek->Velocity.x) * fdt;

	if (mInput->WasKeyPressed(KeyboardKey::R))
	{
		Gostek->Position = start_pos * TILE_SIZE;
	}

	CurrentLevel.Tiles.ForEach([this](auto pos) { CurrentLevel.Tiles.At(pos)->Mem = 0; });

	/// Do collisions
	for (auto& obj : LevelObjects)
	{
		if (auto dynamic = dynamic_cast<Mob*>(obj.get()))
		{
			dynamic->PrevVelocity = dynamic->Velocity;
			dynamic->Velocity.y += gravity * fdt;
			
			const auto tile_size = vec2{ TILE_SIZE };
			auto pos = dynamic->Position;
			const auto size = dynamic->Size;
			auto vel = dynamic->Velocity;

			auto check_y = [&](int& out_yo, float offs) {
				const auto yo = int(pos.y + offs + std::clamp(vel.y * fdt, -float(TILE_SIZE / 2), float(TILE_SIZE / 2))) / 16;
				out_yo = yo;
				auto bottom = ivec2{ int(pos.x + size.x / 2) / 16, yo };
				if (auto tile = CurrentLevel.Tiles.At(bottom); tile && tile->Type != TileType::Air) return tile;
				bottom = ivec2{ int(pos.x) / 16, yo };
				if (auto tile = CurrentLevel.Tiles.At(bottom); tile && tile->Type != TileType::Air) return tile;
				bottom = ivec2{ int(pos.x + (size.x - 1)) / 16, yo };
				if (auto tile = CurrentLevel.Tiles.At(bottom); tile && tile->Type != TileType::Air) return tile;
				return (Tile*)nullptr;
			};

			if (vel.y > 0)
			{
				int yo = 0;
				if (auto tile = check_y(yo, size.y))
				{
					pos.y = yo * 16 - size.y;
					vel.y = 0;
					mJumping = false;
					mCanJump = true;
				}
			}
			else if (vel.y < 0)
			{
				int yo = 0;
				if (auto tile = check_y(yo, 0))
				{
					pos.y = (yo + 1) * 16;
					vel.y = 0;
					mJumping = false;
				}
			}
			dynamic->Velocity.y = vel.y;
			pos.y += vel.y * fdt;
			dynamic->Position.y = pos.y;

			auto check_x = [&](int& out_xo, float offs) {
				const auto xo = int(pos.x + offs + std::clamp(vel.x * fdt, -float(TILE_SIZE / 2), float(TILE_SIZE / 2))) / 16;
				out_xo = xo;
				auto right = ivec2{ xo , int(pos.y + size.y / 2) / 16 };
				if (auto tile = CurrentLevel.Tiles.At(right); tile && tile->Type != TileType::Air) return tile;
				right = ivec2{ xo , int(pos.y) / 16 };
				if (auto tile = CurrentLevel.Tiles.At(right); tile && tile->Type != TileType::Air) return tile;
				right = ivec2{ xo , int(pos.y + (size.y - 1)) / 16 };
				if (auto tile = CurrentLevel.Tiles.At(right); tile && tile->Type != TileType::Air) return tile;
				return (Tile*)nullptr;
			};

			if (vel.x > 0)
			{
				int xo = 0;
				if (auto tile = check_x(xo, size.x))
				{
					pos.x = xo * 16 - size.x;
					vel.x = 0;
				}
			}
			else if (vel.x < 0)
			{
				int xo = 0;
				if (auto tile = check_x(xo, 0))
				{
					pos.x = (xo + 1) * 16;
					vel.x = 0;
				}
			}
			dynamic->Velocity.x = vel.x;
			pos.x += vel.x * fdt;
			dynamic->Position.x = pos.x;

			//ImGui::Text("Position: %gx%g", dynamic->Position.x, dynamic->Position.y);
		}
	}

	mCamera.SetWorldCenter(ivec2(Gostek->Position));
}

void Game::Debug()
{
	if (ImGui::BeginTabBar("tabs"))
	{
		if (ImGui::BeginTabItem("Game"))
		{
			mDebugger->Value("Can Jump", mCanJump);
			mDebugger->Value("Jumping", mJumping);

			mDebugger->Value("Gravity", gravity);
			mDebugger->Value("Zoom", zoom);
			mDebugger->Value("Start Pos", start_pos);
			mDebugger->Value("Jump Vel", jump_velocity);
			mDebugger->Value("Move Accel", move_accel);
			mDebugger->Value("Move Decel", move_decel);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Debug"))
		{
			ImGui::Checkbox("Draw Object Rects", &draw_objects);
			ImGui::Checkbox("Draw Tile Rects", &draw_tiles);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Timing"))
		{
			mTiming.Debug(*mDebugger);
			//ImGui::Checkbox("Draw Object Rects", &draw_objects);
			//ImGui::Checkbox("Draw Tile Rects", &draw_tiles);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Input"))
		{
			mInput->Debug();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void Game::Render()
{
	al_clear_to_color(ToAllegro(Colors::Black));

	al_use_transform(&mCamera.GetTransform());

	al_hold_bitmap_drawing(true);
	for (int y = 0; y < CurrentLevel.Tiles.Height(); ++y)
	{
		for (int x = 0; x < CurrentLevel.Tiles.Width(); ++x)
		{
			auto tile = CurrentLevel.Tiles.At(x, y);
			if (tile->BG)
				LevelTiles.DrawTile(tile->BG, ivec2{ x, y });
			if (tile->Wall)
				LevelTiles.DrawTile(tile->Wall, ivec2{ x, y });
		}
	}
	al_hold_bitmap_drawing(false);

	if (draw_tiles)
	{
		for (int y = 0; y < CurrentLevel.Tiles.Height(); ++y)
		{
			for (int x = 0; x < CurrentLevel.Tiles.Width(); ++x)
			{
				auto tile = CurrentLevel.Tiles.At(x, y);
				if (tile->Type != TileType::Air)
				{
					auto r = RectForTileType(tile->Type) + ivec2{ x,y }*TILE_SIZE;
					al_draw_rectangle(r.p1.x, r.p1.y, r.p2.x, r.p2.y, { float(tile->Mem),0,1,1 }, 0);
				}
			}
		}
	}

	for (auto& obj : LevelObjects)
	{
		auto& frame = obj->Frame(); 
		DrawTile(frame.Image.get(), frame.Pos, glm::floor(obj->Position + frame.Offset + obj->SpriteOffset));

		if (draw_objects)
			al_draw_rectangle(floor(obj->Position.x), floor(obj->Position.y), floor(obj->Position.x + obj->Size.x), floor(obj->Position.y + obj->Size.y), { 1,0,0,1 }, 0);
	}

	ImGui::Allegro::Render(mDisplay);

	al_flip_display();
}

void Game::DrawText(ALLEGRO_FONT* font, vec2 position, Color const& color, Color const& background_color, Align align, std::string_view str)
{
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

void Game::Shutdown()
{
	mBitmaps.clear();
	LevelTiles = {};
	PlayerImages = {};
	CurrentLevel = {};
	PlayerAnimations = {};

	ImGui::Allegro::Shutdown();

	al_destroy_event_queue(mQueue);
	al_destroy_display(mDisplay);

	al_uninstall_mouse();
	al_uninstall_keyboard();
	al_uninstall_system();
}

Bitmap Game::LoadBitmap(std::filesystem::path path)
{
	path = std::filesystem::canonical(path);
	if (auto it = mBitmaps.find(path); it != mBitmaps.end())
		return it->second;
	auto bmp = Bitmap{ al_load_bitmap(path.string().c_str()), &al_destroy_bitmap };
	AssumingNotNull(bmp.get());
	return mBitmaps[path] = std::move(bmp);
}

void Tileset::Load(std::filesystem::path path, ivec2 frame_counts)
{
	mImage = game->LoadBitmap(path);
	AssumingNotNull(mImage.get());
	mFrameCounts = frame_counts;
}

void Tileset::DrawTile(size_t id, ivec2 pos) const
{
	AssumingNotNull(mImage.get());
	const auto src = PosForID(id);
	::DrawTile(mImage.get(), src, pos);
}

void Tileset::DrawTile(size_t id, vec2 pos) const
{
	AssumingNotNull(mImage.get());
	const auto src = PosForID(id);
	::DrawTile(mImage.get(), src, pos);
}

AnimationFrame const& Animation::FrameAtTime(seconds_t time, seconds_t* out_time_in_frame) const
{
	const auto total_time = Frames.size() * FrameTime;
	double frame = 0;
	const auto alpha = std::modf(std::fmod(time, total_time) / FrameTime, &frame);
	if (out_time_in_frame)
		*out_time_in_frame = alpha;
	const auto result = size_t(frame);
	AssumingValidIndex(result, Frames);
	return Frames[result];
}

void Object::Update(seconds_t dt)
{
	Life += dt;
	mAnimTime += dt;
}

AnimationFrame const& Object::Frame() const
{
	AssumingNotNull(mCurrentAnim);
	return mCurrentAnim->FrameAtTime(mAnimTime);
}

void Object::PlayAnim(std::string_view anim)
{
	AssumingNotNull(AnimSource);
	mCurrentAnim = AnimSource->FindAnimation(anim);
	mAnimTime = 0;
}

Animation const* AnimationManager::FindAnimation(std::string_view animation) const
{
	if (auto it = Animations.find(animation); it != Animations.end())
		return &it->second;
	return nullptr;
}

void Mob::Update(seconds_t dt)
{
	Object::Update(dt);
}
