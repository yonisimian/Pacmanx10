#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include "olcPixelGameEngine.h"
#include "Auxiliaries.h"

namespace pm
{	
	class LevelEditor
	{
		olc::PixelGameEngine& game;
		std::vector<olc::Decal*>& decals;
		olc::vi2d vEditorPos; // in "tile space"

		std::vector<Button*> buttons;
		std::vector<GameObject*> selectableTiles;
		GameObject* selectedObject;

		Level* currLevel;
	public:
		LevelEditor(olc::PixelGameEngine& game, std::vector<olc::Decal*>& decals) :
			game(game),
			decals(decals),
			vEditorPos({ 10, 0 })
		{
			currLevel = new Level(game, decals, tileToScreen(vEditorPos));

			buttons.push_back(new Button(game, tileToScreen(6, 0), "+1", [this] { currLevel->incrementWidth(1);   }));
			buttons.push_back(new Button(game, tileToScreen(8, 0), "+5", [this] { currLevel->incrementWidth(5);   }));
			buttons.push_back(new Button(game, tileToScreen(6, 1), "+1", [this] { currLevel->incrementHeight(1);  }));
			buttons.push_back(new Button(game, tileToScreen(8, 1), "+5", [this] { currLevel->incrementHeight(5);  }));
			buttons.push_back(new Button(game, tileToScreen(2, 0), "-1", [this] { currLevel->incrementWidth(-1);  }));
			buttons.push_back(new Button(game, tileToScreen(0, 0), "-5", [this] { currLevel->incrementWidth(-5);  }));
			buttons.push_back(new Button(game, tileToScreen(2, 1), "-1", [this] { currLevel->incrementHeight(-1); }));
			buttons.push_back(new Button(game, tileToScreen(0, 1), "-5", [this] { currLevel->incrementHeight(-1); }));
			buttons.push_back(new Button(game, tileToScreen(4, 0), &currLevel->width, [] {}, false));
			buttons.push_back(new Button(game, tileToScreen(4, 1), &currLevel->height, [] {}, false));
			buttons.push_back(new Button(game, tileToScreen(5, 10), "save"));
			buttons.push_back(new Button(game, tileToScreen(0, 10), "back"));

			selectableTiles.push_back(new RedGhost (game, tileToScreen(0, 3), currLevel->width, currLevel->height, currLevel->board));
			selectableTiles.push_back(new BlueGhost(game, tileToScreen(2, 3), currLevel->width, currLevel->height, currLevel->board));
			selectableTiles.push_back(new Dot      (game, tileToScreen(4, 3)));
			selectableTiles.push_back(new Wall     (game, tileToScreen(0, 5)));
			selectableTiles.push_back(new PowerUp  (game, tileToScreen(2, 5)));
			selectableTiles.push_back(new Pacman   (game, tileToScreen(4, 5), currLevel->width, currLevel->height));

			selectedObject = new PowerUp(game, tileToScreen(2, 5));
		}
		//LevelEditor& operator=(LevelEditor& other) = default;
		bool update()
		{
			if (game.GetKey(olc::ESCAPE).bPressed)
				return false;
			if (buttons.back()->isClicked())
				return false;
			if (game.GetKey(olc::F1).bPressed)
				std::cout << currLevel->exportLevel() << std::endl;


			std::for_each(buttons.begin(), buttons.end(), [](auto b) { b->update(); });
			if (game.GetMouse(0).bPressed)
			{
				olc::vi2d pos = screenToTile(game.GetMousePos());
				if (pos.x >= vEditorPos.x && pos.y >= vEditorPos.y && pos.x < vEditorPos.x + currLevel->width && pos.y < vEditorPos.y + currLevel->height)
				{
					pos -= vEditorPos;
					switch (selectedObject->kind)
					{
					case Kind::PLAYER:   currLevel->board.emplace(std::make_pair(pos, new Pacman   (game, tileToScreen(pos), currLevel->width, currLevel->height))); break;
					case Kind::GHOST_B:  currLevel->board.emplace(std::make_pair(pos, new BlueGhost(game, tileToScreen(pos), currLevel->width, currLevel->height, currLevel->board)));   break;
					case Kind::GHOST_R:  currLevel->board.emplace(std::make_pair(pos, new RedGhost (game, tileToScreen(pos), currLevel->width, currLevel->height, currLevel->board)));   break;
					case Kind::DOT:      currLevel->board.emplace(std::make_pair(pos, new Dot      (game, tileToScreen(pos)))); break;
					case Kind::WALL:     currLevel->board.emplace(std::make_pair(pos, new Wall     (game, tileToScreen(pos)))); break;
					case Kind::POWER_UP: currLevel->board.emplace(std::make_pair(pos, new PowerUp  (game, tileToScreen(pos)))); break;
					}
				}
				else
				{
					std::vector<GameObject*>::iterator it = std::find_if(selectableTiles.begin(), selectableTiles.end(), [&](auto obj) {return screenToTile(obj->vInitPos) == pos; });
					if (it != selectableTiles.end())
					{
						delete selectedObject;
						switch ((*it)->kind)
						{
						case Kind::PLAYER:   selectedObject = new Pacman   (game, tileToScreen(pos), currLevel->width, currLevel->height); break;
						case Kind::GHOST_B:  selectedObject = new BlueGhost(game, tileToScreen(pos), currLevel->width, currLevel->height, currLevel->board); break;
						case Kind::GHOST_R:  selectedObject = new RedGhost (game, tileToScreen(pos), currLevel->width, currLevel->height, currLevel->board); break;
						case Kind::DOT:      selectedObject = new Dot      (game, tileToScreen(pos)); break;
						case Kind::WALL:     selectedObject = new Wall     (game, tileToScreen(pos)); break;
						case Kind::POWER_UP: selectedObject = new PowerUp  (game, tileToScreen(pos)); break;
						}
						std::cout << selectedObject->vInitPos << std::endl;
					}
				}
			}
			if (game.GetMouse(1).bPressed)
			{
				currLevel->eraseAt(screenToTile(game.GetMousePos()));
			}

			drawDebugGrid(game, currLevel->width, currLevel->height, vEditorPos);

			std::for_each(buttons.begin(), buttons.end(), [](auto b) { b->draw(); });
			std::for_each(selectableTiles.begin(), selectableTiles.end(), [](auto obj) {obj->draw(); });
			currLevel->draw();

			if (selectedObject != NULL)
			{
				if (MoveableObject* obj = dynamic_cast<MoveableObject*>(selectedObject))
					obj->setPos(getFixedPos(game.GetMousePos()));
				else
					selectedObject->vInitPos = getFixedPos(game.GetMousePos());
				selectedObject->draw();
			}

			// debug tile
			// game.DrawRect(getFixedPos(game.GetMousePos()), { nTileSize, nTileSize }, olc::YELLOW);

			return true;
		}
	};

}

#endif