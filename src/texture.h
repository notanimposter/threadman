#ifndef texture_h
#define texture_h

#include <SDL2/SDL.h>
#include "ts.h"

struct rgba {
	Uint8 r, g, b, a;
};

/**
 * Represents an info map that doesn't get rendered and can be indexed
 */
class map {
	private:
		SDL_Surface* surface;
	public:
		map (const char* filename) {
			surface = SDL_LoadBMP (filename);
		}
		~map () {
			SDL_FreeSurface (surface);
		}
		
		rgba lookup (int x, int y) {
			Uint32* pixel = (Uint32*) ((Uint8*) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
			rgba color;
			SDL_GetRGBA (*pixel, surface->format, &color.r, &color.g, &color.b, &color.a);
			return color;
		}
};
/**
 * Represents a texture to be rendered
 */
class texture {
	private:
		SDL_Texture* tex;
		SDL_Renderer* renderer;
		SDL_PixelFormat* format;
	public:
		int width;
		int height;
		texture (SDL_Renderer* render, const char* filename) {
			renderer = render;
			SDL_Surface* surface = SDL_LoadBMP (filename);
			width = surface->w;
			height = surface->h;
			tex = SDL_CreateTextureFromSurface (renderer, surface);
			SDL_FreeSurface (surface);
		}
		~texture () {
			SDL_DestroyTexture (tex);
		}
		void draw (SDL_Rect const& bounds) {
			SDL_RenderCopy (renderer, tex, NULL, &bounds);
		}
};
#endif
