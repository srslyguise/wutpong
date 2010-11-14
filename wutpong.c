#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#define PI 3.14159265
#define SPEED 10 //Speed of rectangles
#define RADIUS 8 //Radius of ball

#define X 20 //Initial position of left rectangle
#define Y 240

#define RECTANGLE_BASE 10 //Rectangles width and height
#define RECTANGLE_HEIGHT 100

#define DEF_SPEED_X 13 //Default horizontal and vertical speed of ball
#define DEF_SPEED_Y 8

#define FONT "/usr/share/fonts/X11/TTF/luximb.ttf" //Font path and size
#define FONT_SIZE 12

int left, right, up, down; //Current key pressed
int alive;

SDL_Rect rectangle; //Left rectangle rect
SDL_Rect ball; //Ball rect
SDL_Rect opponent; //Right rectangle rect

TTF_Font* font;

void putpixel(SDL_Surface *, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t); //Put a single pixel on screen
void drawrectangle(SDL_Surface *, uint16_t, uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t); //Draw a rectangle on screen
void drawcircle(SDL_Surface *, uint16_t, uint16_t, double, int16_t, int16_t, uint8_t, uint8_t, uint8_t); //Draw a circle on screen
void putstring(SDL_Surface *, char *, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t); //Put a string on screen

int move(void *); //Thread handler of left rectangle
int moveball(void *); //Thread handler of ball
int automove(void *); //Thread handler of right rectangle

typedef struct ThreadS //Thread parameters
{
	SDL_Surface * screen;
	SDL_mutex * mutex;
} ThreadS;

int main()
{
	SDL_Surface * screen = NULL; //Main screen surface
	SDL_Event e;
	int pressed = 0; //If 1 exit

	SDL_Thread * move_t = NULL; 
	SDL_Thread * moveball_t = NULL; //Threads 
	SDL_Thread * automove_t = NULL;
	SDL_mutex * mutex; //Global Mutex
	ThreadS ts; //Default thread parameters

	alive = left = right = up = down = 0;

	if(SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		printf("Impossibile inizializzare SDL: %s\n", SDL_GetError());
        	exit(1);
    	}

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
	if(screen == NULL)
	{
		printf("Impossibile usare il modo 640x480: %s\n", SDL_GetError());
		exit(1);
	}

	TTF_Init();

	if((font = TTF_OpenFont(FONT, FONT_SIZE)) == NULL)
	{
		printf("Impossibile caricare il font: %s\n", SDL_GetError());
		exit(1);
	}
	mutex = SDL_CreateMutex();

	ts.screen = screen;
	ts.mutex = mutex;

	alive = 1;

	rectangle.x = X - (RECTANGLE_BASE / 2);
	rectangle.y = Y - (RECTANGLE_HEIGHT / 2);
	rectangle.w = RECTANGLE_BASE;
	rectangle.h = RECTANGLE_HEIGHT;

	opponent.x = screen->w - X - (RECTANGLE_BASE / 2);
	opponent.y = Y - (RECTANGLE_HEIGHT / 2);
	opponent.w = RECTANGLE_BASE;
	opponent.h = RECTANGLE_HEIGHT;

	ball.x = screen->w / 2;
	ball.y = screen->h / 2;
	ball.w = RADIUS * 2;
	ball.h = RADIUS * 2;

	moveball_t = SDL_CreateThread(moveball, (void *)&ts);
	move_t = SDL_CreateThread(move, (void *)&ts);
	automove_t = SDL_CreateThread(automove, (void *)&ts);

	putstring(screen, "0|0", (screen->w / 2), FONT_SIZE, 0xff, 0x00, 0x00);

	drawrectangle(screen, rectangle.x, rectangle.y, rectangle.w, rectangle.h, 0xff, 0x00, 0x00);
	drawrectangle(screen, opponent.x, opponent.y, opponent.w, opponent.h, 0xff, 0x00, 0x00);

	while((!pressed) || alive)
	{
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
				case SDL_QUIT:
					pressed = 1;
					alive = 0;
					break;

				case SDL_KEYDOWN:
				{
					switch(e.key.keysym.sym)
					{
						case SDLK_UP:
							up = 1;
							break;
						case SDLK_DOWN:
							down = 1;
							break;
						case SDLK_LEFT:
							left = 1;
							break;
						case SDLK_RIGHT:
							right = 1;
							break;

					}
					break;
				}

				case SDL_KEYUP:
				{
					switch(e.key.keysym.sym)
					{
						case SDLK_UP:
							up = 0;
							break;
						case SDLK_DOWN:
							down = 0;
							break;
						case SDLK_LEFT:
							left = 0;
							break;
						case SDLK_RIGHT:
							right = 0;
							break;

					}
					break;
				}
			}
		}
		SDL_Delay(50);
	}

	SDL_WaitThread(move_t, NULL);
	SDL_WaitThread(automove_t, NULL);
	SDL_WaitThread(moveball_t, NULL);

	SDL_DestroyMutex(mutex);
	TTF_CloseFont(font);

	return 0;
}

