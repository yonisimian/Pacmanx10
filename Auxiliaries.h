#ifndef AUXILIARIES_H
#define AUXILIARIES_H

#define BOARD_MAP std::map<olc::vi2d, std::shared_ptr<GameObject>, std::function<bool(const olc::vi2d& v1, const olc::vi2d& v2)>>
#define MAKE_BOARD BOARD_MAP([&](auto v1, auto v2) { return v1.y == v2.y ? v1.x < v2.x : v1.y < v2.y; })
#define MAKE_TILE(game, type, image) (std::make_pair(pos, std::shared_ptr<GameObject>(new type(game, tileToScreen(pos), isOldschool, image))))
#define MAKE_GHOST(type) std::shared_ptr<Ghost>(new type(game, tileToScreen(pos), width, height, board, isOldschool, decals[SPRITE_GHOST]))

#define PATH_DATA "./Assets/data.txt"
#define PATH_SOUND "./Assets/Sound/"
#define PATH_GRAPHICS "./Assets/Graphics/"

#include <queue>
#include <random>

namespace pm
{
#pragma region Definitions

	const int iTileSize = 8;
	const olc::vf2d vTile(iTileSize, iTileSize);
	const olc::vf2d vHalfTile(iTileSize / 2.0f, iTileSize / 2.0f);

	const int iPacmanSpeed = 30;
	const int iGhostSpeed = 21;
	const int iDotValue = 10;
	const int iChainLength = 16;
	const int iGhostValue = 200;

	const int DEFAULT_LEVEL_WIDTH = 16;
	const int DEFAULT_LEVEL_HEIGHT = 16;

	static const char SYMBOL_EMPTY  =  ' ';
	static const char SYMBOL_PLAYER =  'p';
	static const char SYMBOL_GHOSTR =  'r';
	static const char SYMBOL_GHOSTB =  'b';
	static const char SYMBOL_GHOSTY =  'y';
	static const char SYMBOL_GHOSTG =  'g';
	static const char SYMBOL_WALL   =  '#';
	static const char SYMBOL_DOT    =  '.';
	static const char SYMBOL_POWERUP = 'o';

	static const std::array<std::string, 4> SPRITE_NAMES = { "wall2.png" , "pacman2.png", "mini pacman2.png", "ghost2.png"};
	enum SpritesNames {
		SPRITE_WALL = 0,
		SPRITE_PACMAN,
		SPRITE_MINI_PACMAN,
		SPRITE_GHOST
	};

	enum class Dir {
		UP = 0,
		DOWN,
		LEFT,
		RIGHT
	};
	enum class Kind {
		EMPTY = 0,
		WALL,
		DOT,
		POWER_UP,
		PLAYER,
		GHOST_R,
		GHOST_B,
		GHOST_Y,
		GHOST_G,

		COUNT
	};
	enum class SoundEffect {
		// for effects that have multiple versions
		CLICK = 0,
		FART,
		PAC,
		YUMMY,
		GHOST_EAT_ME,
		GHOST_EATEN,
		VICTORY
	};
	struct LevelData {
		std::string data;
		int width;
		int height;
	};

	/*auto randomBool() {
		static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
		return gen();
	}*/

	bool randomBool(const float p = 0.5) {
		static auto dev = std::random_device();
		static auto gen = std::mt19937{ dev() };
		static auto dist = std::uniform_real_distribution<float>(0, 1);
		return (dist(gen) < p);
	}

	olc::vi2d tileToScreen(int x, int y)
	{
		return olc::vi2d {
			x * iTileSize,
			y * iTileSize
		};
	};
	olc::vi2d tileToScreen(const olc::vf2d& pos)
	{
		return olc::vi2d{
			(int)pos.x * iTileSize,
			(int)pos.y * iTileSize
		};
	};
	olc::vi2d screenToTile(const olc::vf2d& pos)
	{
		return olc::vi2d{
			(int)pos.x / iTileSize,
			(int)pos.y / iTileSize
		};
	};
	olc::vi2d getFixedPos(const olc::vf2d& pos)
	{
		olc::vi2d v = pos / iTileSize;
		return v * iTileSize;
	};
	char kindToChar(const Kind kind)
	{
		switch (kind)
		{
		case Kind::EMPTY:	 return SYMBOL_EMPTY;
		case Kind::PLAYER:	 return SYMBOL_PLAYER;
		case Kind::GHOST_B:  return SYMBOL_GHOSTB;
		case Kind::GHOST_R:  return SYMBOL_GHOSTR;
		case Kind::GHOST_Y:  return SYMBOL_GHOSTY;
		case Kind::GHOST_G:  return SYMBOL_GHOSTG;
		case Kind::DOT:      return SYMBOL_DOT;
		case Kind::WALL:     return SYMBOL_WALL;
		case Kind::POWER_UP: return SYMBOL_POWERUP;
		}
		return SYMBOL_EMPTY;
	}
	Kind charToKind(const char c)
	{
		switch (c)
		{
		case SYMBOL_EMPTY:   return Kind::EMPTY;
		case SYMBOL_WALL:	 return Kind::WALL;
		case SYMBOL_GHOSTR:	 return Kind::GHOST_R;  
		case SYMBOL_GHOSTB:  return Kind::GHOST_B;
		case SYMBOL_GHOSTY:  return Kind::GHOST_Y;
		case SYMBOL_GHOSTG:  return Kind::GHOST_G;
		case SYMBOL_PLAYER:  return Kind::PLAYER;   
		case SYMBOL_DOT:     return Kind::DOT;      
		case SYMBOL_POWERUP: return Kind::POWER_UP;
		}
		return Kind::EMPTY;
	}
	// width, height and pos are in "Tile Space" (and not "Screen Space")
	void drawDebugGrid(olc::PixelGameEngine& game, const int width, const int height, const olc::vi2d& pos = { 0, 0 })
	{
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
				game.DrawRect(tileToScreen(x + pos.x, y + pos.y), { iTileSize, iTileSize }, olc::Pixel(255, 255, 255, 40));
	}
	bool checkCollision(const olc::vi2d& v1, const olc::vi2d& v2)
	{
		return abs(v1.x - v2.x) < iTileSize && abs(v1.y - v2.y) < iTileSize;
	}
	bool isPointInRect(const olc::vf2d& point, const olc::vf2d& rectPos, const olc::vf2d& rectSize)
	{
		return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x && point.y >= rectPos.y && point.y <= rectPos.y + rectSize.y;
	}

#pragma endregion

