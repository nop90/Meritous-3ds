//
//   save.c
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
#include <SDL_mixer.h>
#include <zlib.h>

#include "levelblit.h"
#include "mapgen.h"
#include "demon.h"
#include "tiles.h"

gzFile Filefp;
int game_load = 0;

unsigned char lchar;
int fpos = 0;

void FWChar(unsigned char i)
{
	unsigned char c;
	c = i;
	c ^= 0x55;
	c ^= fpos & 0xFF;
	lchar = c;
	fpos++;
	gzputc(Filefp, c);
}

unsigned char FRChar()
{
	unsigned char c;
	c = gzgetc(Filefp);
	c ^= 0x55;
	c ^= fpos & 0xFF;

	lchar = c;
	fpos++;
	return c;
}

void FWInt(int val)
{
	int i, s;
	i = abs(val);
	s = (val >= 0) ? 0 : 1;

	FWChar((i & 0xFF) >> 0);
	FWChar((i & 0xFF00) >> 8);
	FWChar((i & 0xFF0000) >> 16);
	FWChar((i & 0xFF000000) >> 24);

	FWChar(s);
}

void FWFloat(float i)
{
	int num;
	int frac;

	num = (int)(floorf(i));
	FWInt(num);
	frac = (int)((i - (float)num)*2147483647.0);

	FWInt(frac);
}

int FRInt()
{
	int val;
	int i, s;

	i = FRChar();
	i |= FRChar() << 8;
	i |= FRChar() << 16;
	i |= FRChar() << 24;
	s = FRChar();
	val = i * (s?-1:1);

	return val;
}

float FRFloat()
{
	float i;
	int num;
	int frac;

	double f_frac;

	num = FRInt();
	frac = FRInt();

	f_frac = (double)frac / 2147483647.0;

	i = (float)num + (float)f_frac;
	return i;
}

void SaveGame(char *filename)
{
    Mix_HaltChannel(-1);

	lchar = 0x7c;
	fpos = 0;

	Filefp = gzopen(filename, "wb9");
	if(Filefp != Z_NULL)
	{
        FWChar(0x7C);
        WriteMapData();
        WriteCreatureData();
        WritePlayerData();

        gzclose(Filefp);
	}
}

void LoadGame(char *filename)
{
	unsigned char parity;
	fpos = 0;
	lchar = 0x7c;

	Filefp = gzopen(filename, "rb");
	parity = FRChar();
	if (parity != 0x7C) {
		fprintf(stderr, "Parity byte in error (%x != 0x7C)\nAborting\n", parity);
		exit(2);
	}
	game_load = 1;
}

void CloseFile()
{
	gzclose(Filefp);
}

void DoSaveGame()
{
	SavingScreen(0, 0.0);
	SaveGame("/3ds/Meritous/SaveFile.sav");
}

int IsSaveFile()
{
	FILE *fp;
	if ((fp = fopen("/3ds/Meritous/SaveFile.sav", "rb")) != NULL) {
		fclose(fp);
		return 1;
	}
	return 0;
}
