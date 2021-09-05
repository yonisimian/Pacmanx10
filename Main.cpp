#include "Game.h"

int main()
{
	pm::Game game;
	if (game.Construct(296, 212, 4, 4))
		game.Start();
	return 0;
}