	struct GameObject
	{
		olc::PixelGameEngine& game;
		Kind kind;
		olc::vf2d vInitPos;
		olc::Decal* image;
		bool isOldschool; // true for isOldschool gameplay, false for futuristic gameplay

		GameObject(olc::PixelGameEngine& game, Kind kind, const olc::vi2d& vInitPos, olc::Decal* image, bool isOldschool) :
			game(game),
			kind(kind),
			vInitPos(vInitPos),
			image(image),
			isOldschool(isOldschool)
		{}
		virtual void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const = 0;
		virtual void update(float fElapsedTime) {}
		const char getSymbol() const { return kindToChar(kind); }
	};
	struct Wall : public GameObject
	{
		uint8_t walls; // 0b1111, one bit for each wall (up / down / left / right)
		olc::Pixel color;
		Wall(olc::PixelGameEngine& game, const olc::vi2d& vInitPos, bool isOldschool = true, olc::Decal* image = nullptr, const olc::Pixel color = olc::WHITE) :
			GameObject(game, Kind::WALL, vInitPos, image, isOldschool),
			walls(0b1111),
			color(color)
		{};
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			//game.DrawRect(vInitPos + offset, vTile, color);
			game.FillRect(vInitPos + offset, vTile, olc::Pixel(255, 255, 255, 50));

			//game.DrawDecal(vInitPos + offset, image);

