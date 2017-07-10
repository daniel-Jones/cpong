#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>

#define  BOTSGAME 1

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

void setup();
void draw();
void balllogic();
void botlogic();
void playerbotlogic();
int touchingplayer();
int touchingbot();

/* playfield */
WINDOW *playfield;
int playfieldoffsetx, playfieldoffsety;
#define PLAYFIELDWIDTH 90
#define PLAYFIELDHEIGHT 20

/* ball */
WINDOW *ball;
int ballwidth, ballheight, ballx, bally, balldirx, balldiry;

/* player paddle */
WINDOW *player;
int playerwidth, playerheight, playerx, playery, playerscore;

/* bot paddle */
WINDOW *bot;
int botwidth, botheight, botx, boty, botscore;

/* decoration text */
char *welcome = "PONG in ncurses! How original!";


int main (int argc, char *argv[])
{
	initscr();
	/* check width and height */
	if (COLS < PLAYFIELDWIDTH + 5 || LINES < PLAYFIELDHEIGHT + 5)
	{
		printf("Screen too small to enjoy pong!\n");
		endwin();
		exit(0);
	}
	cbreak();
	noecho();
	curs_set(0);
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	bkgd(COLOR_PAIR(1)); /* stdscr color */
	keypad(stdscr, TRUE);
	setup();
	int ch;
	timeout(50);
	keypad(player, TRUE);
	while(1)
	{	
		ch = getch();
		if (ch == KEY_UP && playery >= (playfieldoffsety + 1))
			playery -= 3;
		if (ch == KEY_DOWN && playery <= PLAYFIELDHEIGHT + 1)
			playery += 3;

		balllogic();
		botlogic();
		if (BOTSGAME == 1)
			playerbotlogic();
		draw();
		//usleep(10000);
	}
	endwin();
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(local_win);
	return local_win;
}

void destroy_win(WINDOW *local_win)
{
	wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(local_win);
	delwin(local_win);
}

void setup()
{
	/* play field */
	playfieldoffsetx = (COLS - PLAYFIELDWIDTH) / 2;
	playfieldoffsety = (LINES - PLAYFIELDHEIGHT) / 2;
	mvprintw(3, (COLS - strlen(welcome)) / 2, welcome);
	playfield = create_newwin(PLAYFIELDHEIGHT, PLAYFIELDWIDTH, playfieldoffsety, playfieldoffsetx);
	
	/* player */
	playerwidth = 2;
	playerheight = 5;
	playerx = playfieldoffsetx + 1;
	playery = playfieldoffsety + ((PLAYFIELDHEIGHT - playerheight) / 2);
	player = create_newwin(playerheight, playerwidth, playery, playerx);
	init_pair(2, COLOR_WHITE, COLOR_WHITE);
	wbkgd(player, COLOR_PAIR(2)); /* player color */

	/* bot */
	botwidth = 2;
	botheight = 5;
	botx = COLS - playfieldoffsetx - 4;
	boty = playfieldoffsety + ((PLAYFIELDHEIGHT - playerheight) / 2);
	bot = create_newwin(playerheight, playerwidth, playery, playerx);
	wbkgd(bot, COLOR_PAIR(2)); /* player color */


	/* ball */
	init_pair(3, COLOR_RED, COLOR_RED);
	ballwidth = 2;
	ballheight = 1;
	ballx = COLS / 2;
	bally = LINES / 2;
	balldirx = 0; /* 0 = left, 1 = right */
	balldiry = 1; /* 0 = down, 1 = up */
	ball = create_newwin(ballheight, ballwidth, ballx, bally);
	wbkgd(ball, COLOR_PAIR(2));
}

void draw()
{
	/* play field */
	// fix one day, deleting the window every time is BAD
	destroy_win(playfield);
	playfield = create_newwin(PLAYFIELDHEIGHT, PLAYFIELDWIDTH, playfieldoffsety, playfieldoffsetx);
	//
	/* player */
	mvwin(player, playery, playerx);
	wrefresh(player);
	/* bot */
	mvwin(bot, boty, botx);
	wrefresh(bot);
	/* ball */
	mvwin(ball, bally, ballx);
	wrefresh(ball);
	/* score */
	mvprintw(5, 1, "YOUR SCORE: %d", playerscore);
	mvprintw(6, 1, "BOTS SCORE: %d", botscore);

}

void balllogic()
{
	if (balldirx == 1)
		ballx += 3;
	if (balldirx == 0)
		ballx -= 3;
	if (balldiry == 1)
		bally--;
	if (balldiry == 0)
		bally++;
	if (touchingbot())
		balldirx = 0;
	if (touchingplayer())
		balldirx = 1;
	if (bally + ballheight >= (LINES - playfieldoffsety))
		balldiry = 1;
	if (bally <= playfieldoffsety)
		balldiry = 0;

	if (ballx < playerx - playerheight)
	{
		botscore++;
		ballx = COLS / 2;
	}
	if (ballx > botx + botheight)
	{
		playerscore++;
		ballx = COLS / 2;
		balldirx = 1;
	}
}

void botlogic()
{
	/* 0 = down, 1 = up, 0 = left, 1 = right */
	if (balldirx == 1 && balldiry == 1 && boty >= (playfieldoffsety + 1))
		boty -= 1;
	if (balldirx == 1 && balldiry == 0 && boty <= PLAYFIELDHEIGHT + 1)
		boty += 1;
}

void playerbotlogic()
{
	/* 0 = down, 1 = up, 0 = left, 1 = right */
	if (balldirx == 0 && balldiry == 1 && playery  + 1 >= (playfieldoffsety + 1))
		playery -= 1;
	if (balldirx == 0 && balldiry == 0 && playery + 1 <= PLAYFIELDHEIGHT + 1)
		playery += 1;
}

int touchingplayer()
{
//	if (ballx == playerx && bally == playery || ballx > playerx && (ballx + ballheight) < (playerx + playerheight) && (bally + ballheight) < (playery + playerheight))
	if (ballx <= playerx && bally > playery && bally < (playery + playerheight))
		return 1;
	return 0;
}

int touchingbot()
{
	if (ballx >= botx && bally > boty && bally < (boty + botheight))
		return 1;
	return 0;
}







