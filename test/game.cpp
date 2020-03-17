#include "game.h"
#include <fstream>

ALLEGRO_USTR_INFO ToAllegro(std::string_view str)
{
	ALLEGRO_USTR_INFO result{};
	al_ref_buffer(&result, str.data(), str.size());
	return result;
}

void Map::BuildRoom(irec2 const& room, ivec2 door_pos, Direction door_dir)
{
	NavGrid.SetBlocking(room, { WallBlocks::Passage, WallBlocks::Sight }, true);
	auto door = SpawnObject<Door>(door_pos);
	door->WallPosition = door_dir;
	if (auto tile = RoomGrid.At(door_pos + ToVector(door_dir)))
		tile->Objects.insert(door);
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
	mInput.MapMouse(MouseButton::Left, "act");
	mInput.MapMouse(MouseButton::Right, "search");
	mInput.MapMouse(MouseButton::Middle, "additional");

	al_identity_transform(&mUICamera);
	mCamera.SetFromDisplay(mDisplay);
}

void Game::Load()
{
	mFont = al_load_ttf_font("data/fonts/plantin_regular.ttf", 24, ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	mFontHeight = al_get_font_line_height(mFont);

	mTileDescription.SetBounds(rec2::from_size(16, 16, mScreenRect.width() -32, 128));
	mTileDescription.SetDefaultStyle({ .Font = mFont });

	auto gfx_path = std::filesystem::path{ "data/gfx" };
	for (auto it = std::filesystem::recursive_directory_iterator{ gfx_path }; it != std::filesystem::recursive_directory_iterator{}; ++it)
	{
		auto path = it->path();
		if (path.extension() == ".png")
		{
			auto bitmap = al_load_bitmap(path.string().c_str());
			path = std::filesystem::relative(path, gfx_path);
			path.replace_extension();
			mImages[path.generic_string()] = bitmap;
		}
	}

	input_gfx["up"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_W.png");
	input_gfx["down"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_S.png");
	input_gfx["left"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_A.png");
	input_gfx["right"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_D.png");
	input_gfx["act"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Left.png");
	input_gfx["search"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Right.png");
	input_gfx["additional"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_Mouse_Middle.png");

	mTileDescription.SetImageResolver([&](std::string_view name) {
		if (auto it = input_gfx.find(name); it != input_gfx.end())
			return it->second;
		return (ALLEGRO_BITMAP*)nullptr;
	});

	LoadClasses("data/stats/heroes.csv", mHeroClasses);
	LoadClasses("data/stats/monsters.csv", mMonsterClasses);

	mCurrentMap.RoomGrid.ForEach([&](ivec2 pos) {
		auto tile = mCurrentMap.RoomGrid.At(pos);
		tile->Bg = mImages[fmt::format("floors/floor{}", random::IntegerRange(RNG, 0, 9))];
		tile->RotationFlags = random::IntegerRange(RNG, 0, 15);
	});
	mCurrentMap.BuildRoom(irec2::from_size(1, 1, 4, 3), { 2,1 }, Direction::Up);
	//mCurrentMap.BuildRoom(irec2::from_size(5, 1, 4, 3));
	//mCurrentMap.BuildRoom(irec2::from_size(1, 4, 4, 5));
	//mCurrentMap.NavGrid.SetBlocksPassage({ 2,0 }, { 2,1 }, false);

	mCurrentMap.SpawnObject<Stairs>({ 1,1 });

	mPlayer = mCurrentMap.SpawnObject<Hero>({ 1,1 }, mHeroClasses["Warrior"]);

	mCurrentMap.SpawnObject<Furniture>({ 3,3 });
	mCurrentMap.SpawnObject<Monster>({ 4,3 }, mMonsterClasses["Goblin Guard"]);
	mCurrentMap.SpawnObject<Trigger>({ 3,4 });
	mCurrentMap.SpawnObject<Trap>({ 4,4 });
	mCurrentMap.SpawnObject<Item>({ 5,4 });

	for (auto& obj : mCurrentMap.Objects)
	{
		obj->Texture = mImages[obj->Image()];
	}
}

void Game::Start()
{
	UpdateCamera();
	mCamera.SetWorldCenter(mCameraTarget);
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
	if (mPanZoomer.Update(mDT))
		mCameraFocus = false;

	if (mCameraFocus)
	{
		auto wc = mCamera.GetWorldCenter();
		wc += ((mCameraTarget - wc) * (float)mDT) * mCameraSpeed;
		mCamera.SetWorldCenter(wc);
	}

	Direction move_dir = Direction::None;
	if (mInput.WasButtonPressed("up"))
		MovePlayer(Direction::Up);
	else if (mInput.WasButtonPressed("left"))
		MovePlayer(Direction::Left);
	else if (mInput.WasButtonPressed("right"))
		MovePlayer(Direction::Right);
	else if (mInput.WasButtonPressed("down"))
		MovePlayer(Direction::Down);

	auto mouse_tile_pos = GetMouseTilePosition();
	mTileDescription.Clear();
	if (mCurrentMap->IsValid(mouse_tile_pos))
	{
		std::string name_at = "Unknown";
		if (mCurrentMap.NavGrid.WasSeen(mouse_tile_pos))
		{
			name_at = "Floor";

			if (mCurrentMap.NavGrid.Visible(mouse_tile_pos))
			{
				for (auto& object : mCurrentMap.RoomGrid.At(mouse_tile_pos)->Objects)
				{
					if (object->Visible())
						name_at = object->Name();
				}
			}
		}
		
		mTileDescription.AddParagraph(name_at);

		if (IsNeighbor(mouse_tile_pos, mPlayer->Position()) && !mCurrentMap.NavGrid.BlocksPassage(mouse_tile_pos, mPlayer->Position()))
		{
			if (CanMoveTo(mouse_tile_pos))
			{
				AddCommand("act", "Move", [&] {
					MovePlayer(ToDirection(mouse_tile_pos - mPlayer->Position()));
				});
			}

			if (!name_at.empty())
			{
				AddCommand("search", "Search " + name_at, [&] {

				});
			}
		}
	}
	else
		mTileDescription.AddParagraph("Unknown");

	std::string input_line;
	for (auto& cmd : mCommands)
		input_line += fmt::format("<img={}>{}", cmd.Input, cmd.Text);
	mTileDescription.AddParagraph(input_line);

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

	const auto wall_color = ToAllegro(Colors::White);

	/// Floors
	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		if (!mCurrentMap.NavGrid.WasSeen(pos)) return;

		al_draw_rotated_bitmap(tile->Bg, half_tile_size, half_tile_size, world_pos.x + half_tile_size, world_pos.y + half_tile_size, glm::radians((tile->RotationFlags >> 2) * 90.0f), tile->RotationFlags & 3);
	});

	/// Sub-wall objects
	std::vector<TileObject*> obj_to_draw;

	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		obj_to_draw.clear();

		for (auto obj : tile->Objects)
		{
			if (!obj->Visible() || obj->Position() != pos || obj->Z() >= ObjectZ::Walls)
				continue;
			obj_to_draw.push_back(obj);
		}

		std::sort(obj_to_draw.begin(), obj_to_draw.end(), [](TileObject* a, TileObject* b) { return a->Z() < b->Z(); });
		DrawObjects(obj_to_draw, pos);
	});

	/// Walls
	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		if (!mCurrentMap.NavGrid.WasSeen(pos))
			return;

		auto adj = mCurrentMap.NavGrid.At(pos)->BlocksWall(WallBlocks::Passage);
		if (adj.is_set(Direction::Left)) al_draw_line(world_pos.x, world_pos.y - wall_width / 2, world_pos.x, world_pos.y + tile_width + wall_width / 2, wall_color, wall_width);
		if (adj.is_set(Direction::Up)) al_draw_line(world_pos.x - wall_width / 2, world_pos.y, world_pos.x + tile_width + wall_width / 2, world_pos.y, wall_color, wall_width);
		if (adj.is_set(Direction::Right)) al_draw_line(world_pos.x + tile_width, world_pos.y, world_pos.x + tile_width, world_pos.y + tile_width, wall_color, wall_width);
		if (adj.is_set(Direction::Down)) al_draw_line(world_pos.x - wall_width / 2, world_pos.y + tile_width, world_pos.x + tile_width + wall_width / 2, world_pos.y + tile_width, wall_color, wall_width);
	});

	/// Above-wall objects
	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		obj_to_draw.clear();

		for (auto obj : tile->Objects)
		{
			if (!obj->Visible() || obj->Position() != pos || obj->Z() < ObjectZ::Walls)
				continue;
			obj_to_draw.push_back(obj);
		}

		std::sort(obj_to_draw.begin(), obj_to_draw.end(), [](TileObject* a, TileObject* b) { return a->Z() < b->Z(); });
		DrawObjects(obj_to_draw, pos);
	});

	static constexpr auto half_black = Colors::GetBlack(0.6f);
	
	/// Never seen or hidden
	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		if (!mCurrentMap.NavGrid.WasSeen(pos))
			al_draw_filled_rectangle(world_pos.x, world_pos.y, world_pos.x + tile_width, world_pos.y + tile_width, ToAllegro(Colors::Black));
		else if (!mCurrentMap.NavGrid.Visible(pos))
			al_draw_filled_rectangle(world_pos.x, world_pos.y, world_pos.x + tile_width, world_pos.y + tile_width, ToAllegro(half_black));
	});
	/// Mouse selection
	auto select = mCurrentMap.RoomGrid.RectForTile(GetMouseTilePosition(), tile_size);
	al_draw_filled_rounded_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, 4.0f, 4.0f, ToAllegro(Colors::GetWhite(0.5f)));
	al_draw_rounded_rectangle(select.p1.x, select.p1.y, select.p2.x, select.p2.y, 4.0f, 4.0f, ToAllegro(Colors::Magenta), 4.0f);

	al_use_transform(&mUICamera);

	auto bounds = mTileDescription.GetBounds().grown(8);
	al_draw_filled_rounded_rectangle(bounds.p1.x, bounds.p1.y, bounds.p2.x, bounds.p2.y, 4.0f, 4.0f, ToAllegro(Colors::GetBlack(0.75f)));
	al_draw_rounded_rectangle(bounds.p1.x, bounds.p1.y, bounds.p2.x, bounds.p2.y, 4.0f, 4.0f, ToAllegro(Colors::White), 4.0f);
	mTileDescription.Draw();

	ImGui::Allegro::Render(mDisplay);

	al_flip_display();
}