			// draw walls outlines
			if (walls & 0b1000) game.DrawLine(vInitPos + offset,						   vInitPos + offset + olc::vi2d(iTileSize - 1, 0        ), color);
			if (walls & 0b0100) game.DrawLine(vInitPos + offset + olc::vi2d(0, iTileSize - 1), vInitPos + offset + olc::vi2d(iTileSize - 1, iTileSize - 1), color);
			if (walls & 0b0010) game.DrawLine(vInitPos + offset,						   vInitPos + offset + olc::vi2d(0,             iTileSize - 1), color);
			if (walls & 0b0001) game.DrawLine(vInitPos + offset + olc::vi2d(iTileSize - 1, 0), vInitPos + offset + olc::vi2d(iTileSize - 1, iTileSize - 1), color);
		}
	};
	struct Dot : public GameObject
	{
		int value;
		Dot(olc::PixelGameEngine& game, const olc::vi2d& vInitPos, bool isOldschool = true, olc::Decal* image = nullptr) :
			GameObject(game, Kind::DOT, vInitPos, image, isOldschool),
			value(isOldschool ? iDotValue : randomBool(0.6f))
		{}
		virtual void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			if (isOldschool)
				game.FillCircle(vInitPos + offset + olc::vi2d(iTileSize / 2, iTileSize / 2), iTileSize / 8);
			else
				game.DrawString(vInitPos + offset + olc::vi2d(1, 1) , std::to_string(value));
		}
	};
	struct PowerUp : public GameObject
	{
		static const int speed = 400;
		float blue;
		bool dirUp;
		PowerUp(olc::PixelGameEngine& game, const olc::vi2d& vInitPos, bool isOldschool = true, olc::Decal * image = nullptr) :
			GameObject(game, Kind::POWER_UP, vInitPos, image, isOldschool),
			blue(255),
			dirUp(false)
		{}
		void update(float fElapsedTime) override
		{
			if (blue >= 255) dirUp = false;
			if (blue <= 0)   dirUp = true;
			blue += (dirUp ? 1 : -1) * fElapsedTime * speed;
		}
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			game.FillCircle(vInitPos + offset + olc::vi2d(iTileSize / 2, iTileSize / 2), iTileSize / 4, olc::Pixel(212, 212, int(blue)));
		}
	};

	class MoveableObject : public GameObject
	{
	protected:
		olc::vf2d vPos;
		Dir initDir;
		Dir currDir;
		Dir nextDir;
		int iSpeed;
		int iLevelWidth;
		int iLevelHeight;
	public:
		MoveableObject(olc::PixelGameEngine& game, const Kind kind, const olc::vf2d& pos, olc::Decal* image, bool isOldschool, const int speed, const int iLevelWidth, const int iLevelHeight, const Dir dir = Dir::RIGHT) :
			GameObject(game, kind, pos, image, isOldschool),
			initDir(dir),
			iSpeed(speed),
			iLevelWidth(iLevelWidth),
			iLevelHeight(iLevelHeight)
		{
			resetPos();
		}
		virtual void update(float fElapsedTime) = 0;
		BOARD_MAP::iterator getCollision(BOARD_MAP& board)
		{
			olc::vi2d offset(0, 0);
			switch (currDir)
			{
			case Dir::DOWN:  offset = olc::vi2d(0, 1); break;
			case Dir::RIGHT: offset = olc::vi2d(1, 0); break;
			}
			olc::vi2d pos = screenToTile(vPos) + offset;

			if (!checkCollision(vPos, tileToScreen(pos)))
				return board.end();
			return board.find(pos);
		}
		virtual void collideWithWall()
		{
			stepBack();
		}
		void resetPos()
		{
			vPos = vInitPos;
			currDir = nextDir = initDir;
		}
		const olc::vf2d& getPos() const { return vPos; }
		olc::vf2d* getPosPtr() { return &vPos; }
		void setPos(olc::vf2d& pos) { vPos = pos; }
		void setPos(olc::vi2d pos) { vPos = { float(pos.x), float(pos.y) }; }
		const Dir getDir() const { return currDir; }
		//void setDir(Dir dir) { this->dir = dir; }
	protected:
		void stepForward(float fElapsedTime)
		{
			if (currDir != nextDir && (int)vPos.x % iTileSize == 0 && (int)vPos.y % iTileSize == 0)
				currDir = nextDir;
			switch (currDir)
			{
			case Dir::UP:	 vPos.y -= fElapsedTime * iSpeed; break;
			case Dir::DOWN:	 vPos.y += fElapsedTime * iSpeed; break;
			case Dir::LEFT:	 vPos.x -= fElapsedTime * iSpeed; break;
			case Dir::RIGHT: vPos.x += fElapsedTime * iSpeed; break;
			}
			if (vPos.x > iLevelWidth * iTileSize) vPos.x = 0;
			if (vPos.y > iLevelHeight * iTileSize) vPos.y = 0;
			if (vPos.x < 0) vPos.x = iLevelWidth * iTileSize;
			if (vPos.y < 0) vPos.y = iLevelHeight * iTileSize;
		}
		void stepBack()
		{
			vPos = screenToTile(vPos);
			switch (currDir)
			{
			case Dir::UP:	vPos.y++; break;
			case Dir::LEFT: vPos.x++; break;
			}
			vPos = tileToScreen(vPos.x, vPos.y);
		}
	};

