#include <stdio.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0x34c3eb

int main() 
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Liquid Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	SDL_Surface* surface = SDL_GetWindowSurface(window);

	SDL_Rect rectangle = (SDL_Rect){50, 50, 100, 50}; 
	SDL_FillRect(surface, &rectangle, COLOR_WHITE);

	SDL_Rect blue_rectangle = (SDL_Rect){150, 50, 100, 50}; 
	SDL_FillRect(surface, &blue_rectangle, COLOR_BLUE);
	SDL_UpdateWindowSurface(window);

	SDL_Delay(3000);
}