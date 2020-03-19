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

	al_identity_transform(&mUICamera);
	mCamera.SetFromDisplay(mDisplay);
}

void Game::Load()
{
	mFont = al_load_ttf_font("data/fonts/plantin_regular.ttf", 24, ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	mBigFont = al_load_ttf_font("data/fonts/heroquest.ttf", 60, ALLEGRO_NO_PREMULTIPLIED_ALPHA);

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

	/// Load scripts
	{
		for (auto it = std::filesystem::recursive_directory_iterator{ "data/scripts" }; it != std::filesystem::recursive_directory_iterator{}; ++it)
		{
			auto path = it->path();
			if (path.extension() == ".rsl")
			{
				//mScriptModule
				std::stringstream buffer;
				std::ifstream srm{ path };
				buffer << srm.rdbuf();
				srm.close();

				mScriptModule.Parse(buffer.str(), path.string());
			}
		}

		auto monster_class = mScriptModule.CreateNativeClass<Monster>(mScriptModule.GlobalNamespace(), "Monster", rsl::NativeClassType::RefType);

		monster_class->AddMethod(mScriptModule, "CanSeePlayer", &Monster::CanSeePlayer);
		monster_class->AddMethod(mScriptModule, "Wander", &Monster::Wander);

		mScriptModule.Link();

		mMonsterAIScript = mMonsterAIContext.New(mScriptModule.FindClass("MonsterAI"));
	}

	mImages["input/up"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_W.png");
	mImages["input/down"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_S.png");
	mImages["input/left"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_A.png");
	mImages["input/right"] = al_load_bitmap("shared/ControllerGraphics/Keyboard & Mouse/Light/Keyboard_White_D.png");
	
	mTileDescription.SetImageResolver([&](std::string_view name) {
		return GetImage(name);
	});

	LoadClasses("data/stats/heroes.csv", mHeroClasses);
	LoadClasses("data/stats/monsters.csv", mMonsterClasses);

	mCurrentMap.RoomGrid.ForEach([&](ivec2 pos) {
		auto tile = mCurrentMap.RoomGrid.At(pos);
		tile->Bg = GetImage(fmt::format("floors/floor{}", random::IntegerRange(RNG, 0, 9)));
		tile->RotationFlags = random::IntegerRange(RNG, 0, 15);
	});
	mCurrentMap.BuildRoom(irec2::from_size(1, 1, 4, 3), { 2,1 }, Direction::Up);
	//mCurrentMap.BuildRoom(irec2::from_size(5, 1, 4, 3));
	//mCurrentMap.BuildRoom(irec2::from_size(1, 4, 4, 5));
	//mCurrentMap.NavGrid.SetBlocksPassage({ 2,0 }, { 2,1 }, false);

	mCurrentMap.SpawnObject<Stairs>({ 1,1 });

	mPlayer = mCurrentMap.SpawnObject<Hero>({ 1,1 }, mHeroClasses["Warrior"]);

	mCurrentMap.SpawnObject<Furniture>({ 3,3 });
	auto monster = mCurrentMap.SpawnObject<Monster>({ 4,3 }, mMonsterClasses["Goblin Guard"]);
	mCurrentMap.SpawnObject<Trigger>({ 3,4 });
	mCurrentMap.SpawnObject<Trap>({ 4,4 });
	mCurrentMap.SpawnObject<Item>({ 5,4 });

	for (auto& obj : mCurrentMap.Objects)
	{
		obj->Texture = GetImage(obj->Image());
	}
}

void Game::Start()
{
	UpdateCamera();
	mCamera.SetWorldCenter(mCameraTarget);
	mTiming.Reset();
	SwitchMode(&Game::ModePlayerMovement);
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
		Approach(wc, mCameraTarget, float(mDT * mCameraSpeed));
		mCamera.SetWorldCenter(wc);
	}

	DoModeAction(ModeAction::Update);
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

	const auto wall_color = ToAllegro(Colors::White);
	static constexpr auto half_black = Colors::GetBlack(0.6f);

	/// Floors
	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		if (!mCurrentMap.NavGrid.WasSeen(pos)) return;

		al_draw_rotated_bitmap(tile->Bg, half_tile_size, half_tile_size, world_pos.x + half_tile_size, world_pos.y + half_tile_size, glm::radians((tile->RotationFlags >> 2) * 90.0f), tile->RotationFlags & 3);
	});

	/// Sub-wall objects
	DrawObjects([](TileObject const* obj) { return obj->Z() >= ObjectZ::Walls; });

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
	DrawObjects([](TileObject const* obj) { return obj->Z() < ObjectZ::Walls; });

	/// Never seen or hidden
	ForEachVisibleTile([&](ivec2 pos, RoomTile* tile, vec2 world_pos) {
		if (!mCurrentMap.NavGrid.WasSeen(pos))
			al_draw_filled_rectangle(world_pos.x, world_pos.y, world_pos.x + tile_width, world_pos.y + tile_width, ToAllegro(Colors::Black));
		else if (!mCurrentMap.NavGrid.Visible(pos))
			al_draw_filled_rectangle(world_pos.x, world_pos.y, world_pos.x + tile_width, world_pos.y + tile_width, ToAllegro(half_black));
	});

	al_use_transform(&mUICamera);

	al_draw_filled_rectangle(0, mScreenRect.height() - 50, mScreenRect.width(), mScreenRect.height(), ToAllegro(half_black));
	DrawText(mFont, { 20, mScreenRect.height() - 40 }, Colors::White, Colors::Transparent, Align::LeftTop, "AP:");
	const auto orb = GetImage("ui/orb322");
	for (int i = 0; i < mPlayer->Class->Speed; i++)
	{
		if (i < mPlayer->AP)
		{
			if (mPlayer->AP == 1)
				al_draw_filled_circle(70 + i * 32 + 16, (mScreenRect.height() - 40) + 16, 13, ToAllegro(vec4{ (float)TriangleWave(1.0, 0.0, 4.0, mTiming.RealTotalGameTime()), 0,0,1 }));
			else
				al_draw_filled_circle(70 + i * 32 + 16, (mScreenRect.height() - 40) + 16, 13, ToAllegro(Colors::DarkCyan));
		}
		al_draw_scaled_bitmap(orb, 0, 0, 32, 32, 70 + i * 32, mScreenRect.height() - 40, 32, 32, 0);
	}

	DoModeAction(ModeAction::UIDraw);

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

void Game::DirectionAction(Direction dir)
{
	const auto player_pos = mPlayer->Position();
	const auto target_pos = player_pos + ToVector(dir);

	/// Bump first

	for (auto& obj : mCurrentMap->At(player_pos)->Objects)
		if (obj->WallPosition == dir && obj->TryBump(this, dir))
			return;
	
	if (!mCurrentMap->IsValid(target_pos))
		return;

	if (mCurrentMap.NavGrid.BlocksPassage(player_pos, target_pos))
		return;

	bool can_move = true;
	for (auto& obj : mCurrentMap->At(target_pos)->Objects)
		if (obj->BlocksMovement())
		{
			can_move = false;
			if (obj->TryBump(this, dir))
				return;
		}

	/// Then move
	if (can_move)
	{
		mPlayer->MoveTo(target_pos);
		SpendAP();

		for (auto& obj : mCurrentMap.RoomGrid.At(mPlayer->Position())->Objects)
			if (obj->EnteredTile(this))
				return;
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

void Game::SpendAP()
{
	UpdateCamera();
	mPlayer->AP--;
	if (mPlayer->AP == 0)
	{
		SwitchMode(&Game::ModeEndTurn);
	}
}

void Game::DrawObjects(std::function<bool(TileObject*)> filter)
{
	std::vector<TileObject*> obj_to_draw;

	ForEachVisibleTile([&, filter = std::move(filter)](ivec2 pos, RoomTile* tile, vec2 world_pos) {

		for (auto obj : tile->Objects)
		{
			if (!obj->Visible() || obj->Position() != pos || filter(obj))
				continue;

			bool visible = obj->ShowInFog();
			if (!visible)
			{
				for (int x = 0; x < obj->Size.x; x++)
					for (int y = 0; y < obj->Size.y; y++)
						if (mCurrentMap.NavGrid.Visible(pos + ivec2{ x, y }))
						{
							visible = true;
							break;
						}
			}

			if (visible)
				obj_to_draw.push_back(obj);
		}
	});

	std::sort(obj_to_draw.begin(), obj_to_draw.end(), [](TileObject* a, TileObject* b) { return a->Z() < b->Z(); });

	for (auto obj : obj_to_draw)
	{
		vec2 gfx_offset = { 0, 0 };
		if (obj->WallPosition != Direction::None)
			gfx_offset = vec2{ ToVector(obj->WallPosition) } *half_tile_size;

		al_draw_tinted_scaled_rotated_bitmap(
			obj->Texture,
			ToAllegro(Colors::White),
			half_tile_size, half_tile_size, float(obj->Position().x) * tile_width + half_tile_size + gfx_offset.x, float(obj->Position().y) * tile_width + half_tile_size + gfx_offset.y, 1.0f, 1.0f,
			glm::radians((obj->RotationFlags >> 2) * 90.0f), obj->RotationFlags & 3
		);
	}
}

ALLEGRO_BITMAP* Game::GetImage(std::string_view name) const
{
	auto it = mImages.find(name);
	if (it == mImages.end())
		mReporter.Error("Image '{}' not found", name);
	return it->second;
}

void Game::ModePlayerMovement(ModeAction action)
{
	switch (action)
	{
	case ModeAction::Update:
		if (mInput.WasButtonPressed("up"))
			DirectionAction(Direction::Up);
		else if (mInput.WasButtonPressed("left"))
			DirectionAction(Direction::Left);
		else if (mInput.WasButtonPressed("right"))
			DirectionAction(Direction::Right);
		else if (mInput.WasButtonPressed("down"))
			DirectionAction(Direction::Down);
		break;
	}
}

void Game::ModeEndTurn(ModeAction action)
{
	static constexpr seconds_t sign_time = 0.65;
	switch (action)
	{
	case ModeAction::Update:
		if (mTiming.TimeSinceFlag("mode_entered") >= sign_time)
		{
			mPlayer->AP = mPlayer->Class->Speed;
			SwitchMode(&Game::ModeEvilTurn);
		}
		break;
	case ModeAction::UIDraw:
	{
		auto delta = std::pow(mTiming.FlagDelta("mode_entered", sign_time) * 2.0 - 1.0, 5);
		DrawText(mBigFont, { mScreenRect.width() / 2.0f, (mScreenRect.height() / 2.0f) + mScreenRect.height() * delta }, Colors::Red, Colors::GetBlack(0.7f), Align::CenterMiddle,
			"Evil Turn", mPlayer->AP);
		break;
	}
	}
}

void Game::ModeEvilTurn(ModeAction action)
{
	switch (action)
	{
	case ModeAction::Enter:
		break;
	case ModeAction::Leave:
		break;
	case ModeAction::Update:
		break;
	}
	/*
	auto yield_result = mMonsterAIContext.Call(mMonsterAIScript, monster->Class->AI, monster);
	while (mMonsterAIContext.Suspended())
	{
		fmt::print("yielded: {}\n", yield_result->ToString());
		mMonsterAIContext.Resume(yield_result.Value());
	}
	*/
}

void Game::ModeStartTurn(ModeAction action)
{
	static constexpr seconds_t sign_time = 0.65;
	switch (action)
	{
	case ModeAction::Update:
		if (mTiming.TimeSinceFlag("mode_entered") >= sign_time)
		{
			mPlayer->AP = mPlayer->Class->Speed;
			SwitchMode(&Game::ModePlayerMovement);
		}
		break;
	case ModeAction::UIDraw:
	{
		auto delta = std::pow(mTiming.FlagDelta("mode_entered", sign_time) * 2.0 - 1.0, 5);
		DrawText(mBigFont, { mScreenRect.width() / 2.0f, (mScreenRect.height() / 2.0f) + mScreenRect.height() * delta }, Colors::Green, Colors::GetBlack(0.7f), Align::CenterMiddle,
			"Hero Turn", mPlayer->AP);
		break;
	}
	}
}

void Game::SwitchMode(GameMode mode)
{
	DoModeAction(ModeAction::Leave);
	mCurrentMode = mode;
	mTiming.SetFlag("mode_entered");
	DoModeAction(ModeAction::Enter);
}

void Game::ReportSingle(rsl::ReportType type, rsl::ReportModule in_module, rsl::SourcePos const& pos, std::string_view message)
{
	OSInterface::ReportSingle(type, in_module, pos, message);
	if ((int)type >= (int)ReportType::Warning)
	{
		mReporter.Error("Script {}: {}: {}: {}", type, in_module, pos.ToString(), message);
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

bool Door::TryBump(Game* game, Direction from)
{
	if (!Open)
	{
		game->OpenDoor(this);
		return true;
	}
	else
		return false;
}

void Game::OpenDoor(Door* door)
{
	if (door->Locked)
	{
		/// TODO: Show locked message
		return;
	}

	auto pos = door->Position();
	auto dir = door->WallPosition;
	mCurrentMap.NavGrid.SetBlocksPassage(pos, pos + ToVector(dir), false);
	door->Texture = GetImage(door->OpenImage());
	door->Open = true;

	SpendAP();
}	

bool Monster::CanSeePlayer() const
{
	/// TODO: Last Player Position stuff
	//return Game()->OurWorld->NavigationGrid().CanSee(Position(), Game()->CurrentPlayer->GetPosition(), true);
	return false;
}

bool Monster::CanAttackPlayer() const
{
	/*
	if (CanAttackDiagonally())
	return IsSurrounding(Position(), Game()->CurrentPlayer->GetPosition());
	else
	return IsNeighbor(Position(), Game()->CurrentPlayer->GetPosition());
	*/
	return false;
}

void Monster::AttackPlayer()
{

}

bool Monster::CanMoveTowardPlayer()
{
	//return Game()->CanEnter(GetPosition() + DirToPlayer());
	return false;
}

void Monster::MoveTowardPlayer()
{
	//SetPosition(GetPosition() + DirToPlayer());
}

void Monster::Wander()
{
	/*
	auto wander_pos = GetPosition() + Game()->GetRandomNeighbor();
	if (Game()->CanEnter(wander_pos))
	SetPosition(wander_pos);
	*/
}

void Monster::AITurn()
{
	if (CanSeePlayer())
	{
		/// mLastSeenPlayerPosition = Game()->CurrentPlayer->GetPosition();
		/// mLastSeenPlayerTime = Game()->WorldTime
		if (CanAttackPlayer())
			AttackPlayer();
		else if (CanMoveTowardPlayer())
			MoveTowardPlayer();
	}
	else
		Wander();

	/*
	if (Health() < ScaredHealth())
	{
	if (CanRunAwayFromPlayer())
	RunAwayFromPlayer(); /// NOTE: Different than MoveAwayFromPlayer
	else if (CanAttackPlayer())
	AttackPlayer();
	}
	else if (TooFarFromPlayer() and CanAttackPlayer() and CanMoveTowardPlayer())
	{
	if (Game()->WithProbability(ChargeProbability()))
	MoveTowardPlayer();
	else
	AttackPlayer();
	}
	else if (TooCloseToPlayer() and CanAttackPlayer() and CanMoveAwayFromPlayer())
	{
	if (Game()->WithProbability(RetreatProbability()))
	MoveAwayFromPlayer();
	else
	AttackPlayer();
	}
	else if (CanAttackPlayer())
	AttackPlayer();
	else if (TooFarFromPlayer() and CanMoveTowardPlayer())
	MoveTowardPlayer();
	else if (TooCloseToPlayer() and CanMoveAwayFromPlayer())
	MoveAwayFromPlayer();
	else
	StandStill();
	*/
}