int move(void * p)
{
	ThreadS * ts = (ThreadS *)p;
	SDL_Surface * screen = ts->screen;

	printf("move thread starting\n");

	while(alive)
	{
		SDL_mutexP(ts->mutex);
		
		if(up)
		{
			if(rectangle.y >= ((rectangle.h / 2)) + SPEED)
			{
				drawrectangle(screen, rectangle.x, rectangle.y, rectangle.w, rectangle.h, 0x00, 0x00, 0x00);
				drawrectangle(screen, rectangle.x, rectangle.y-=SPEED, rectangle.w, rectangle.h, 0xff, 0x00, 0x00);
			}
		}

		if(down)
		{
			if(rectangle.y <= (screen->h - ((rectangle.h / 2)) - SPEED))
			{
				drawrectangle(screen, rectangle.x, rectangle.y, rectangle.w, rectangle.h, 0x00, 0x00, 0x00);
				drawrectangle(screen, rectangle.x, rectangle.y+=SPEED, rectangle.w, rectangle.h, 0xff, 0x00, 0x00);
			}
		}

		SDL_mutexV(ts->mutex);
		SDL_Delay(30);
	}
	alive = 0;

	printf("move thread ending\n");

	return 0;
}

int automove(void * p)
{
	ThreadS * ts = (ThreadS *)p;
	SDL_Surface * screen = ts->screen;

	printf("automove thread starting\n");

	while(alive)
	{
		SDL_mutexP(ts->mutex);
		
		if(ball.y < opponent.y)
		{
			if(opponent.y >= ((opponent.h / 2)) + SPEED)
			{
				drawrectangle(screen, opponent.x, opponent.y, opponent.w, opponent.h, 0x00, 0x00, 0x00);
				drawrectangle(screen, opponent.x, opponent.y-=SPEED, opponent.w, opponent.h, 0xff, 0x00, 0x00);
			}
		}

		if(ball.y > (opponent.y + opponent.h))
		{
			if(opponent.y <= (screen->h - ((opponent.h / 2)) - SPEED))
			{
				drawrectangle(screen, opponent.x, opponent.y, opponent.w, opponent.h, 0x00, 0x00, 0x00);
				drawrectangle(screen, opponent.x, opponent.y+=SPEED, opponent.w, opponent.h, 0xff, 0x00, 0x00);
			}
		}

		SDL_mutexV(ts->mutex);
		SDL_Delay(30);
	}
	alive = 0;

	printf("automove thread ending\n");

	return 0;
}

