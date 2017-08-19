//
//   gamemap.c
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

#include "levelblit.h"
#include "mapgen.h"
#include "tiles.h"
#include "save.h"

SDL_Surface *automap = NULL;
void RecordRoom(int room_id);
int full_rend = 0;
void FullRender();
int c_scroll_x=0, c_scroll_y=0;
SDL_Surface *overview = NULL;

void InitAutomap()
{
	int i;
	if (automap != NULL) SDL_FreeSurface(automap);
	if (overview != NULL) SDL_FreeSurface(overview);

	automap = IMG_Load("romfs:/i/automap.png"); //520x520
	overview = IMG_Load("romfs:/i/overview.png"); //200x200

	//SDL_SetColorKey(automap, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);

	full_rend = 0;
	if (game_load) {
		if (artifacts[0]) {
			FullRender();
		} else {
			for (i = 0; i < 3000; i++) {
				if (rooms[i].visited) {
					RecordRoom(i);
				}
			}
			RecordRoom(player_room);
		}
	}
}

void FullRender()
{
	int x, y, i;
	Uint8 *pix;

	for (y = 0; y < map.h; y++) {
		for (x = 0; x < map.w; x++) {
			if (TileData[Get(x, y)].Is_Solid) {
				if (Get(x, y) != 17) {
					pix = automap->pixels;
					pix += automap->w * (y + 4);
					pix += x + 4;
					*pix = 128;
				}
			}
		}
	}

	for (i = 0; i < 3000; i++) {
		if (rooms[i].visited) {
			RecordRoom(i);
		}
	}
	RecordRoom(player_room);

	full_rend = 1;
}

void DisplayRoom(int room_id, Uint8 room_bg)
{
	int x, y;
	int rx, ry;
	SDL_Rect fill;
	Uint8 *pix;

	for (y = 0; y < rooms[room_id].h; y++) {
		for (x = 0; x < rooms[room_id].w; x++) {
			rx = x + rooms[room_id].x;
			ry = y + rooms[room_id].y;

			pix = automap->pixels;
			pix += automap->w * (ry + 4);
			pix += rx + 4;

			*pix = automap_cols[TileData[Get(rx, ry)].Type];
			if (*pix == 192) *pix = room_bg;
		}
	}

	fill.x = (rooms[room_id].x * 200 / 512);
	fill.y = (rooms[room_id].y * 200 / 512);
	fill.w = (rooms[room_id].w * 200 / 512);
	fill.h = (rooms[room_id].h * 200 / 512);

	SDL_FillRect(overview, &fill, 200);
}

void RecordRoom(int room_id)
{
	static int last_player_room = -1;
	if (last_player_room != -1) {
		if (rooms[last_player_room].checkpoint) DisplayRoom(last_player_room, 150 - (rooms[last_player_room].s_dist/5*12));
		else DisplayRoom(last_player_room, 192 - (rooms[last_player_room].s_dist/5*12));
	}
	last_player_room = room_id;
	DisplayRoom(room_id, 0);
}

