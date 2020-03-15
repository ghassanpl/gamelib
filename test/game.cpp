#include "game.h"

ALLEGRO_USTR_INFO ToAllegro(std::string_view str)
{
	ALLEGRO_USTR_INFO result{};
	al_ref_buffer(&result, str.data(), str.size());
	return result;
}

void Map::BuildRoom(irec2 const& room, ALLEGRO_BITMAP* bg)
{
	NavGrid.SetBlocking(room, { WallBlocks::Passage, WallBlocks::Sight }, true);
	RoomGrid.ForEachInRect(room, [this, bg](ivec2 pos) {
		auto tile = RoomGrid.At(pos);
		tile->Bg = bg;
		});
}

void Map::DetermineVisibility(vec2 from_position)
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

Map::Map()
{
	RoomGrid.Reset(26, 19);
	NavGrid.Reset(26, 19);
	NavGrid.BuildWalls<ghassanpl::flag_bits(BlockNavigationGrid::IterationFlags::OnlyValid)>([&](ivec2 from, ivec2 to) {
		return NavGrid.IsValid(to) ? enum_flags<WallBlocks>{} : enum_flags<WallBlocks>::all(WallBlocks::Sight);
		});
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

	mScreenSize = ivec2{ al_get_display_width(mDisplay), al_get_display_height(mDisplay) };

	al_register_event_source(mQueue, al_get_keyboard_event_source());
	al_register_event_source(mQueue, al_get_mouse_event_source());
	al_register_event_source(mQueue, al_get_display_event_source(mDisplay));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	al_add_new_bitmap_flag(ALLEGRO_MIPMAP);
	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_MAG_LINEAR);
	al_add_new_bitmap_flag(ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	//al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

	mInput.Init();
	mInput.MapKeyAndButton('up', KeyboardKey::W, XboxGamepadButton::Up);
	mInput.MapKeyAndButton('down', KeyboardKey::S, XboxGamepadButton::Down);
	mInput.MapKeyAndButton('left', KeyboardKey::A, XboxGamepadButton::Left);
	mInput.MapKeyAndButton('righ', KeyboardKey::D, XboxGamepadButton::Right);
	mInput.MapMouse(MouseButton::Left, 'act');
	mInput.MapMouse(MouseButton::Right, 'exam');
	mInput.MapMouse(MouseButton::Middle, 'addi');

	al_identity_transform(&mUICamera);
	mCamera.SetFromDisplay(mDisplay);

}