#pragma region Ghosts 
	class Ghost : public MoveableObject
	{
		static const int WEAK_TIME = 6;

	public:
		static enum class GhostState {
			STRONG,
			WEAK,
			EATEN,
		};

	protected:
		olc::Pixel color;
		GhostState currState;
		float fWeakTime;
		olc::vf2d* vTargetPos;
		olc::vf2d* vCurrTarget;
		BOARD_MAP& board;

		/* a map of distances from target.
		positive number = the distance.
		-1 = wall.
		-2 = this (ghost). */
		int* map;
	public:
		Ghost(olc::PixelGameEngine& game, const olc::vi2d& vPos, olc::Decal* image, olc::Pixel color, Kind kind, const int levelWidth, const int levelHeight, BOARD_MAP& board, bool isOldschool = true, olc::vf2d* vTargetPos = nullptr, const Dir initialDir = Dir::RIGHT) :
			MoveableObject(game, kind, vPos, image, isOldschool, iGhostSpeed, levelWidth, levelHeight, initialDir),
			color(color),
			currState(GhostState::STRONG),
			fWeakTime(WEAK_TIME),
			vTargetPos(vTargetPos),
			vCurrTarget(vTargetPos),
			board(board),
			map(new int[levelWidth * levelHeight])
		{}
		virtual ~Ghost() { delete map; }
		virtual void recalculateRoute() = 0;
		void collideWithWall() override
		{
			stepBack();
			recalculateRoute();
		}
		void update(float fElapsedTime)
		{
			switch (currState)
			{
			case GhostState::STRONG: updateStrong(fElapsedTime); break;
			case GhostState::WEAK:	 updateWeak(fElapsedTime); break;
			case GhostState::EATEN:  updateEaten(fElapsedTime); break;
			}
		}
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			switch (currState)
			{
			//case GhostState::STRONG: game.DrawRect(vPos + offset, { iTileSize, iTileSize }, color);			break;
			case GhostState::STRONG: game.DrawDecal(vPos + offset, image, { 1.0f, 1.0f }, color);			break;
			case GhostState::WEAK:	 game.DrawDecal(vPos + offset, image, { 1.0f, 1.0f }, olc::DARK_BLUE); break;
			case GhostState::EATEN:  game.DrawDecal(vPos + offset, image, { 1.0f, 1.0f }, olc::Pixel(0, 0, 128, 50)); break;
			}
		}
		void makeWeak()
		{
			fWeakTime = WEAK_TIME;
			currState = GhostState::WEAK;
		}
		void makeEaten()
		{
			vCurrTarget = &vInitPos;
			currState = GhostState::EATEN;
		}
		GhostState getState() const { return currState; }
		void updateTarget(olc::vf2d* vTarget) { vTargetPos = vTarget; vCurrTarget = vTargetPos; }
	private:
		virtual void updateStrong(float fElapsedTime)
		{
			recalculateRoute();
			stepForward(fElapsedTime);
		}
		void updateWeak(float fElapsedTime)
		{
			fWeakTime -= fElapsedTime;
			if (fWeakTime <= 0)
			{
				vCurrTarget = vTargetPos;
				currState = GhostState::STRONG;
			}
		}
		void updateEaten(float fElapsedTime)
		{
			updateWeak(fElapsedTime);
			smartChase();
			stepForward(fElapsedTime);
		}
	protected:
		// update nextDir to chase pacman smartly
		void smartChase()
		{
			// initialize the map
			map = new int[iLevelWidth * iLevelHeight];
			for (int x = 0; x < iLevelWidth; x++)
				for (int y = 0; y < iLevelHeight; y++)
				{
					BOARD_MAP::const_iterator it = board.find(olc::vi2d(x, y));
					if (it != board.end() && it->second->kind == Kind::WALL)
						map[x + y * iLevelWidth] = -1;
					else
						map[x + y * iLevelWidth] = 0;
				}

			// fill the map with numbers
			olc::vi2d vTargetTile = screenToTile(*vCurrTarget);
			olc::vi2d vMyTile = screenToTile(vPos);
			map[vTargetTile.x + iLevelWidth * vTargetTile.y] = 0;
			map[vMyTile.x + iLevelWidth * vMyTile.y] = -2;
			fillMap(vTargetTile.x, vTargetTile.y, 1);
			map[vMyTile.x + iLevelWidth * vMyTile.y] = 0;

			// decide on route
			int min = iLevelWidth * iLevelHeight;
			int minI = 0;
			olc::vi2d possibleTiles[4] = {
				olc::vi2d(vMyTile.x - 1, vMyTile.y), // 0
				olc::vi2d(vMyTile.x + 1, vMyTile.y), // 1
				olc::vi2d(vMyTile.x, vMyTile.y - 1), // 2
				olc::vi2d(vMyTile.x, vMyTile.y + 1)  // 3
			};
			for (int i = 0; i < 4; ++i)
			{
				olc::vi2d tile = possibleTiles[i];
				if (tile.x < 0 || tile.x >= iLevelWidth || tile.y < 0 || tile.y >= iLevelHeight
					|| map[tile.x + tile.y * iLevelWidth] <= 0)
					continue;
				int distance = map[tile.x + iLevelWidth * tile.y];
				if (distance >= 0 && distance < min)
				{
					min = distance;
					minI = i;
				}
			}

			// update nextDir
			switch (minI)
			{
			case 0: nextDir = Dir::LEFT;  break;
			case 1: nextDir = Dir::RIGHT; break;
			case 2: nextDir = Dir::UP;	  break;
			case 3: nextDir = Dir::DOWN;  break;
			}
		}
		void dumbChase1()
		{
			olc::vi2d targetTile = screenToTile(*vCurrTarget);
			olc::vi2d myTile = screenToTile(vPos);
			nextDir = (targetTile.x == myTile.x ? (targetTile.y > myTile.y ? Dir::DOWN : Dir::UP) : (targetTile.x > myTile.x ? Dir::RIGHT : Dir::LEFT));
		}
		void dumbChase2()
		{
			olc::vi2d targetTile = screenToTile(*vCurrTarget);
			olc::vi2d myTile = screenToTile(vPos);
			nextDir = (targetTile.y == myTile.y ? (targetTile.x > myTile.x ? Dir::RIGHT : Dir::LEFT) : (targetTile.y > myTile.y ? Dir::DOWN : Dir::UP));
		}
		void dumbMoving()
		{
			switch (currDir)
			{
			case Dir::UP:
			case Dir::DOWN:
				nextDir = rand() % 2 == 0 ? Dir::LEFT : Dir::RIGHT;
				break;
			case Dir::LEFT:
			case Dir::RIGHT:
				nextDir = rand() % 2 == 0 ? Dir::UP : Dir::DOWN;
				break;
			}
		}
	private:
		void fillMap(int x, int y, int val)
		{
			if ((map[x + y * iLevelWidth] < val && map[x + y * iLevelWidth] != 0))
				return;

			map[x + y * iLevelWidth] = val;

			if (x - 1 >= 0 || !(map[x - 1 + y * iLevelWidth] <= val && map[x - 1 + y * iLevelWidth] != 0))
				fillMap(x - 1, y, val + 1);
			if (x + 1 < iLevelWidth || !(map[x + 1 + y * iLevelWidth] <= val && map[x + 1 + y * iLevelWidth] != 0))
				fillMap(x + 1, y, val + 1);
			if (y - 1 >= 0 || !(map[x + (y - 1) * iLevelWidth] <= val && map[x + (y - 1) * iLevelWidth] != 0))
				fillMap(x, y - 1, val + 1);
			if (y + 1 < iLevelHeight || !(map[x + (y + 1) * iLevelWidth] <= val && map[x + (y + 1) * iLevelWidth] != 0))
				fillMap(x, y + 1, val + 1);
		}
		void printMap()
		{
			std::cout << "printing map" << std::endl;
			for (int x = 0; x < iLevelWidth; x++)
			{
				for (int y = 0; y < iLevelHeight; y++)
				{
					int val = map[y + x * iLevelWidth];
					if (val < 10 && val >= 0)
						std::cout << " ";
					std::cout << val << "|";
				}
				std::cout << std::endl;
			}
		}
	};

	// Yellow ghost: changes directions only when hittin' walls.
	class YellowGhost : public Ghost
	{
	public:
		YellowGhost(olc::PixelGameEngine& game, const olc::vi2d& vPos, const int levelWidth, const int levelHeight, BOARD_MAP& board, bool isOldschool = true, olc::Decal* image = nullptr, olc::vf2d* vTargetPos = nullptr) :
			Ghost(game, vPos, image, olc::YELLOW, Kind::GHOST_Y, levelWidth, levelHeight, board, isOldschool, vTargetPos, Dir::DOWN)
		{}
		void updateStrong(float fElapsedTime) override
		{
			// recalculateRoute();
			stepForward(fElapsedTime);
		}
		void recalculateRoute() override
		{
			dumbMoving();
		}
	};

	// Blue ghost: mostly smart, a little dumb chase
	class BlueGhost : public Ghost
	{
		static const float constexpr TIME_SMART = 6.0f;
		static const float constexpr TIME_DUMB  = 3.0f;

		float fPassedTime;
		bool bSmart;
	public:
		BlueGhost(olc::PixelGameEngine& game, const olc::vi2d& vPos, const int levelWidth, const int levelHeight, BOARD_MAP& board, bool isOldschool = true, olc::Decal* image = nullptr, olc::vf2d* vTargetPos = nullptr) :
			Ghost(game, vPos, image, olc::BLUE, Kind::GHOST_B, levelWidth, levelHeight, board, isOldschool, vTargetPos, Dir::DOWN),
			fPassedTime(0.0f),
			bSmart(true)
		{}
		void updateStrong(float fElapsedTime) override
		{
			fPassedTime += fElapsedTime;
			if ((bSmart && fPassedTime > TIME_SMART) || (!bSmart && fPassedTime > TIME_DUMB))
			{
				bSmart = !bSmart;
				fPassedTime = 0;
			}

			recalculateRoute();
			stepForward(fElapsedTime);
		}
		void recalculateRoute() override
		{
			bSmart ? smartChase() : dumbChase1();
		}
	};

	// Red ghost: mostly dumb moving, a little dumb chasing
	class RedGhost : public Ghost
	{
		static const float constexpr TIME_SMART = 3.0f;
		static const float constexpr TIME_DUMB = 5.0f;

		float fPassedTime;
		bool bSmart;
	public:
		RedGhost(olc::PixelGameEngine& game, const olc::vi2d& vPos, const int levelWidth, const int levelHeight, BOARD_MAP& board, bool isOldschool = true, olc::Decal* image = nullptr, olc::vf2d* vTargetPos = nullptr) :
			Ghost(game, vPos, image, olc::RED, Kind::GHOST_R, levelWidth, levelHeight, board, isOldschool, vTargetPos),
			fPassedTime(0.0f),
			bSmart(true)
		{}
		void updateStrong(float fElapsedTime) override
		{
			fPassedTime += fElapsedTime;
			if ((bSmart && fPassedTime > TIME_SMART) || (!bSmart && fPassedTime > TIME_DUMB))
			{
				bSmart = !bSmart;
				fPassedTime = 0;
			}

			recalculateRoute();
			stepForward(fElapsedTime);
		}
		void recalculateRoute() override
		{
			bSmart ? dumbChase2() : dumbMoving();
		}
	};

	// Green ghost: every 5 seconds changes behaviour randomly
	class GreenGhost : public Ghost
	{
		static enum class Behaviour {
			SMART,
			DUMB1,
			DUMB2,
			DUMB3
		};
		static const float constexpr TIME_OF_BEHAVIOUR = 5.0f;

		float fPassedTime;
		Behaviour behaviour;
	public:
		GreenGhost(olc::PixelGameEngine& game, const olc::vi2d& vPos, const int levelWidth, const int levelHeight, BOARD_MAP& board, bool isOldschool = true, olc::Decal* image = nullptr, olc::vf2d* vTargetPos = nullptr) :
			Ghost(game, vPos, image, olc::GREEN, Kind::GHOST_G, levelWidth, levelHeight, board, isOldschool, vTargetPos),
			fPassedTime(0.0f),
			behaviour(Behaviour::DUMB1)
		{}
		void updateStrong(float fElapsedTime) override
		{
			if ((fPassedTime += fElapsedTime) > TIME_OF_BEHAVIOUR)
			{
				switch (rand() % 6)
				{
				case 1:  behaviour = Behaviour::SMART; break;
				case 2:  behaviour = Behaviour::DUMB1; break;
				case 3:  behaviour = Behaviour::DUMB2; break;
				default: behaviour = Behaviour::DUMB3; break;
				}
				fPassedTime = 0;
			}

			if (behaviour != Behaviour::DUMB3)
				recalculateRoute();
			stepForward(fElapsedTime);
		}
		void recalculateRoute() override
		{
			switch (behaviour)
			{
			case Behaviour::SMART: smartChase(); return;
			case Behaviour::DUMB1: dumbChase1(); return;
			case Behaviour::DUMB2: dumbChase2(); return;
			case Behaviour::DUMB3: dumbMoving(); return;
			}
		}
	};

