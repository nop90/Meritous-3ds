//
//   help.c
//
//   Copyright 2007, 2008 Lancer-X/ASCEAI
//
//   This file is part of Meritous.
//
//   Meritous is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Meritous is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Meritous.  If not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string.h>

#include "levelblit.h"

struct help_line {
	char *t;
};

struct help_section {
	int lines;
	char *identifier;
	struct help_line *l[256];
};

struct help_file {
	int sections;
	struct help_section *s[256];
};

struct help_file *hlp = NULL;
int my_line;
int my_sec;
int my_cursor;
int my_link;
void InitHelp()
{
	FILE *fp;
	struct help_section *current_sec = NULL;
	struct help_line *current_line = NULL;
	char linebuf[80];
	hlp = malloc(sizeof(struct help_file));
	hlp->sections = 0;

	fp = fopen("romfs:/d/helpfile.txt", "r");
	while (!feof(fp)) {
		fgets(linebuf, 79, fp);
		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = 0;

		if (linebuf[0] == '\'') {
			// comment
			continue;
		}
		if (linebuf[0] == ':') {
			// section
			hlp->s[hlp->sections] = malloc(sizeof(struct help_section));
			current_sec = hlp->s[hlp->sections];
			hlp->sections++;
			current_sec->identifier = (char*)malloc(strlen(linebuf));
			current_sec->lines = 0;
			strcpy(current_sec->identifier, linebuf+1);
			continue;
		}

		// different line
		if (current_sec != NULL) {
			current_sec->l[current_sec->lines] = malloc(sizeof(struct help_line));
			current_line = current_sec->l[current_sec->lines];
			current_sec->lines++;
			current_line->t = (char*)malloc(strlen(linebuf)+1);
			strcpy(current_line->t, linebuf);
		}
	}
	fclose(fp);
}

void DisplayHelp()
{
	static int tick = 0;
	int i;
	struct help_section *current_sec = NULL;
	char *ltext;
	char c_ident[20];
	int line_num;
	int follow_link = 0;
	char linkfollow[20] = "";

	DrawRect(23, 23, 274 + 80, 194, 0);
	DrawRect(24, 24, 272 + 80, 192, 200);
	DrawRect(25, 25, 270 + 80, 190, 255);
	DrawRect(26, 26, 268 + 80, 188, 200);
	DrawRect(27, 27, 266 + 80, 186, 100);
	DrawRect(30, 30, 260 + 80, 180, 20);
	DrawRect(35, 35, 250 + 80, 170, 60);

	// 70x40 display
	current_sec = hlp->s[my_sec];

	my_line = my_cursor - 15;
	if (my_line < 0) my_line = 0;
	if (my_line >= (current_sec->lines)) my_line = current_sec->lines - 1;
	for (i = 0; i < 2; i++) {
		draw_text(23+i, 40+(my_cursor - my_line)*10, "->", 255);
		draw_text((SCREEN_W-41)+i, 40+(my_cursor - my_line)*10, "<-", 255);
	}

	for (i = 0; i < 16; i++) {
		line_num = my_line + i;
		if (line_num >= 0) {
			if (line_num < current_sec->lines) {
				ltext = current_sec->l[line_num]->t;

				switch (ltext[0]) {
					case '!':

						draw_text(80 + (240-strlen(ltext+1)*8)/2, 40+i*10, ltext+1, 255);
						break;
					case '?':
						strncpy(c_ident, ltext+1, strchr(ltext+1, '?')-ltext-1);
						c_ident[strchr(ltext+1, '?')-ltext-1] = 0;

						draw_text(80, 40+i*10, strchr(ltext+1, '?')+1, my_cursor == line_num ? 200+(tick%16)*3 : 150);
						if ((my_link == 1)&&(my_cursor == line_num)) {
							follow_link = 1;
							//my_link = 0;
							strcpy(linkfollow, c_ident);
						}
						break;
					default:
                            draw_text(80, 40+i*10, ltext, 200);
						break;
				}
			}
		}
	}
	tick++;
	  //	SDL_UpdateRect(screen, 0, 0, 0, 0);
	SDL_Flip(screen);

	if (follow_link) {
		for (i = 0; i < hlp->sections; i++) {
			if (strcmp(linkfollow, hlp->s[i]->identifier) == 0) {
				my_sec = i;
				my_cursor = 0;
				break;
			}
		}
		my_link = 0;
	}
}

int MoveCursor()
{
	SDL_Event ev;
	static int key_delay = 0;
	static int key_up = 0, key_down = 0;

	if (key_delay > 0) key_delay--;

	my_link = 0;
	while (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_KEYDOWN) {
			if (ev.key.keysym.sym == SDLK_DOWN) {
				key_down = 1;
				key_delay = 10;
				if (my_cursor < hlp->s[my_sec]->lines-1) my_cursor++;
			}
			if (ev.key.keysym.sym == SDLK_UP) {
				key_up = 1;
				key_delay = 10;
				if (my_cursor > 0) my_cursor--;
			}
			if (ev.key.keysym.sym == SDLK_x) {
				return 0;
			}
			if (ev.key.keysym.sym == SDLK_ESCAPE) {
				return 0;
			}
			if (ev.key.keysym.sym == SDLK_b)
				my_link = 1;
			}

		if (ev.type == SDL_KEYUP) {
			if (ev.key.keysym.sym == SDLK_DOWN) {
				key_down = 0;
			}
			if (ev.key.keysym.sym == SDLK_UP) {
				key_up = 0;
			}
		}
		if (ev.type == SDL_QUIT) {
			return 0;
		}
	}

	if (key_delay == 0) {
		if (key_up == 1) {
			if (my_cursor > 0) my_cursor--;
		}
		if (key_down == 1) {
			if (my_cursor < hlp->s[my_sec]->lines-1) my_cursor++;
		}
	}

	return 1;
}

void ShowHelp()
{
	int in_help = 1;
	if (hlp == NULL) {
		InitHelp();
	}
	my_line = 0;
	my_sec = 0;
	my_cursor = 0;
	my_link = 0;

	while (in_help)
	{
		DisplayHelp();
		in_help = MoveCursor();
		SDL_Delay(30);
	}
}
