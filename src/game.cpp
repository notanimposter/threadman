
#include "texture.h"
#include "game.h"
#include <SDL2/SDL.h>

GameInfo game;

void specific_ghost_ai (entity* ghost, int* x, int* y) {
	if (game.ghost_mode == 2) { // the same for every ghost
		*x = std::rand() % 28;
		*y = std::rand() % 36;
	} else if (ghost == game.blinky) {
		if (game.ghost_mode == 0) {
			*x = 2;
			*y = 0;
		} else {
			game.man->lock.lock_shared ();
			to_tile (game.man->get_center_x (), game.man->get_center_y (), x, y);
			game.man->lock.unlock_shared ();
		}
	} else if (ghost == game.pinky) {
		if (game.ghost_mode == 0) {
			*x = 25;
			*y = 0;
		} else {
			game.man->lock.lock_shared ();
			int tempx, tempy;
			to_tile (game.man->get_center_x (), game.man->get_center_y (), &tempx, &tempy);
			int rotation = game.man->facing;
			int xmove = ((rotation + 1) % 2) * -1 * (rotation - 1);
			int ymove = (rotation % 2) * (rotation - 2);
			tempx += xmove * 4;
			tempy += ymove * 4;
			if (rotation == 1) tempx -= 4; // overflow emulation lol
			game.man->lock.unlock_shared ();
			*x = tempx;
			*y = tempy;
		}
	} else if (ghost == game.inky) {
		if (game.ghost_mode == 0) {
			*x = 27;
			*y = 35;
		} else {
			game.man->lock.lock_shared ();
			int tempx, tempy;
			to_tile (game.man->get_center_x (), game.man->get_center_y (), &tempx, &tempy);
			int rotation = game.man->facing;
			int xmove = ((rotation + 1) % 2) * -1 * (rotation - 1);
			int ymove = (rotation % 2) * (rotation - 2);
			tempx += xmove * 2;
			tempy += ymove * 2;
			if (rotation == 1) tempx -= 2; // overflow emulation lol
			game.man->lock.unlock_shared ();
			game.blinky->lock.lock_shared ();
			int bx, by;
			to_tile (game.blinky->get_center_x (), game.blinky->get_center_y (), &bx, &by);
			tempx += tempx - bx;
			tempy += tempy - by;
			game.blinky->lock.unlock_shared ();
			*x = tempx;
			*y = tempy;
		}
	} else if (ghost == game.clyde) {
		if (game.ghost_mode == 0) {
			*x = 0;
			*y = 35;
		} else {
			game.man->lock.lock_shared ();
			int tempx, tempy;
			to_tile (game.man->get_center_x (), game.man->get_center_y (), &tempx, &tempy);
			game.man->lock.unlock_shared ();
			ghost->lock.lock_shared ();
			int gx, gy;
			to_tile (ghost->get_center_x (), ghost->get_center_y (), &gx, &gy);
			ghost->lock.unlock_shared ();
			if (std::sqrt((gx-tempx)*(gx-tempx)+(gy-tempy)*(gy-tempy)) > 8) {
				*x = tempx;
				*y = tempy;
			} else {
				*x = 0;
				*y = 35;
			}
		}
	}
}

void to_tile (int x, int y, int* tilex, int* tiley) {
	*tilex = (int)(x/ 224.0 * 28);
	*tiley = (int)(y/ 288.0 * 36);
}

void ghost_ai (entity* ghost) {
	while (!game.should_quit) {
		int target_tile_x, target_tile_y;
		specific_ghost_ai (ghost, &target_tile_x, &target_tile_y);
		ghost->lock.lock_shared ();
		int ghost_tile_x, ghost_tile_y;
		int ghost_center_x = ghost->get_center_x ();
		int ghost_center_y = ghost->get_center_y ();
		to_tile (ghost_center_x, ghost_center_y, &ghost_tile_x, &ghost_tile_y);
		
		int rotation, xmove, ymove;
		int r, test_xmove, test_ymove, x, y;
		float mind = 999999; // big number
		for (r=0;r<4;r++) {
			if (r == ghost->facing+2 || r == ghost->facing-2) continue; // ignore the opposite direction from the one we're facing so we can't turn around
			test_xmove = ((r + 1) % 2) * -1 * (r - 1);
			test_ymove = (r % 2) * (r - 2);
			x = (target_tile_x - (ghost_tile_x+test_xmove));
			x *= x;
			y = (target_tile_y - (ghost_tile_y+test_ymove));
			y *= y;
			if (x+y < mind && game.maps["walkable"]->lookup (ghost_tile_x + test_xmove, ghost_tile_y+test_ymove).a > 127) {
				xmove = test_xmove;
				ymove = test_ymove;
				rotation = r;
				mind = x+y;
			}
		}
		ghost->lock.unlock_shared ();
		ghost->lock.lock ();
		if (ghost_center_x % 8 == 4 && ghost_center_y % 8 == 4 && (ghost_tile_x+xmove > 27 || ghost_tile_x+xmove < 0 || game.maps["walkable"]->lookup (ghost_tile_x + xmove, ghost_tile_y+ymove).a > 127)) {
			ghost->facing = rotation;
		}
		rotation = ghost->facing;
		xmove = ((rotation + 1) % 2) * -1 * (rotation - 1);
		ymove = (rotation % 2) * (rotation - 2);
		if ( ghost_center_x % 8 != 4 || ghost_center_y % 8 != 4 || ghost_tile_x+xmove > 27 || ghost_tile_x+xmove < 0 || game.maps["walkable"]->lookup (ghost_tile_x + xmove, ghost_tile_y+ymove).a > 127) {
			ghost->x += xmove;
			ghost->y += ymove;
			if (ghost->x < 0) ghost->x += 224;
			if (ghost->x > 224) ghost->x -= 224;
		}
		ghost->lock.unlock ();
		SDL_Delay (17); // make the ghosts slightly slower than the man TODO add delta time
	}
}
