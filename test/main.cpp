#include "game.h"
#include <gtest/gtest.h>

int ToAllegro(HorizontalAlign align)
{
	switch (align)
	{
	case HorizontalAlign::Center:
		return ALLEGRO_ALIGN_CENTER;
	case HorizontalAlign::Right:
		return ALLEGRO_ALIGN_RIGHT;
	default:
		return ALLEGRO_ALIGN_LEFT;
	}
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	Game game;
	game.Init();
	game.Load();
	game.Start();
	game.Loop();
	game.Shutdown();
}
