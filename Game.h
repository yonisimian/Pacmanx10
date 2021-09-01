#ifndef GAME_H
#define GAME_H

#include "olcPixelGameEngine.h"
#include "olcPGEX_Sound.h"
#include "Auxiliaries.h"
#include "LevelEditor.h"

#include <fstream>
#include <bitset>

#define PATH_DATA "./data.txt"

namespace fs = std::experimental::filesystem;

namespace pm
{
	class Game : public olc::PixelGameEngine
	{
		static const float constexpr COUNT_DOWN_TIME = 3.0f;
		static const float constexpr CHAIN_DOWN_TIME = 1.0f;
		static const int DEFAULT_LIFE = 3;

		enum class GameState {
			MM_MAIN,
			MM_OPTIONS,
			MM_ABOUT,
			MM_HIGHSCORES,
			GAME_SET,
			GAME_PLAY,
			GAME_PAUSE,
			GAME_WIN,
			LEVEL_EDITOR,
		};

		std::vector<LevelData> levelDatas;

		std::unique_ptr<Level> currLevel;
		int iCurrLevel;

		int score;
		float time;
		float timeCountDown; // for GAME_SET
		int lives;

		// futuristic gameplay
		int chain;
		float chainCountDown;

		GameState currState;
		GameState nextState;

		LevelEditor* editor;


		// sound management
		//olc::SOUND::AudioSample aDemoSample;
		//int aTmpSample = 0;
	public:
		Game() :
			iCurrLevel(0),
			score(0),
			time(0.0f),
			timeCountDown(COUNT_DOWN_TIME),
			chain(0),
			chainCountDown(CHAIN_DOWN_TIME),
			lives(DEFAULT_LIFE),
			currState(GameState::GAME_SET),
			nextState(GameState::GAME_SET)
			//aDemoSample("./Assets/Sound/demo.wav"),
			//aTmpSample(olc::SOUND::LoadAudioSample("./Assets/Sound/demo.wav"))
		{
			sAppName = "Pacmanx10";
		}

		// load all the levels from "data.txt" to 
		void getLevels()
		{
			std::ifstream inputDataFile{ PATH_DATA };
			
			while (inputDataFile.good())
			{
				std::string result;
				std::string buffer;
				int lastLineSize = 0; // lines length, aka width
				int i = 0;			  // num of lines, aka height
				for (; std::getline(inputDataFile, buffer) && !buffer.empty(); i++)
				{
					if (i > 0 && buffer.size() != lastLineSize)
					{
						std::cout << "corrupted file!" << std::endl;
						lastLineSize = 0;
						break;
					}
					result += buffer;
					lastLineSize = buffer.size();
				}

				// check validity
				if (lastLineSize == 0)
					continue;
				//int count[static_cast<int>(Kind::COUNT)];
				//for (auto c : result)
				//	count[static_cast<int>(charToKind(c))]++;
				//if (count[static_cast<int>(Kind::PLAYER)] != 1 || count[static_cast<int>(Kind::DOT)] < 1 || (count[static_cast<int>(Kind::GHOST_B)] +
				//	count[static_cast<int>(Kind::GHOST_R)] + count[static_cast<int>(Kind::GHOST_G)] + count[static_cast<int>(Kind::GHOST_Y)] < 1))
				//	continue;
				
				levelDatas.push_back({ result, lastLineSize, i });
			}

			inputDataFile.close();
		}

		// load currLevel to be the next level
		void loadNextLevel()
		{
			if (++iCurrLevel == levelDatas.size())
				iCurrLevel = 0;
			currLevel.reset(new Level(*this, levelDatas[iCurrLevel], false));
		}

		// load currLevel to be the previous level
		void loadPrevLevel()
		{
			if (--iCurrLevel < 0)
				iCurrLevel = levelDatas.size() - 1;
			currLevel.reset(new Level(*this, levelDatas[iCurrLevel], false));
		}

		// load currLevel to be the next level
		bool loadLevel(int level)
		{
			if (level < 0 || level >= levelDatas.size())
				return false;
			currLevel.reset(new Level(*this, levelDatas[level], false));
			iCurrLevel = level;
			return true;
		}


		bool OnUserCreate() override
		{
			getLevels();
			loadLevel(0);

			editor = new LevelEditor(*this);

			return true;
		}

