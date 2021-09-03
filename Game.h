#ifndef GAME_H
#define GAME_H

#define OLC_PGE_APPLICATION
#define OLC_PGEX_SOUND

#include "olcPixelGameEngine.h"
#include "olcPGEX_Sound.h"
#include "Auxiliaries.h"
#include "LevelEditor.h"

#include <fstream>
#include <bitset>

#define PATH_DATA "./data.txt"
#define PATH_SOUND "./Assets/Sound/"

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
			MM_ABOUT,
			MM_HIGHSCORES,
			GAME_SET,
			GAME_PLAY,
			GAME_PAUSE,
			GAME_WIN,
			LEVEL_EDITOR,
		};

		// =============== menus' stuff
		Title title_game;
		std::vector<Button*> mm_main_buttons;
		std::vector<Button*> mm_abut_buttons;
		std::vector<Button*> mm_high_buttons;

		std::vector<TextBox*> mm_abut_texts;
		std::vector<TextBox*> mm_high_texts;
		bool bQuit;


		// =============== game's stuff

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
		int aBG;
		int aGameover;
		int aLevel;
		std::vector<int> aPac;
		std::vector<int> aClick;
		std::vector<int> aFart;
		std::vector<int> aWah;
		std::vector<int> aYum;
		std::vector<int> aBlbl;
	public:
		Game() :
			title_game (*this, tileToScreen(6, 1), "Pacmanx10"),
			bQuit(false),
			iCurrLevel(0),
			score(0),
			time(0.0f),
			timeCountDown(COUNT_DOWN_TIME),
			chain(0),
			chainCountDown(0),
			lives(DEFAULT_LIFE),
			currState(GameState::MM_MAIN),
			nextState(GameState::MM_MAIN),
			aBG(olc::SOUND::LoadAudioSample(PATH_SOUND "main_menu.wav")),
			aGameover(olc::SOUND::LoadAudioSample(PATH_SOUND "game_over.wav")),
			aLevel(olc::SOUND::LoadAudioSample(PATH_SOUND "level_music.wav"))
		{
			sAppName = "Pacmanx10";
		}

#pragma region Levels Management
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
			resetCurrLevel();
		}

		// load currLevel to be the previous level
		void loadPrevLevel()
		{
			if (--iCurrLevel < 0)
				iCurrLevel = levelDatas.size() - 1;
			resetCurrLevel();
		}

		// load currLevel to be the next level
		bool loadLevel(int level)
		{
			if (level < 0 || level >= levelDatas.size())
				return false;
			iCurrLevel = level;
			resetCurrLevel();
			return true;
		}

		void resetCurrLevel()
		{
			currLevel.reset(new Level(*this, levelDatas[iCurrLevel], false));
		}