void DisplayAutomap()
{
	SDL_Rect from;
	SDL_Rect position;
	int x, y;
	int df_x, df_y;
	int rx, ry;
	int rw, rh;
	int tile;
	int nearest_checkpoint;
	unsigned char col;
	unsigned char xcol = 0;
	static int t = 0;
	int minimap_scroll_x, minimap_scroll_y;

	int x_something = 0;
	int y_something = 0;

	if (key_held[K_UP]) {
		c_scroll_y -= 32 + (key_held[K_SP]*64);
	}
	if (key_held[K_DN]) {
		c_scroll_y += 32 + (key_held[K_SP]*64);
	}
	if (key_held[K_LT]) {
		c_scroll_x -= 32 + (key_held[K_SP]*64);
	}
	if (key_held[K_RT]) {
		c_scroll_x += 32 + (key_held[K_SP]*64);
	}

	if (c_scroll_x < 0) c_scroll_x = 0;
	if (c_scroll_y < 0) c_scroll_y = 0;
	if (c_scroll_x >= 512*31) c_scroll_x = 512*31 - 32;
	if (c_scroll_y >= 512*32) c_scroll_y = 512*32 - 32;

	nearest_checkpoint = GetNearestCheckpoint(c_scroll_x, c_scroll_y);

	if (artifacts[0] && (!full_rend)) FullRender();

	minimap_scroll_x = (c_scroll_x / 38 - x_something);
	minimap_scroll_y = (c_scroll_y / 49 - y_something);
	if (minimap_scroll_x < 0) minimap_scroll_x = 0;
	if (minimap_scroll_y < 0) minimap_scroll_y = 0;

	if (minimap_scroll_x >= 512 - (x_something)) minimap_scroll_x = 512 - (x_something) - 1;
	if (minimap_scroll_y >= 512 - (y_something)) minimap_scroll_y = 512 - (y_something) - 1;

//	DrawRect(2, 32, 276, 195, 255);
	DrawRect(0, 32, 400, 195, 255);
	t++;

	for (y = 0; y < 54; y++) {
		for (x = 0; x < 60; x++) {
			xcol = 0;
			df_x = x * 8 + -140;
			df_y = y * 8 + -90;

			rx = c_scroll_x / 32 - 27 + x;
			ry = c_scroll_y / 32 - 27 + y;
			if ((rx >= 0)&&(ry >= 0)&&(rx < 512)&&(ry < 512)) {
				if (rooms[GetRoom(rx, ry)].visited) {
					tile = Get(rx, ry);
					if (tele_select && (nearest_checkpoint == GetRoom(rx, ry)) && ((t / 3) % 2)) {
						xcol = 255;
					}
					col = automap_cols[TileData[tile].Type];
					DrawRect(df_x, df_y, 8, 8, col ^ xcol);

					switch (tile) {
						case 25:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, '*', 0 ^ xcol);
							break;
						case 26:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, '+', 0 ^ xcol);
							break;
						case 28:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, 'S', 0 ^ xcol);
							break;
						case 29:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, 'C', 0 ^ xcol);
							break;
						case 30:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, 'R', 0 ^ xcol);
							break;
						case 31:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, 'S', 0 ^ xcol);
							break;
						case 32:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, 29, 0 ^ xcol);
							break;
						case 53:
							DrawRect(df_x, df_y, 8, 8, 192 ^ xcol);
							draw_char(df_x, df_y, 28, 0 ^ xcol);
							break;
					}

					if (tile < 12) {
						DrawRect(df_x, df_y, 8, 8, 0 ^ xcol);
						draw_char(df_x, df_y, tile + 10, 255 ^ xcol);
					}

					if ((tile >= 45)&&(tile <= 52)) {
						DrawRect(df_x, df_y, 8, 8, 0 ^ xcol);
						draw_char(df_x, df_y, (tile - 45)%4 + 14, 255 ^ xcol);
					}

					if ((tile >= 13)&&(tile < 17)) {
						DrawRect(df_x, df_y, 8, 8, 0 ^ xcol);
						draw_char(df_x, df_y, tile - 13 + 22, 255 ^ xcol);
					}
					if ((tile >= 21)&&(tile < 25)) {
						DrawRect(df_x, df_y, 8, 8, 0 ^ xcol);
						draw_char(df_x, df_y, tile - 13 + 22, 255 ^ xcol);
					}
					if ((tile >= 38)&&(tile < 42)) {
						DrawRect(df_x, df_y, 8, 8, 0 ^ xcol);
						draw_char(df_x, df_y, tile - 13 + 22, 255 ^ xcol);
						draw_char(df_x, df_y, 26, 255 ^ xcol);
					}

					if (( (player_x / 32) == rx)&&( (player_y / 32) == ry)) {
						DrawRect(df_x, df_y, 8, 8, 0 ^ xcol);
						draw_char(df_x, df_y, 30, 255 ^ xcol);
					}
				}
			}
		}
	}

    position.x = 283;
	position.y = 32;
	position.w = 114;
	position.h = 236;

	from.x = (minimap_scroll_x );
	from.y = (minimap_scroll_y );
	from.w = 114;
	from.h = 236;

	SDL_FillRect(screen, &position, 255);
	SDL_BlitSurface(automap, &from, screen, &position);

	// Now, to cover up the gaps!

	// +-------1----+-----+
	// |            |     |
	// 3            4     5
	// |            |     |
	// |            |     |
	// |            |     |
	// +-------2----+-----+

	DrawRect(0, 29, 400, 3, 230-32); 	// 1
	DrawRect(0, 227-5, 400, 4, 230-32); 	// 2
	DrawRect(0, 29, 4, 227-32, 230-32); 	// 3
	DrawRect(280, 29, 3, 195, 230-32); 	// 4
	DrawRect(397, 29, 3, 195, 230-32); 	// 5

}