		bool OnUserUpdate(float fElapsedTime) override
		{
			Clear(olc::BLACK);

			switch (currState)
			{
				case GameState::MM_MAIN:
				{
					break;
				}
				case GameState::MM_OPTIONS:
				{
					break;
				}
				case GameState::MM_ABOUT:
				{
					break;
				}
				case GameState::MM_HIGHSCORES:
				{
					break;
				}
				case GameState::GAME_SET:
				{
					currLevel->player->getInput(*this);

					timeCountDown -= fElapsedTime;
					if (timeCountDown <= 0)
					{
						nextState = GameState::GAME_PLAY;
						timeCountDown = COUNT_DOWN_TIME;
						break;
					}

					drawGame();

					FillRectDecal(currLevel->vPos, tileToScreen(currLevel->width, currLevel->height) + olc::vi2d(1, 1), olc::Pixel(0, 0, 0, 150));

					DrawStringDecal(tileToScreen(currLevel->width / 2 - 1, currLevel->height / 2 - 1), std::to_string(int(timeCountDown) + 1));

					break;
				}
				case GameState::GAME_PLAY:
				{
					// ============== INPUT ==============
					currLevel->player->getInput(*this);
					if (GetKey(olc::P).bPressed)
					{
						nextState = GameState::GAME_PAUSE;
						break;
					}
					if (GetKey(olc::E).bPressed)
					{
						nextState = GameState::LEVEL_EDITOR;
						break;
					}

					// ============== UPDATE ==============
					time += fElapsedTime;
					chainCountDown -= fElapsedTime;
					
					if (chainCountDown < 0.0f)
					{
						score += chain;
						chain = 0;
						chainCountDown = 0;
					}

					// update pacman
					currLevel->player->update(fElapsedTime);
					olc::vi2d offset = olc::vi2d(0, 0);
					BOARD_MAP::iterator it = currLevel->player->getCollision(currLevel->board);
					if (it != currLevel->board.end())
						switch (it->second->kind)
						{
						case Kind::WALL:
							currLevel->player->collideWithWall();
							break;
						case Kind::DOT:
						{
							//olc::SOUND::PlaySample(aTmpSample);
							Dot* d = dynamic_cast<Dot*>(it->second.get());
							if (currLevel->isOldschool)
								score += d->value;
							else if (chainCountDown > 0.0f)
							{
								chain <<= 1;
								chain += d->value;
								chain &= 0b1111111111111111;
							}
							chainCountDown = CHAIN_DOWN_TIME;
							currLevel->board.erase(it);
							if (--currLevel->iDots == 0) // end level!
								nextState = GameState::GAME_WIN;
							break;
						}
						case Kind::POWER_UP:
							std::for_each(currLevel->ghosts.begin(), currLevel->ghosts.end(), [&](auto ghost) {ghost->makeWeak(); });
							score += 50;
							currLevel->board.erase(it);
							break;
						}

					if (nextState != GameState::GAME_WIN) // not sure about this
					{
						// update ghosts
						for (auto ghost : currLevel->ghosts)
						{
							// move forward
							ghost->update(fElapsedTime);

							// check collision with pacman
							if (checkCollision(currLevel->player->getPos(), ghost->getPos()))
							{
								switch (ghost->getState())
								{
								case Ghost::GhostState::STRONG:
									resetGame();
									lives--;
									nextState = GameState::GAME_SET;
									break;
								case Ghost::GhostState::WEAK:
									ghost->makeEaten();
									score += 200;
									break;
									//case Ghost::GhostState::EATEN: break;
								}
							}

							// check collision with walls
							it = ghost->getCollision(currLevel->board);
							if (it != currLevel->board.end() && it->second->kind == Kind::WALL)
								ghost->collideWithWall();
						}
					}

					// ============== DRAW ==============
					drawGame();

					break;
				}
				case GameState::GAME_PAUSE:
				{
					if (GetKey(olc::ESCAPE).bPressed)
						nextState = GameState::GAME_PLAY;

					drawGame();

					FillRectDecal(currLevel->vPos, tileToScreen(currLevel->width, currLevel->height) + olc::vi2d(1, 1), olc::Pixel(0, 0, 0, 150));

					DrawStringDecal(tileToScreen(currLevel->width / 2 - 3, currLevel->height / 2 - 1), "PAUSE!");
					break;
				}
				case GameState::GAME_WIN:
				{
					loadNextLevel();
					nextState = GameState::GAME_SET;

					drawGame();

					break;
				}
				case GameState::LEVEL_EDITOR:
				{
					if (!editor->update())
						nextState = GameState::GAME_SET;
					break;
				}
			}

			currState = nextState;

			return true;
		}

	private:
		void resetGame()
		{
			currLevel->player->resetPos();
			std::for_each(currLevel->ghosts.begin(), currLevel->ghosts.end(), [&](auto ghost) {ghost->resetPos(); });
		}
		void drawGame()
		{
			drawDebugGrid(*this, currLevel->width, currLevel->height);

			currLevel->draw();

			// UI
			int x = currLevel->width * iTileSize + 4;
			int y = currLevel->height * iTileSize + 4;
			DrawString({ 0, y + 1 }, "Time:  " + std::to_string((int)time));
			DrawString({ 0, y + 11 }, "Score: " + std::to_string(score));
			DrawString({ 0, y + 21 }, "Lives: " + std::to_string(lives));
			if (!currLevel->isOldschool)
			{
				DrawString({ 0, y + 31 }, "Chain: " + std::bitset<16>(chain).to_string());
				DrawString({ 0, y + 41 }, "Chain-Time: " + std::to_string(chainCountDown));
			}

			// debug tile
			// DrawRect(player->getPos(), { iTileSize, iTileSize }, olc::YELLOW);
		}
	};
}

#endif