#pragma endregion

		bool OnUserCreate() override
		{
			// UI
			mm_main_buttons.push_back(new Button(*this, tileToScreen(13, 5), "Play", [this] { olc::SOUND::StopSample(aBG); olc::SOUND::PlaySample(aLevel, true); nextState = GameState::GAME_SET; }));
			mm_main_buttons.push_back(new Button(*this, tileToScreen(12, 7) + olc::vi2d(iTileSize / 2, 0), "About", [this] { playEffect(SoundEffect::CLICK); nextState = GameState::MM_ABOUT; }));
			mm_main_buttons.push_back(new Button(*this, tileToScreen(10, 9), "Highscores", [this] { playEffect(SoundEffect::CLICK); nextState = GameState::MM_HIGHSCORES; }));
			mm_main_buttons.push_back(new Button(*this, tileToScreen(13, 11), "Quit", [this] { playEffect(SoundEffect::FART); bQuit = true; }));

			mm_abut_buttons.push_back(new Button(*this, tileToScreen(13, 15), "Back", [this] { playEffect(SoundEffect::FART); nextState = GameState::MM_MAIN; }));
			mm_abut_texts.push_back(new TextBox(*this, tileToScreen(6, 5), "Explanation stuff,\n\nI ain't good at it\n\n        UWU"));
			mm_abut_texts.push_back(new TextBox(*this, tileToScreen(6, 12), "We love you David!"));

			mm_high_buttons.push_back(new Button(*this, tileToScreen(13, 15), "Back", [this] { playEffect(SoundEffect::FART); nextState = GameState::MM_MAIN; }));
			mm_high_texts.push_back(new TextBox(*this, tileToScreen(7, 5), "\n\n\n  Coming Soon!  \n\n\n"));

			// Audio
			for (int i = 1; i <= 4; ++i) aPac  .push_back(olc::SOUND::LoadAudioSample(PATH_SOUND "pac_0"    + std::to_string(i) + ".wav"));
			for (int i = 1; i <= 3; ++i) aYum  .push_back(olc::SOUND::LoadAudioSample(PATH_SOUND "yummy_0"  + std::to_string(i) + ".wav"));
			for (int i = 1; i <= 3; ++i) aWah  .push_back(olc::SOUND::LoadAudioSample(PATH_SOUND "wah_0"    + std::to_string(i) + ".wav"));
			for (int i = 1; i <= 2; ++i) aFart .push_back(olc::SOUND::LoadAudioSample(PATH_SOUND "fart_0"   + std::to_string(i) + ".wav"));
			for (int i = 1; i <= 3; ++i) aBlbl .push_back(olc::SOUND::LoadAudioSample(PATH_SOUND "blblbl_0" + std::to_string(i) + ".wav"));
			for (int i = 1; i <= 2; ++i) aClick.push_back(olc::SOUND::LoadAudioSample(PATH_SOUND "click_0"  + std::to_string(i) + ".wav"));

			// Game
			getLevels();
			if (!loadLevel(3)) return false;

			if (!olc::SOUND::InitialiseAudio()) return false;
			olc::SOUND::PlaySample(aBG, true);

			editor = new LevelEditor(*this);

			return true;
		}

		bool OnUserDestroy() override
		{
			olc::SOUND::DestroyAudio();

			return true;
		}

		bool OnUserUpdate(float fElapsedTime) override
		{
			Clear(olc::BLACK);

			switch (currState)
			{
				case GameState::MM_MAIN:
				{
					std::for_each(mm_main_buttons.begin(), mm_main_buttons.end(), [](auto b) { b->update(); });

					title_game.draw();
					std::for_each(mm_main_buttons.begin(), mm_main_buttons.end(), [](auto b) { b->draw(); });
					break;
				}
				case GameState::MM_ABOUT:
				{
					std::for_each(mm_abut_buttons.begin(), mm_abut_buttons.end(), [](auto b) { b->update(); });

					title_game.draw();
					std::for_each(mm_abut_texts.begin(),   mm_abut_texts.end(),   [](auto t) { t->draw(); });
					std::for_each(mm_abut_buttons.begin(), mm_abut_buttons.end(), [](auto b) { b->draw(); });
					break;
				}
				case GameState::MM_HIGHSCORES:
				{
					std::for_each(mm_high_buttons.begin(), mm_high_buttons.end(), [](auto b) { b->update(); });

					title_game.draw();
					std::for_each(mm_high_texts.begin(), mm_high_texts.end(),     [](auto t) { t->draw(); });
					std::for_each(mm_high_buttons.begin(), mm_high_buttons.end(), [](auto b) { b->draw(); });
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
						olc::SOUND::StopSample(aLevel);
						olc::SOUND::PlaySample(aBG);
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

					// update powerUps animation
					std::for_each(currLevel->powerUps.begin(), currLevel->powerUps.end(), [&](std::shared_ptr<PowerUp> pu) {pu->update(fElapsedTime); });

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
							playEffect(SoundEffect::PAC);
							Dot* d = dynamic_cast<Dot*>(it->second.get());
							if (currLevel->isOldschool)
								score += d->value;
							else
							{
								chain <<= 1;
								chain += d->value;
								chain &= (int(pow(2, iChainLength)) - 1);
							}
							chainCountDown = CHAIN_DOWN_TIME;
							currLevel->board.erase(it);
							if (--currLevel->iDots == 0) // end level!
							{
								playEffect(SoundEffect::VICTORY);
								nextState = GameState::GAME_WIN;
							}
							break;
						}
						case Kind::POWER_UP:
							playEffect(SoundEffect::YUMMY);
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
									playEffect(SoundEffect::GHOST);
									resetLevel();
									lives--;
									nextState = GameState::GAME_SET;
									break;
								case Ghost::GhostState::WEAK:
									ghost->makeEaten();
									score += currLevel->isOldschool ? 200 : int(pow(2, iChainLength - 1));
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
					{
						olc::SOUND::StopSample(aBG);
						olc::SOUND::PlaySample(aLevel);
						nextState = GameState::GAME_PLAY;
					}

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

			return !bQuit;
		}

	private:
		void resetLevel()
		{
			currLevel->player->resetPos();
			std::for_each(currLevel->ghosts.begin(), currLevel->ghosts.end(), [&](auto ghost) {ghost->resetPos(); });
		}
		void drawGame()
		{
			//drawDebugGrid(*this, currLevel->width, currLevel->height);

			currLevel->draw();

			// UI
			int x = currLevel->width * iTileSize + 4;
			int y = currLevel->height * iTileSize + 4;
			DrawString({ 0, y + 1 }, "Time:  " + std::to_string((int)time));
			DrawString({ 0, y + 11 }, "Score: " + std::to_string(score));
			DrawString({ 0, y + 21 }, "Lives: " + std::to_string(lives));
			if (!currLevel->isOldschool)
			{
				DrawString({ 0, y + 31 }, "Chain: " + std::bitset<iChainLength>(chain).to_string());
				DrawString({ 0, y + 41 }, "Chain-Time: " + std::to_string(chainCountDown).substr(0, 4));
			}

			// debug tile
			// DrawRect(player->getPos(), { iTileSize, iTileSize }, olc::YELLOW);
		}
		void playEffect(const SoundEffect se)
		{
			std::vector<int> v;
			switch (se)
			{
			case SoundEffect::VICTORY: v = aWah;   break;
			case SoundEffect::YUMMY:   v = aYum;   break;
			case SoundEffect::PAC:     v = aPac;   break;
			case SoundEffect::FART:    v = aFart;  break;
			case SoundEffect::GHOST:   v = aBlbl;  break;
			case SoundEffect::CLICK:   v = aClick; break;
			default: return;
			}
			olc::SOUND::PlaySample(v[rand() % v.size()]);
		}
	};
}

#endif