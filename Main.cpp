#include "Game.h"

int main()
{
	pm::Game game;
	if (game.Construct(320, 240, 4, 4))
		game.Start();
	return 0;
}