#pragma endregion

	class Pacman : public MoveableObject
	{
		bool wasRight;
	public:
		Pacman(olc::PixelGameEngine& game, const olc::vi2d& vInitPos, const int iLevelWidth, const int iLevelHeight, bool isOldschool = true, olc::Decal * image = nullptr) :
			MoveableObject(game, Kind::PLAYER, vInitPos, image, isOldschool, int(iPacmanSpeed), iLevelWidth, iLevelHeight),
			wasRight(true)
		{}
		void getInput(olc::PixelGameEngine& game)
		{
			if (game.GetKey(olc::UP).bPressed)		nextDir = Dir::UP;
			if (game.GetKey(olc::DOWN).bPressed)	nextDir = Dir::DOWN;
			if (game.GetKey(olc::LEFT).bPressed)  { nextDir = Dir::LEFT;  wasRight = false; }
			if (game.GetKey(olc::RIGHT).bPressed) { nextDir = Dir::RIGHT; wasRight = true;  }
		}
		void update(float fElapsedTime) { stepForward(fElapsedTime); }
		void collideWithWall() override { stepBack(); }
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			olc::Pixel color = isOldschool ? olc::YELLOW : olc::YELLOW;
			
			olc::vf2d vLeftTop = olc::vi2d(vPos) + offset;
			switch (currDir)
			{
			case Dir::UP:    wasRight ? game.DrawWarpedDecal(image, { vLeftTop + tileToScreen(0,1), vLeftTop + vTile, vLeftTop + tileToScreen(1,0), vLeftTop }) : game.DrawWarpedDecal(image, { vLeftTop + vTile, vLeftTop + tileToScreen(0,1), vLeftTop, vLeftTop + tileToScreen(1,0) });; break;
			case Dir::DOWN:  wasRight ? game.DrawWarpedDecal(image, { vLeftTop + tileToScreen(1,0), vLeftTop, vLeftTop + tileToScreen(0,1), vLeftTop + vTile }) : game.DrawWarpedDecal(image, { vLeftTop, vLeftTop + tileToScreen(1,0), vLeftTop + vTile, vLeftTop + tileToScreen(0,1) }); break;
			case Dir::LEFT:  game.DrawWarpedDecal(image, { vLeftTop + tileToScreen(1,0), vLeftTop + vTile, vLeftTop + tileToScreen(0,1), vLeftTop }); break;
			case Dir::RIGHT: game.DrawWarpedDecal(image, { vLeftTop, vLeftTop + tileToScreen(0,1), vLeftTop + vTile, vLeftTop + tileToScreen(1,0) }); break;
			}
			// yes I know it's a lame solution, It's 4am don't yell at me

			//game.DrawWarpedDecal(image, { vLeftTop, vLeftTop + tileToScreen(0,1), vLeftTop + vTile, vLeftTop + tileToScreen(1,0) }); // right
			//game.DrawWarpedDecal(image, { vLeftTop + tileToScreen(0,1), vLeftTop + vTile, vLeftTop + tileToScreen(1,0), vLeftTop }); // up-right
			//game.DrawWarpedDecal(image, { vLeftTop + tileToScreen(1,0), vLeftTop, vLeftTop + tileToScreen(0,1), vLeftTop + vTile }); // down-right
			//game.DrawWarpedDecal(image, { vLeftTop, vLeftTop + tileToScreen(1,0), vLeftTop + vTile, vLeftTop + tileToScreen(0,1) }); // down-left
			//game.DrawWarpedDecal(image, { vLeftTop + tileToScreen(1,0), vLeftTop + vTile, vLeftTop + tileToScreen(0,1), vLeftTop }); // left
			//game.DrawWarpedDecal(image, { vLeftTop + vTile, vLeftTop + tileToScreen(0,1), vLeftTop, vLeftTop + tileToScreen(1,0) }); // up-left
		}
	};

	struct Level {
		olc::PixelGameEngine& game;
		olc::vi2d vPos; // in screen space
		std::vector<olc::Decal*>& decals;
		BOARD_MAP board;
		bool isOldschool;
		std::vector<std::shared_ptr<Ghost>> ghosts;
		std::vector<std::shared_ptr<PowerUp>> powerUps;
		std::shared_ptr<Pacman> player;
		int width;
		int height;
		int iDots;
		Level(olc::PixelGameEngine& game, std::vector<olc::Decal*>& decals, const olc::vi2d& pos = { 0, 0 }, bool isOldschool = true, const int width = DEFAULT_LEVEL_WIDTH, const int height = DEFAULT_LEVEL_HEIGHT) :
			game(game),
			vPos(pos),
			decals(decals),
			board(MAKE_BOARD),
			isOldschool(isOldschool),
			player(nullptr),
			width(width),
			height(height),
			iDots(0)
		{}
		Level(olc::PixelGameEngine& game, std::vector<olc::Decal*>& decals, LevelData data, bool isOldschool = true, const olc::vi2d& pos = { 0, 0 }) :
			game(game),
			vPos(pos),
			decals(decals),
			board(MAKE_BOARD),
			isOldschool(isOldschool),
			player(nullptr),
			iDots(0)
		{
			width = data.width;
			height = data.height;

			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++)
					addAt({ x, y }, charToKind(data.data[y * width + x]));

			for (auto ghost : ghosts)
				ghost->updateTarget(player->getPosPtr());

			// deternime walls (up / down / left / right)
			for (BOARD_MAP::iterator it = board.begin(); it != board.end(); ++it)
			{
				if (Wall* wall = dynamic_cast<Wall*>(it->second.get()))
				{
					int x = it->first.x;
					int y = it->first.y;
					auto cond = [&](auto i) { return i >= 0 && i < width* height&& data.data[i] == SYMBOL_WALL; };

					int location = (y - 1) * width + x;
					if (y != 0          && cond(location))  wall->walls &= 0b0111;	location = (y + 1) * width + x;
					if (y != height - 1 && cond(location))	wall->walls &= 0b1011;	location = y * width + (x - 1);
					if (x != 0          && cond(location))  wall->walls &= 0b1101;	location = y * width + (x + 1);
					if (x != width - 1  && cond(location))  wall->walls &= 0b1110;
				}
			}
		}
		void incrementWidth(const int value)
		{
			width += value;
			if (width < 0) width = 0;
			if (value < 0)
				for (int x = width; x < width - value; x++)
					for (int y = 0; y < height; y++)
						eraseAt({ x, y });
		}
		void incrementHeight(const int value)
		{
			height += value;
			if (height < 0) height = 0;
			if (value  < 0)
				for (int x = 0; x < width; x++)
					for (int y = height; y < height - value; y++)
						eraseAt({ x, y });
		}
	public:
		// pos in tile space
		void addAt(const olc::vi2d& pos, Kind kind)
		{
			std::shared_ptr<Ghost> ghost(nullptr);
			switch (kind)
			{
			case Kind::PLAYER:
				player = std::shared_ptr<Pacman>(new Pacman(game, tileToScreen(pos), width, height, isOldschool, decals[isOldschool ? SPRITE_PACMAN : SPRITE_MINI_PACMAN]));
				board.emplace(std::make_pair(pos, player));
				break;
			case Kind::GHOST_B:  ghost = MAKE_GHOST(BlueGhost);   break;
			case Kind::GHOST_R:  ghost = MAKE_GHOST(RedGhost);    break;
			case Kind::GHOST_Y:  ghost = MAKE_GHOST(YellowGhost); break;
			case Kind::GHOST_G:  ghost = MAKE_GHOST(GreenGhost);  break;
			case Kind::DOT:      board.emplace(MAKE_TILE(game, Dot, nullptr));  ++iDots;   break;
			case Kind::WALL:     board.emplace(MAKE_TILE(game, Wall, decals[SPRITE_WALL]));    break;  break;
			case Kind::POWER_UP: {std::shared_ptr<PowerUp> pu(new PowerUp(game, tileToScreen(pos), isOldschool)); board.emplace(std::make_pair(pos, pu)); powerUps.push_back(pu);  break; }
			}

			if (ghost != nullptr)
			{
				board.emplace(std::make_pair(pos, ghost));
				ghosts.push_back(ghost);
			}
		}
		// pos in tile space
		void eraseAt(olc::vi2d pos)
		{
			board.erase(pos - screenToTile(vPos));
			std::remove_if(ghosts.begin(), ghosts.end(), [&](auto ghost) {return screenToTile(ghost->getPos() - vPos) == pos; });
			if (player != nullptr && screenToTile(player->getPos() - vPos) == pos)
				player.reset();
		}
		void draw() const
		{
			std::for_each(board.begin(), board.end(), [&](auto pair) { pair.second->draw(vPos); });
			std::for_each(ghosts.begin(), ghosts.end(), [&](auto ghost) { ghost->draw(vPos); });
			if (player != nullptr)
				player->draw(vPos);
		}
		std::string exportLevel() const
		{
			std::string str = std::to_string(width) + "x" + std::to_string(height) + "x";

			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++)
				{
					olc::vi2d pos(x, y); // idk why but the matrix is transposed
					if (player != nullptr && pos == screenToTile(player->vInitPos))
					{
						str.push_back(player->getSymbol());
						continue;
					}
					auto it = std::find_if(ghosts.begin(), ghosts.end(), [&](auto ghost) { return pos == screenToTile(ghost->vInitPos); });
					if (it != ghosts.end())
					{
						str.push_back((*it)->getSymbol());
						continue;
					}

					BOARD_MAP::const_iterator it2 = board.find(pos);
					if (it2 == board.end())
						str.push_back(' ');
					else
						str.push_back(it2->second->getSymbol());
				}
			std::cout << str << std::endl;
			return str;
		}
	};