int moveball(void * p)
{
	ThreadS * ts = (ThreadS *)p;
	SDL_Surface * screen = ts->screen;

	int16_t speed_x, speed_y;
	uint8_t score[2] = {0};
	char score_str[11] = {0};

	srand(time(NULL));

	speed_x = DEF_SPEED_X;
	speed_y = DEF_SPEED_Y * ((rand() % 2) ? 1 : -1); //Random vertical direction

	printf("moveball thread starting\n");

	while(alive)
	{
		SDL_mutexP(ts->mutex);

		if(ball.x < ((RADIUS * 2) + 10))
		{
			drawcircle(screen, ball.x, ball.y, RADIUS, 0, 0, 0x00, 0x00, 0x00);
			ball.x = rectangle.x + rectangle.w + 20;
			ball.y = rectangle.y;
			speed_y = DEF_SPEED_Y * ((rand() % 2) ? 1 : -1);
			speed_x = speed_x * - 1;
			sprintf(score_str, "%d|%d", score[0], score[1]++);
			putstring(ts->screen, score_str, (ts->screen->w / 2), FONT_SIZE, 0x00, 0x00, 0x00);

			memset(score_str, 0, 11);

			sprintf(score_str, "%d|%d", score[0], score[1]);
			putstring(ts->screen, score_str, (ts->screen->w / 2), FONT_SIZE, 0xff, 0x00, 0x00);
		}

		if(ball.x > (screen->w - (RADIUS * 2) - 10))
		{
			drawcircle(screen, ball.x, ball.y, RADIUS, 0, 0, 0x00, 0x00, 0x00);
			ball.x = opponent.x - 20;
			ball.y = opponent.y;
			speed_y = DEF_SPEED_Y * ((rand() % 2) ? 1 : -1);
			speed_x = speed_x * - 1;
			sprintf(score_str, "%d|%d", score[0]++, score[1]);
			putstring(ts->screen, score_str, (ts->screen->w / 2), FONT_SIZE, 0x00, 0x00, 0x00);

			memset(score_str, 0, 11);

			sprintf(score_str, "%d|%d", score[0], score[1]);
			putstring(ts->screen, score_str, (ts->screen->w / 2), FONT_SIZE, 0xff, 0x00, 0x00);
		}

		if(ball.x >= (opponent.x - (RADIUS * 2)))
			if((ball.y >= (opponent.y - (opponent.h / 2.5)) && (ball.y <= (opponent.y + opponent.h))))
				speed_x = speed_x * - 1;
		
		if(ball.x <= (rectangle.x + rectangle.w + (RADIUS * 2)))
			if((ball.y >= (rectangle.y - (rectangle.h / 2.5)) && (ball.y <= (rectangle.y + rectangle.h))))
				speed_x = speed_x * - 1;

		if(ball.y >= (screen->h - (RADIUS * 2)))
			speed_y = speed_y * - 1;
		
		if(ball.y <= (RADIUS * 2))
			speed_y = speed_y * - 1;

		drawcircle(screen, ball.x, ball.y, RADIUS, 0, 0, 0x00, 0x00, 0x00);
		drawcircle(screen, ball.x+=speed_x, ball.y+=speed_y, RADIUS, 0, 0, 0xff, 0x00, 0x00);

		SDL_mutexV(ts->mutex);
		SDL_Delay(20);
	}
	alive = 0;

	printf("moveball thread ending\n");

	return 0;
}

void drawrectangle(SDL_Surface * screen, uint16_t x, uint16_t y, uint16_t base, uint16_t height, uint8_t R, uint8_t G, uint8_t B)
{
	int i;
	SDL_Rect rect;

	rect.x = x - (base / 2);
	rect.y = y - (height / 2);
	rect.w = base;
	rect.h = height;

	for(i = 0; i < base; i++)
		putpixel(screen, (rect.x + i), rect.y, R, G, B);

	for(i = 0; i < base; i++)
		putpixel(screen, (rect.x + i), (rect.y + height - 1), R, G, B);

	for(i = 0; i < height; i++)
		putpixel(screen, rect.x, (rect.y + i), R, G, B);

	for(i = 0; i < height; i++)
		putpixel(screen, (rect.x + base - 1), (rect.y + i), R, G, B);


	SDL_UpdateRects(screen, 1, &rect);
}

void drawcircle(SDL_Surface * screen, uint16_t x, uint16_t y, double radius, int16_t a, int16_t b, uint8_t R, uint8_t G, uint8_t B)
{
	double i;
	SDL_Rect rect;
	int16_t xX, yY = 0;

	rect.x = x - radius - (b - a);
	rect.y = y - radius - (a - b);
	rect.w = (radius * 2) + ((b - a) * 2);
	rect.h = (radius * 2) + ((a - b) * 2);

	for(i = 0; i < 360; i+=0.1)
	{
		xX = x + ((radius + (b - a)) * cos(i * PI/180));
		yY = y + ((radius + (a - b)) * sin(i * PI/180));

		putpixel(screen, xX, yY, R, G, B);
	}

	SDL_UpdateRects(screen, 1, &rect);
}

void putpixel(SDL_Surface * screen, uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
{
	uint16_t * ptr = NULL;

	if((x <= 0) || (y <= 0))
		return;

	if((x >= screen->w) || (y >= screen->h))
		return;

	ptr = (uint16_t *)screen->pixels + y*screen->pitch/2 + x;
	*ptr = SDL_MapRGB(screen->format, R, G, B);
}

void putstring(SDL_Surface * screen, char * string, uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
{
	SDL_Surface * fontSurface;
	SDL_Color color;
	SDL_Rect fontRect;

	color.r = R;
	color.g = G;
	color.b = B;

	fontSurface = TTF_RenderText_Solid(font, string, color);
        fontRect.x = x;
        fontRect.y = y;

        SDL_BlitSurface(fontSurface, NULL, screen, &fontRect);
	SDL_Flip(screen);
}
