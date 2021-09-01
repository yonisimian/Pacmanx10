#define OLC_PGE_APPLICATION
#include "Game.h"

int main()
{
	srand(time(NULL));
	pm::Game game;
	if (game.Construct(408, 192, 4, 4))
		game.Start();
	return 0;
}