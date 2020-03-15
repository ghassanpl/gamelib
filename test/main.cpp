#include "game.h"

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

int main()
{
	Game game;
	game.Init();
	game.Load();
	game.UpdateCamera();
	game.Start();
	game.Loop();
	game.Shutdown();
}
