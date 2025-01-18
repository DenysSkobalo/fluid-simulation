#include <stdio.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0x34c3eb
#define COLOR_GRAY 0x1f1f1f1f
#define CELL_SIZE 20
#define LINE_WIDTH 2
#define COLUMNS SCREEN_WIDTH/CELL_SIZE
#define ROWS SCREEN_HEIGHT/CELL_SIZE
#define WATER_TYPE 0
#define SOLID_TYPE 1

struct Cell
{
	int type;
	double fill_level;
	int x;
	int y;
};

void draw_cell(SDL_Surface* surface, struct Cell cell)
{
	int pixel_x = cell.x*CELL_SIZE;
	int pixel_y = cell.y*CELL_SIZE;
	SDL_Rect cell_rect = (SDL_Rect){pixel_x, pixel_y, CELL_SIZE, CELL_SIZE};
	Uint32 color = COLOR_BLACK;
	// BG - backgorund color
	SDL_FillRect(surface, &cell_rect, COLOR_BLACK);
	// water fill level
	if (cell.type == WATER_TYPE)
  {
		int water_height = cell.fill_level > 1 ? CELL_SIZE : cell.fill_level * CELL_SIZE;
		int empty_height = CELL_SIZE - water_height;
	  SDL_Rect water_rect = (SDL_Rect){pixel_x, pixel_y + empty_height, CELL_SIZE, water_height};
	  SDL_FillRect(surface, &water_rect, COLOR_BLUE);
	}
	// solid blocks
	if (cell.type == SOLID_TYPE)
		{
			SDL_FillRect(surface, &cell_rect, COLOR_WHITE);
		}
}

void draw_environment(SDL_Surface* surface , struct Cell environment[ROWS*COLUMNS])
{
	for (int i=0; i<ROWS*COLUMNS; i++)
	{
		draw_cell(surface, environment[i]);
	}
}

void draw_grid(SDL_Surface* surface)
{
	for (int i=0; i<COLUMNS; i++)
	{
		SDL_Rect column = (SDL_Rect){i*CELL_SIZE, 0, LINE_WIDTH, SCREEN_HEIGHT};
		SDL_FillRect(surface, &column, COLOR_GRAY);
	}
	for (int j=0; j<ROWS; j++)
	{
		SDL_Rect row = (SDL_Rect){0, j*CELL_SIZE, SCREEN_WIDTH, LINE_WIDTH};
		SDL_FillRect(surface, &row, COLOR_GRAY);
	}
}

void initialize_environment(struct Cell environment[ROWS * COLUMNS])
{
	for (int i=0; i<ROWS; i++) 
	{ 
		for (int j=0; j<COLUMNS; j++)
		{
			environment[j + COLUMNS*i] = (struct Cell){WATER_TYPE, 0, j, i};
		}
	}
}

// Rule 1: Water flows down
void simultaion_phase_rule1(struct Cell environment[ROWS*COLUMNS])
{
	struct Cell environment_next[ROWS*COLUMNS];
	for (int i=0; i<ROWS*COLUMNS; i++)
		environment_next[i] = environment[i];

	for (int i=0; i<ROWS; i++)
		{
			for (int j=0; j<COLUMNS; j++)
			{
				struct Cell source_cell = environment[j + COLUMNS*i];
				int rule_1_applied = 0;
				if (source_cell.type == WATER_TYPE && i<ROWS-1)
				{
					struct Cell destination_cell = environment[j + COLUMNS*(i+1)];
					// how much liquid can flow intro the destination cell ? 
					if (destination_cell.fill_level < source_cell.fill_level)
					{
						double free_space_destination = 1 - destination_cell.fill_level;
						if (free_space_destination >= source_cell.fill_level)
						{
							environment_next[j + COLUMNS*i].fill_level = 0;
							environment_next[j + COLUMNS*(i+1)].fill_level += source_cell.fill_level;
						} 
						else 
						{
							environment_next[j + COLUMNS*i].fill_level -= free_space_destination;
							environment_next[j + COLUMNS*(i+1)].fill_level = 1;
						}
					}
				}
			}
		}
		for (int i=0; i<ROWS*COLUMNS; i++)
			environment[i] = environment_next[i];
}

