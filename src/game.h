#ifndef game_h
#define game_h

#include <shared_mutex>
#include "texture.h"
#include <string>
#include <map>
#include <list>
#include <thread>
#include <atomic>

void to_tile (int x, int y, int* tilex, int* tiley);

struct entity {
	int x, y;
	texture* sprite;
	int facing;
	std::shared_mutex lock;
	int get_center_x () {
		return (x + sprite->width / 2);
	}
	int get_center_y () {
		return (y + sprite->height / 2);
	}
};

struct GameInfo {
	SDL_Window* window;
	SDL_Renderer* renderer;
	std::map<std::string, texture*> sprites;
	std::map<std::string, map*> maps;
	std::list<entity*> entities;
	entity* man;
	int want_to_turn;
	std::atomic<bool> should_quit;
	std::atomic<int> ghost_mode;
	entity *blinky, *pinky, *inky, *clyde;
	std::thread blinky_thread, pinky_thread, inky_thread, clyde_thread;
	int dots;
	SDL_TimerID ghost_phase_timer;
};

extern GameInfo game;

void ghost_ai (entity* ghost);

#endif