#pragma region UI

	struct UI
	{
		olc::PixelGameEngine& game;
		olc::vf2d vPos;
		UI(olc::PixelGameEngine& game, olc::vf2d pos) : game(game), vPos(pos) {}
		virtual void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const = 0;
	};

	struct Title : public UI
	{
		std::string text;
		olc::Pixel color;
		Title(olc::PixelGameEngine& game, olc::vf2d pos, std::string text, olc::Pixel color = olc::WHITE) :
			UI(game, pos),
			text(text),
			color(color)
		{}
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			game.DrawString(vPos + offset + olc::vi2d(2, 1),   text, olc::CYAN,    2);
			game.DrawString(vPos + offset + olc::vi2d(-2, -1), text, olc::MAGENTA, 2);
			game.DrawString(vPos + offset,					   text, color,        2);
		}
	};

	struct TextBox : public UI
	{
		std::string text;
		int longestLine;
		int numOfLines;
		TextBox(olc::PixelGameEngine& game, olc::vf2d pos, std::string text) :
			UI(game, pos),
			text(text),
			longestLine(0),
			numOfLines(1)
		{
			int currLineLength = 0;
			for (char c : text)
			{
				if (c == '\n')
				{
					numOfLines++;
					if (currLineLength > longestLine)
						longestLine = currLineLength;
					currLineLength = 0;
				}
				else
					currLineLength++;
			}
			if (currLineLength > longestLine)
				longestLine = currLineLength;
		}
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			olc::vi2d rectSize(tileToScreen(longestLine + 1, numOfLines + 1));
			game.DrawRect(vPos + offset - olc::vi2d(4, 4), rectSize, olc::Pixel(0, 222, 222));
			game.FillRectDecal(vPos + offset - olc::vi2d(4, 4), rectSize, olc::Pixel(200, 222, 200, 50));
			game.DrawString(vPos + offset, text, olc::WHITE, 1);
		}
	};

	struct Button : public UI
	{
		olc::vf2d vSize;
		std::string text;
		int* iText;
		std::function<void()> fncOnClick;
		bool isClickable;
		Button(olc::PixelGameEngine& game, olc::vf2d pos, std::string text, std::function<void()> fncOnClick = [] {}, bool isClickable = true) :
			UI(game, pos),
			vSize(tileToScreen(text.length(), 1) + vHalfTile),
			text(text),
			iText(nullptr),
			fncOnClick(fncOnClick),
			isClickable(isClickable)
		{}
		Button(olc::PixelGameEngine& game, olc::vf2d pos, int* text, std::function<void()> fncOnClick = [] {}, bool isClickable = true) :
			UI(game, pos),
			vSize(tileToScreen(std::to_string(*text).length(), 1) + vHalfTile),
			text(std::to_string(*text)),
			iText(text),
			fncOnClick(fncOnClick),
			isClickable(isClickable)
		{}
		bool isClicked() { return isClickable && game.GetMouse(0).bPressed && isPointInRect(game.GetMousePos(), vPos, vSize); }
		void update()
		{
			if (isClicked())
				fncOnClick();
		}
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			game.FillRect(vPos, vSize, olc::GREY);
			game.DrawRect(vPos, vSize, olc::Pixel(200, 100, 0));

			game.DrawString(olc::vi2d(vPos.x + iTileSize / 2 - 1, vPos.y + vSize.y / 2 - 3), iText == nullptr ? text : std::to_string(*iText), olc::BLACK);
			if (isClickable && isPointInRect(game.GetMousePos(), vPos, vSize))
				game.FillRectDecal(vPos, vSize, olc::Pixel(255, 170, 0, 80));
		}
	};

	class Switch : public UI
	{
		olc::vf2d vSize;
		Button btn;
		std::string textOff;
		std::string textOn;
		std::function<void()> fncOnClick;
		bool isOn;

	public:
		Switch(olc::PixelGameEngine& game, olc::vf2d pos, std::string textOff, std::string textOn, std::function<void()> fncOnClick = [] {}, bool isOn = true) :
			UI(game, pos),
			vSize(tileToScreen(2, 1) + vHalfTile),
			btn(game, vPos + tileToScreen(3, 0), isOn ? textOn : textOff, [] {}, false),
			textOff(textOff),
			textOn(textOn),
			fncOnClick(fncOnClick),
			isOn(isOn)
		{}
		void update()
		{
			if (game.GetMouse(0).bPressed && isPointInRect(game.GetMousePos(), vPos, vSize))
			{
				btn.text = (isOn = !isOn) ? textOn : textOff;
				fncOnClick();
			}
		}
		void draw(const olc::vf2d& offset = { 0.0f, 0.0f }) const override
		{
			// switch
			game.FillRect(vPos, vSize, isOn ? olc::GREEN : olc::RED);
			game.DrawRect(vPos, vSize, olc::Pixel(200, 100, 0));
			game.FillRect(vPos + olc::vi2d(isOn ? iTileSize * 1.5f : 0, 0), olc::vi2d(iTileSize, vSize.y), olc::WHITE);
			game.DrawRect(vPos + olc::vi2d(isOn ? iTileSize * 1.5f : 0, 0), olc::vi2d(iTileSize, vSize.y), olc::DARK_GREY);

			// text
			btn.draw();
		}
	};

#pragma endregion

}

#endif