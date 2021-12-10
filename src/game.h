#ifndef game_h
#define game_h

#include <shared_mutex>
#include "texture.h"
#include <string>
#include <map>
#include <list>
#include <thread>
#include <atomic>

/**
 * Converts a pixel location into a tile location
 */
void to_tile (int x, int y, int* tilex, int* tiley);


/**
 * Entity format with a sprite, a location, a rotation, and a shared mutex for multithreading
 */
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
/**
 * Basically a bunch of globals and lists of assets/entities
 */
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

/**
 * Moves a ghost entity according to its correct AI
 */
void ghost_ai (entity* ghost);

#endif
