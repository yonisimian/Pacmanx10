#include "Game.h"

int main()
{
	pm::Game game;
	if (game.Construct(362, 192, 4, 4))
		game.Start();
	return 0;
}