// Rule 2: Water flows to the right
void simultaion_phase_rule2(struct Cell environment[ROWS*COLUMNS])
{
	struct Cell environment_next[ROWS*COLUMNS];
	for (int i=0; i<ROWS*COLUMNS; i++)
		environment_next[i] = environment[i];

	for (int i=0; i<ROWS; i++)
		{
			for (int j=0; j<COLUMNS; j++)
			{
				// check fi cell below is either full or solid or bottom border
				struct Cell source_cell = environment[j + COLUMNS*i];
				if (i+1 == ROWS || environment[j + COLUMNS*(i+1)].fill_level >= environment[j + COLUMNS* i].fill_level || environment[j + COLUMNS*(i+1)].type == SOLID_TYPE)
				{
					if (source_cell.type == WATER_TYPE && j>0)
					{
						// how much liquid can flow to the left ? 
						struct Cell destination_cell = environment[(j-1) + COLUMNS*i];
						if (destination_cell.type == WATER_TYPE && destination_cell.fill_level < source_cell.fill_level)
						{
							double delta_fill = source_cell.fill_level - destination_cell.fill_level;
							environment_next[j + COLUMNS*i].fill_level -= delta_fill / 3;
							environment_next[(j-1) + COLUMNS*i].fill_level += delta_fill / 3;
						}
					}
					if (source_cell.type == WATER_TYPE && j<COLUMNS-1)
					{
						// how much liquid can flow to the right ?
						struct Cell destination_cell = environment[(j+1) + COLUMNS*i];
						if (destination_cell.fill_level < source_cell.fill_level)
						{
							double delta_fill = source_cell.fill_level - destination_cell.fill_level;
							environment_next[j + COLUMNS*i].fill_level -= delta_fill / 3;
							environment_next[(j+1) + COLUMNS*i].fill_level += delta_fill / 3;
						}
					}
				}
			}
		}
		for (int i=0; i<ROWS*COLUMNS; i++)
			environment[i] = environment_next[i];
}

// Rule 3: Pressurized cells can release fluid upwards
void simultaion_phase_rule3(struct Cell environment[ROWS*COLUMNS])
{
	struct Cell environment_next[ROWS*COLUMNS];
	for (int i=0; i<ROWS*COLUMNS; i++)
		environment_next[i] = environment[i];

	for (int i=0; i<ROWS; i++)
		{
			for (int j=0; j<COLUMNS; j++)
			{
				// check if source cell's fill level is > 1
				// check if here is a water cell above into which
				// fluid can be transferred
				struct Cell source_cell = environment[j + COLUMNS*i];
				if (source_cell.type == WATER_TYPE && source_cell.fill_level > 1 && i > 0 && environment[j+COLUMNS*(i-1)].type == WATER_TYPE && source_cell.fill_level > environment[j+COLUMNS*(i-1)].fill_level)
				{
					struct Cell destination_cell = environment[j+COLUMNS*(i-1)];
					// cell is pressurized and water can flow up
					double transfer_fill = (source_cell.fill_level - 1);
					environment[j+COLUMNS*i].fill_level -=  transfer_fill;
					environment_next[j+COLUMNS*(i-1)].fill_level +=  transfer_fill;
				}
			}
		}
		for (int i=0; i<ROWS*COLUMNS; i++)
			environment[i] = environment_next[i];
}

void simulation_step(struct Cell environment[ROWS*COLUMNS])
{
	simultaion_phase_rule1(environment);
	simultaion_phase_rule2(environment);
}

int main() 
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Liquid Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	SDL_Surface* surface = SDL_GetWindowSurface(window);

	draw_grid(surface);
	// model the cell grid
	struct Cell environment[ROWS * COLUMNS];

	initialize_environment(environment);

	int simulation_running = 1;
	SDL_Event event;
	int current_type = SOLID_TYPE;
	int delete_mode = 0;
	while (simulation_running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) 
			{
				simulation_running = 0;
			}
			if (event.type == SDL_MOUSEMOTION)
			{
				if (event.motion.state != 0)
				{
					int cell_x = event.motion.x / CELL_SIZE;
					int cell_y = event.motion.y / CELL_SIZE;
					int fill_level = delete_mode ? 0 : 1;
					struct Cell cell;
					if (delete_mode != 0)
					{
						current_type = WATER_TYPE;
						fill_level = 0;
						cell = (struct Cell){current_type, fill_level, cell_x, cell_y};
					}
					else
					{
						fill_level = environment[cell_x + COLUMNS * cell_y].fill_level + 1;
						cell = (struct Cell){current_type, fill_level, cell_x, cell_y};
					}
					// printf("Writing cell at: x=%d, y=%d\n", cell_x, cell_y);
					environment[cell_x + COLUMNS*cell_y] = cell;
				}
			}
			if (event.type == SDL_KEYDOWN)
			{
					if (event.key.keysym.sym == SDLK_SPACE)
						current_type = !current_type;
					if (event.key.keysym.sym == SDLK_BACKSPACE)
						delete_mode = !delete_mode;
			}
		}

	  // perform simpulation steps
		simulation_step(environment);

		draw_environment(surface, environment);
		draw_grid(surface);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(30);
	}
}