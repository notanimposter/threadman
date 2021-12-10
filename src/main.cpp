#include <thread>
#include <string>
#include <experimental/filesystem>
#include <SDL2/SDL.h>

#include "ts.h"
#include "texture.h"
#include "game.h"

void load () {
	// set up some globals
	game.should_quit = false;
	game.dots = 0;
	game.ghost_mode = 0;
	game.ghost_phase_timer = SDL_GetTicks ();
	
	// set up the window
	game.window = SDL_CreateWindow ("Thread-Man", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
	game.renderer = SDL_CreateRenderer(game.window, -1, 0);
	SDL_RenderSetLogicalSize (game.renderer, 224, 288);
	
	// load all the game assets
	for (auto const& f : std::experimental::filesystem::directory_iterator ("assets/textures")) {
		if (f.path ().extension () == ".bmp") {
			ts::printf ("Loaded texture: %s\n", f.path ().stem ().c_str ());
			game.sprites[f.path ().stem ()] = new texture (game.renderer, f.path ().relative_path ().c_str ());
		}
	}
	for (auto const& f : std::experimental::filesystem::directory_iterator ("assets/maps")) {
		if (f.path ().extension () == ".bmp") {
			ts::printf ("Loaded map: %s\n", f.path ().stem ().c_str ());
			game.maps[f.path ().stem ()] = new map (f.path ().relative_path ().c_str ());
		}
	}
	// make the dots
	for (int x=0;x<28;x++) {
		for (int y=0;y<36;y++) {
			if (game.maps["dots"]->lookup (x,y).a > 127) game.entities.push_back (new entity {x*8, y*8, game.sprites["dot"]});
			if (game.maps["bigdots"]->lookup (x,y).a > 127) game.entities.push_back (new entity {x*8, y*8, game.sprites["bigdot"]});
		}
	}
	// make pac-man
	game.man = new entity {13*8-3, 26*8-3, game.sprites["man"], 0};
	game.entities.push_back (game.man);
	
	// make the ghosts, start their AI threads
	game.blinky = new entity {15*8-3, 20*8-3, game.sprites["blinky"], 0};
	game.entities.push_back (game.blinky);
	game.blinky_thread = std::thread (ghost_ai, game.blinky);
	
	game.pinky = new entity {13*8-3, 20*8-3, game.sprites["pinky"], 0};
	game.entities.push_back (game.pinky);
	game.pinky_thread = std::thread (ghost_ai, game.pinky);
	
	game.inky = new entity {11*8-3, 20*8-3, game.sprites["inky"], 0};
	game.entities.push_back (game.inky);
	game.inky_thread = std::thread (ghost_ai, game.inky);
	
	game.clyde = new entity {9*8-3, 20*8-3, game.sprites["clyde"], 0};
	game.entities.push_back (game.clyde);
	game.clyde_thread = std::thread (ghost_ai, game.clyde);
}
void unload () {
	game.should_quit = true; // this will cause the threads to exit eventually
	
	// should note that this step doesn't destroy the ghost entities, just stops their AI.
	// so inky's AI won't complain that blinky doesn't exist anymore. blinky still exists, just has no AI
	game.blinky_thread.join ();
	game.pinky_thread.join ();
	game.inky_thread.join ();
	game.clyde_thread.join ();
	// now we delete the entities
	for (auto entity : game.entities) {
		delete entity;
	}
	// free up the assets
	for (auto const& [key, val] : game.sprites) {
		delete val;
	}
	for (auto const& [key, val] : game.maps) {
		delete val;
	}
}
void keypressed (SDL_KeyboardEvent event) {
	// rotation number represents # of quarter turns ccw starting from facing right
	// not sure exactly why I went with this format but it works ok
	switch (event.keysym.sym) {
		case SDLK_LEFT:
			game.want_to_turn = 2;
			break;
		case SDLK_RIGHT:
			game.want_to_turn = 0;
			break;
		case SDLK_UP:
			game.want_to_turn = 1;
			break;
		case SDLK_DOWN:
			game.want_to_turn = 3;
			break;
	}
}

void update () {
	// ghost phase timer
	// technically this isn't quite right because the ghosts are supposed to eventually settle down in chase mode, but
	// this has them always switch back to scatter after 20s of chase. would be easy to stick the numbers into an array to
	// mimic that game-stage type system, but initially I had the dots just come back at the end of the game so it made more
	// sense to loop it indefinitely at the time. also, the frightened mode is supposed to pause the timer rather than reset it
	// technically, but I'll be honest I didn't feel like keeping two separate timers. Originally I was going to use SDL_AddTimer
	// for it, which would have made this a bit cleaner, but I couldn't get it to work. The timing seemed really inconsistent for
	// whatever reason.
	Uint32 time = SDL_GetTicks ();
	if (game.ghost_mode == 0 && time - game.ghost_phase_timer > 7000) {
		game.ghost_phase_timer = time;
		game.ghost_mode = 1;
	} else if (game.ghost_mode == 1 && time - game.ghost_phase_timer > 20000) {
		game.ghost_phase_timer = time;
		game.ghost_mode = 0;
	} else if (game.ghost_mode == 2 && time - game.ghost_phase_timer > 6000) {
		game.ghost_phase_timer = time;
		game.ghost_mode = 0;
	}
	game.man->lock.lock_shared ();
	// handle collisions
	int man_tile_x, man_tile_y;
	int man_center_x = game.man->get_center_x ();
	int man_center_y = game.man->get_center_y ();
	
	to_tile (man_center_x, man_center_y, &man_tile_x, &man_tile_y);
	for (auto* &target : game.entities) { // check every entity
		if (target == game.man) continue; // we don't need to check the man against himself
		target->lock.lock ();
		int target_tile_x, target_tile_y;
		to_tile (target->get_center_x (), target->get_center_y (), &target_tile_x, &target_tile_y);
		if (man_tile_x == target_tile_x && man_tile_y == target_tile_y) {
			if (target->sprite == game.sprites["dot"]) {
				delete target;
				target = NULL;
				game.dots++;
			} else if (target->sprite == game.sprites["bigdot"]) {
				delete target;
				target = NULL;
				game.ghost_mode = 2;
				game.ghost_phase_timer = SDL_GetTicks ();
			} else if (target == game.blinky || target == game.pinky || target == game.inky || target == game.clyde) {
				if (game.ghost_mode == 2) { // kill the ghost
					target->x = 13*8-3;
					target->y = 14*8-3;
				} else { // kill pac-man
					game.man->lock.unlock_shared ();
					game.man->lock.lock ();
					game.man->x = 13*8-3;
					game.man->y = 26*8-3;
					game.man->lock.unlock ();
					game.man->lock.lock_shared ();
				}
			}
		}
		if (target != NULL) target->lock.unlock ();
	}
	game.entities.remove (NULL); // remove all the null values from the array

	game.man->lock.unlock_shared ();
	game.man->lock.lock ();

	// man rotation
	int rotation = game.want_to_turn;
	int xmove = ((rotation + 1) % 2) * -1 * (rotation - 1);
	int ymove = (rotation % 2) * (rotation - 2);
	
	if (man_center_x % 8 == 4 && man_center_y % 8 == 4 && (man_tile_x+xmove > 27 || man_tile_x+xmove < 0 || game.maps["walkable"]->lookup (man_tile_x + xmove, man_tile_y+ymove).a > 127)) {
		game.man->facing = game.want_to_turn;
	}
	
	// man movement
	rotation = game.man->facing;
	xmove = ((rotation + 1) % 2) * -1 * (rotation - 1);
	ymove = (rotation % 2) * (rotation - 2);
	
	if ( man_center_x % 8 != 4 || man_center_y % 8 != 4 || man_tile_x+xmove > 27 || man_tile_x+xmove < 0 || game.maps["walkable"]->lookup (man_tile_x + xmove, man_tile_y+ymove).a > 127) {
		game.man->x += xmove;
		game.man->y += ymove;
		if (game.man->x < 0) game.man->x += 224;
		if (game.man->x > 224) game.man->x -= 224;
	}
	game.man->lock.unlock ();
}
void draw () {
	SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
	SDL_RenderClear(game.renderer);
	// draw the map
	game.sprites["map"]->draw (SDL_Rect{0,0,224,288});
	for (auto entity : game.entities) {
		entity->lock.lock_shared ();
		if ((game.ghost_mode == 2) && (entity == game.blinky || entity == game.pinky || entity == game.inky || entity == game.clyde)) {
			game.sprites["fear"]->draw (SDL_Rect{entity->x, entity->y, entity->sprite->width, entity->sprite->height});
		} else
			entity->sprite->draw (SDL_Rect{entity->x, entity->y, entity->sprite->width, entity->sprite->height});
		entity->lock.unlock_shared ();
	}
	// win condition
	if (game.dots >= 240) {
		texture* win = game.sprites["win"];
		win->draw (SDL_Rect{84,124,win->width * 2, win->height * 2});
	}
	// draw to the screen
	SDL_RenderPresent (game.renderer);
}

// main loop
int main (int argc, char** argv) {
	SDL_Init (SDL_INIT_EVERYTHING);
	ts::printf ("Initialized SDL\n");
	load ();
	ts::printf ("Game loaded\n");
	bool done = false;
	SDL_Event event;
	while (!done) {
		// first we handle any important events
		while (SDL_PollEvent (&event)>0) {
			switch (event.type) {
				case SDL_QUIT:
					ts::printf ("Quit signal received\n");
					done = true;
					break;
				case SDL_KEYDOWN:
					keypressed (event.key);
					break;
			}
		}
		// now update the game
		update ();
		// now draw the screen
		draw ();
		// TODO correct timings
		// was going to add time deltas and try to chase 60fps but frankly this is working fine
		// I'm just going to leave it
		SDL_Delay (16);
	}
	unload ();
	ts::printf ("Game unloaded\n");
	SDL_DestroyRenderer(game.renderer);
	SDL_DestroyWindow(game.window);
	SDL_Quit ();
	ts::printf ("Quit SDL\n");
	return 0;
}