void Game::Load()
{
	mFont = al_load_ttf_font("data/fonts/plantin_regular.ttf", 36, ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	mFontHeight = al_get_font_line_height(mFont);

	mTileDescription.SetBounds(rec2::from_size(16, 16, 300, 200));
	mTileDescription.SetDefaultStyle({ .Font = mFont });

	tiles[0] = al_load_bitmap("data/tile.png");
	tiles[1] = al_load_bitmap("data/tile2.png");
	tiles[2] = al_load_bitmap("data/tile3.png");
	tiles[3] = al_load_bitmap("data/tile4.png");
	tiles[4] = al_load_bitmap("data/green.png");
	tiles[5] = al_load_bitmap("data/beige.png");
	tiles[6] = al_load_bitmap("data/yellow.png");

	input_gfx['up'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_W.png");
	input_gfx['down'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_S.png");
	input_gfx['left'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_A.png");
	input_gfx['righ'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_D.png");
	input_gfx['act'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Left.png");
	input_gfx['exam'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Right.png");
	input_gfx['addi'] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Middle.png");

	mTileDescription.SetImageResolver([](std::string_view name) {

	});

	characters[0] = al_load_bitmap("data/barbarian_shadow.png");

	mCurrentMap.RoomGrid.ForEach([&](ivec2 pos) {
		auto tile = mCurrentMap.RoomGrid.At(pos);
		tile->Bg = tiles[random::IntegerRange(RNG, 0, 3)];
		tile->RotationFlags = random::IntegerRange(RNG, 0, 15);
		});
	mCurrentMap.BuildRoom(irec2::from_size(1, 1, 4, 3), tiles[4]);
	mCurrentMap.BuildRoom(irec2::from_size(5, 1, 4, 3), tiles[5]);
	mCurrentMap.BuildRoom(irec2::from_size(1, 4, 4, 5), tiles[6]);
	mCurrentMap.NavGrid.SetBlocksPassage({ 2,0 }, { 2,1 }, false);

	mPlayer = mCurrentMap.SpawnObject<TileObject>({ 0,0 });
	mPlayer->Texture = characters[0];
}

void Game::Start()
{
	mTiming.Reset();
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

	mCommands.clear();

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
	mPanZoomer.Update(mDT);

	Direction move_dir = Direction::None;
	if (mInput.WasButtonPressed('up'))
		MovePlayer(Direction::Up);
	else if (mInput.WasButtonPressed('left'))
		MovePlayer(Direction::Left);
	else if (mInput.WasButtonPressed('righ'))
		MovePlayer(Direction::Right);
	else if (mInput.WasButtonPressed('down'))
		MovePlayer(Direction::Down);

	auto mouse_tile_pos = GetMouseTilePosition();
	mTileDescription.Clear();
	if (mCurrentMap->IsValid(mouse_tile_pos) && IsNeighbor(mouse_tile_pos, mPlayer->Position()) && !mCurrentMap.NavGrid.BlocksPassage(mouse_tile_pos, mPlayer->Position()))
	{
		AddCommand('act', "Move", [&] {
			MovePlayer(ToDirection(mouse_tile_pos - mPlayer->Position()));
		});
	}

	for (auto& cmd : mCommands)
	{
		if (mInput.WasButtonPressed(cmd.Input))
			cmd.Func();
	}

}

void Game::Debug()
{
	mDebugger.Value("Mouse World Pos", GetMouseWorldPosition());
	mDebugger.Value("Mouse Tile Pos", GetMouseTilePosition());

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

	/// Floors
	constexpr float half_tile_size = tile_width / 2;
	for (int y = 0; y < mCurrentMap.RoomGrid.Height(); y++)
	{
		auto yp = y * tile_width;
		for (int x = 0; x < mCurrentMap.RoomGrid.Width(); x++)
		{
			if (!mCurrentMap.NavGrid.WasSeen({ x, y })) continue;

			auto xp = x * tile_width;
			auto tile = mCurrentMap.RoomGrid.At(x, y);
			al_draw_rotated_bitmap(tile->Bg, half_tile_size, half_tile_size, xp + half_tile_size, yp + half_tile_size, glm::radians((tile->RotationFlags >> 2) * 90.0f), tile->RotationFlags & 3);
		}
	}

	/// Objects
	for (int y = 0; y < mCurrentMap.RoomGrid.Height(); y++)
	{
		auto yp = y * tile_width;
		for (int x = 0; x < mCurrentMap.RoomGrid.Width(); x++)
		{
			if (!mCurrentMap.NavGrid.WasSeen({ x, y })) continue;

			auto xp = x * tile_width;
			auto tile = mCurrentMap.RoomGrid.At(x, y);

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
	for (int y = 0; y < mCurrentMap.RoomGrid.Height(); y++)
	{
		auto yp = y * tile_width;
		for (int x = 0; x < mCurrentMap.RoomGrid.Width(); x++)
		{
			if (!mCurrentMap.NavGrid.WasSeen({ x, y })) continue;

			auto xp = x * tile_width;
			auto adj = mCurrentMap.NavGrid.At(x, y)->BlocksWall(WallBlocks::Passage);
			if (adj.is_set(Direction::Left)) al_draw_line(xp, yp - wall_width / 2, xp, yp + tile_width + wall_width / 2, wall_color, wall_width);
			if (adj.is_set(Direction::Up)) al_draw_line(xp - wall_width / 2, yp, xp + tile_width + wall_width / 2, yp, wall_color, wall_width);
			if (adj.is_set(Direction::Right)) al_draw_line(xp + tile_width, yp, xp + tile_width, yp + tile_width, wall_color, wall_width);
			if (adj.is_set(Direction::Down)) al_draw_line(xp - wall_width / 2, yp + tile_width, xp + tile_width + wall_width / 2, yp + tile_width, wall_color, wall_width);

		}
	}

	static constexpr auto half_black = Colors::GetBlack(0.5f);
	for (int y = 0; y < mCurrentMap.RoomGrid.Height(); y++)
	{
		auto yp = y * tile_width;
		for (int x = 0; x < mCurrentMap.RoomGrid.Width(); x++)
		{
			auto xp = x * tile_width;
			if (!mCurrentMap.NavGrid.Visible({ x, y }))
			{
				al_draw_filled_rectangle(xp, yp, xp + tile_width, yp + tile_width, ToAllegro(half_black));
			}
		}
	}

	/// Mouse selection
	if (auto tile = GetMouseTile())
	{
		auto select = mCurrentMap.RoomGrid.RectForTile(GetMouseTilePosition(), tile_size);
		//al_draw_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, ToAllegro(Colors::Red), 6);
		al_draw_filled_rounded_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, 4.0f, 4.0f, ToAllegro(Colors::GetWhite(0.5f)));
		al_draw_rounded_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, 4.0f, 4.0f, ToAllegro(Colors::Magenta), 4.0f);
	}

	al_use_transform(&mUICamera);

	mTileDescription.Draw();
	/*
	rec2 selected_tile_desc_rect = rec2::from_size(16, 16, 300, 200);
	float y = 0;
	al_draw_filled_rounded_rectangle(selected_tile_desc_rect.p1.x, selected_tile_desc_rect.p1.y, selected_tile_desc_rect.p2.x, selected_tile_desc_rect.p2.y, 4.0f, 4.0f, ToAllegro(half_black));
	for (auto& cmd : mCommands)
	{
		auto glyph = input_gfx[cmd.Input];
		if (glyph) al_draw_scaled_bitmap(glyph, 0, 0, 100, 100, selected_tile_desc_rect.p1.x, selected_tile_desc_rect.p1.y, mFontHeight, mFontHeight, 0);
		DrawText(mFont, { selected_tile_desc_rect.p1.x + mFontHeight, selected_tile_desc_rect.p1.x + y }, Colors::White, Colors::Transparent, HorizontalAlign::Left, "{}", cmd.Text);
		y += mFontHeight + 4;
	}
	*/

	ImGui::Allegro::Render(mDisplay);

	al_flip_display();
}

void Game::UpdateCamera()
{
	auto player_world_pos = mCurrentMap.RoomGrid.TilePositionToWorldPosition(mPlayer->Position(), tile_size) + tile_size / 2.0f;
	mCamera.SetWorldCenter(player_world_pos);
	mCurrentMap.DetermineVisibility(player_world_pos);
}

void Game::MovePlayer(Direction move_dir)
{
	const auto target_pos = mPlayer->Position() + ToVector(move_dir);
	if (!mCurrentMap.NavGrid.BlocksPassage(mPlayer->Position(), target_pos))
	{
		mPlayer->MoveTo(target_pos);

		UpdateCamera();
	}
}

void Game::AddCommand(InputID input, std::string_view text, std::function<void()> func)
{
	mCommands.push_back(Command{
		.Text = (std::string)text,
		.Input = input,
		.Func = std::move(func)
	});

	mTileDescription.AddParagraph(fmt::format("<img={}> {}", (int)input, text));
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

void TileObject::MoveTo(ivec2 pos)
{
	if (!mParentMap->RoomGrid.IsValid(pos)) return;

	mParentMap->RoomGrid.At(mPosition)->Objects.erase(this);
	mPosition = pos;
	mParentMap->RoomGrid.At(mPosition)->Objects.insert(this);
}