void Game::UpdateCamera()
{
	auto player_world_pos = mCurrentMap.RoomGrid.TilePositionToWorldPosition(mPlayer->Position(), tile_size) + tile_size / 2.0f;
	mCameraTarget = player_world_pos;
	mCameraFocus = true;
	mCurrentMap.DetermineVisibility(player_world_pos);
}

bool Game::CanMoveIn(Direction move_dir)
{
	const auto target_pos = mPlayer->Position() + ToVector(move_dir);

	return CanMoveTo(target_pos);
}

bool Game::CanMoveTo(ivec2 target_pos)
{
	if (!IsNeighbor(mPlayer->Position(), target_pos))
		return false;

	if (!mCurrentMap->IsValid(target_pos))
		return false;

	if (mCurrentMap.NavGrid.BlocksPassage(mPlayer->Position(), target_pos))
		return false;

	for (auto& obj : mCurrentMap->At(target_pos)->Objects)
		if (obj->BlocksMovement())
			return false;

	return true;
}

void Game::MovePlayer(Direction move_dir)
{
	const auto target_pos = mPlayer->Position() + ToVector(move_dir);
	if (CanMoveIn(move_dir))
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
}

void Game::DrawObjects(gsl::span<TileObject* const> objects, ivec2 pos)
{
	for (auto obj : objects)
	{
		vec2 gfx_offset = { 0, 0 };
		if (obj->WallPosition != Direction::None)
			gfx_offset = vec2{ ToVector(obj->WallPosition) } *half_tile_size;

		al_draw_tinted_scaled_rotated_bitmap(
			obj->Texture,
			ToAllegro(Colors::White),
			half_tile_size, half_tile_size, float(pos.x) * tile_width + half_tile_size + gfx_offset.x, float(pos.y) * tile_width + half_tile_size + gfx_offset.y, 1.0f, 1.0f,
			glm::radians((obj->RotationFlags >> 2) * 90.0f), obj->RotationFlags & 3
		);
	}
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

	/// TODO: Remove and insert to ALL tiles in Size
	mParentMap->RoomGrid.At(mPosition)->Objects.erase(this);
	mPosition = pos;
	mParentMap->RoomGrid.At(mPosition)->Objects.insert(this);
}