#define _USE_MATH_DEFINES
#define AUTA_1 5
#define AUTA_2 5
#define AUTA_3 5
#define AUTA_4 5
#define AUTA_5 5
#define BELKA_3 3
#define BELKA_2 5
#define BELKA_1 4
#define ZOLWIE_3 4
#define ZOLWIE_2 5
#define SCREEN_WIDTH 550
#define BOARD_HEIGHT 550
#define SCREEN_HEIGHT 650
#define UPPER_MIDDLE_EDGE 270
#define LOWER_MIDDLE_EDGE 305
#define DIFF 38
#define START_HEIGHT 210
#define LEFT_LAND_SPOT 20
#define RIGHT_LAND_SPOT 65
#define SPOT_DIFF 29
#define JUSTIFY 3
#define SPOTS 5
#define LIVES 3
#define LIVES_DIFF 25
#define LIVES_Y 580
#define PASEK_CZASU 200
#define CZAS 50
#define CZAS_KRYTYCZNY 10
#define NESTS_COUNT 5
#define VERTICAL_SPEED_1 37
#define VERTICAL_SPEED_2 38
#define HORIZONTAL_SPEED 29
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>

extern "C" {
#include "SDL2-2.0.10/include/SDL.h"
#include "SDL2-2.0.10/include/SDL_main.h"
}

typedef struct
{
	SDL_Surface* sciezka;
	int wysokosc;
	int szerokosc;
	int wsp_x;
	int wsp_y;
}obiekt_t;

typedef struct
{
	int lewy_x;
	int prawy_x;
}gniazdo_t;

const int PROPORCJA = PASEK_CZASU / CZAS;
const int SCREEN_UP = (SCREEN_HEIGHT - BOARD_HEIGHT) / 2;
const int FROGGER_START_X = SCREEN_WIDTH / 2;
const int FROGGER_START_Y = BOARD_HEIGHT - 28;

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void SprawdzBledy(SDL_Surface* obiekt, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer)
{
	if (obiekt == NULL) 
	{
		printf("error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
	};
}

void SprawdzBledyGrupie(obiekt_t obiekt[], int liczba_obiektow, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer)
{
	for (int i = 0; i < liczba_obiektow; i++)
	{
		SprawdzBledy(obiekt[i].sciezka, screen, scrtex, window, renderer);
	}
}

// ustawianie wartoœci pojedynczemu obiektowi
void Ustaw_Wartosci_Jednemu(obiekt_t* obiekt, SDL_Surface* sciezka, int wsp_x, int wsp_y,
	int szerokosc, int wysokosc)
{
	obiekt->sciezka = sciezka;
	obiekt->wsp_x = wsp_x;
	obiekt->wsp_y = wsp_y;
	obiekt->szerokosc = szerokosc;
	obiekt->wysokosc = wysokosc;
}

// przyjmowanie tablicy obiektów, i nadawanie im wartoœci
void Ustaw_Wartosci_Obiektom(obiekt_t obiekt[], int liczba_obiektow, SDL_Surface* sciezka,
	int wsp_y, int szerokosc, int wysokosc, int dystans)
{
	for (int i = 0; i < liczba_obiektow; i++)
	{
		int wsp_x = rand() % (2 * SCREEN_WIDTH);
		for (int j = i - 1; j >= 0; j--)
		{
			while (abs(wsp_x - obiekt[j].wsp_x) <= dystans)
			{
				wsp_x = rand() % (2 * SCREEN_WIDTH);
				j = i - 1;
			}
		}
		Ustaw_Wartosci_Jednemu(&obiekt[i], sciezka, wsp_x, wsp_y, szerokosc, wysokosc);
	}
}

// ustawia wspó³rzêdne krañców obiektów (lewy, prawy, górny, dolny)
void Tworz_Wspolrzedne_Obiektow(obiekt_t obiekt, int* lewy_x_obiekt, int* prawy_x_obiekt, int* gorny_y_obiekt, int* dolny_y_obiekt)
{
	 *lewy_x_obiekt = obiekt.wsp_x - obiekt.szerokosc / 2;
	 *prawy_x_obiekt = obiekt.wsp_x + obiekt.szerokosc / 2;
	 *dolny_y_obiekt = obiekt.wsp_y + obiekt.wysokosc / 2;
	 *gorny_y_obiekt = obiekt.wsp_y - obiekt.wysokosc / 2;
}

// sprawia, ¿e frogger 'p³ynie' razem z ¿ó³wiami lub k³odami
void Przesun_Froggera(SDL_Surface* screen, obiekt_t* frogger, obiekt_t obiekt, bool lewo)
{
	if (frogger->wsp_y >= 62 && frogger->wsp_y <= SCREEN_HEIGHT / 2 - 8 - SCREEN_UP)
	{
		int lewy_x_frogger, prawy_x_frogger, dolny_y_frogger, gorny_y_frogger;
		Tworz_Wspolrzedne_Obiektow(*frogger, &lewy_x_frogger, &prawy_x_frogger, &gorny_y_frogger, &dolny_y_frogger);
		int srodek_y_frogger = frogger->wsp_y;

		int lewy_x_obiekt, prawy_x_obiekt, dolny_y_obiekt, gorny_y_obiekt;
		Tworz_Wspolrzedne_Obiektow(obiekt, &lewy_x_obiekt, &prawy_x_obiekt, &gorny_y_obiekt, &dolny_y_obiekt);


		if (lewy_x_frogger >= lewy_x_obiekt && prawy_x_frogger <= prawy_x_obiekt
			&& srodek_y_frogger >= gorny_y_obiekt && srodek_y_frogger <= dolny_y_obiekt)
		{
			if(lewo)
			{
				DrawSurface(screen, frogger->sciezka, frogger->wsp_x--, frogger->wsp_y);
			}
			else
			{
				DrawSurface(screen, frogger->sciezka, frogger->wsp_x++, frogger->wsp_y);
			}
		}
	}
}

// rysuje wszystkie obiekty poruszaj¹ce siê w lew¹ stronê ekranu
void Rysuj_Jadace_W_Lewo(SDL_Surface* screen, obiekt_t obiekt[], int liczba_obiektow, bool kolizja, obiekt_t* frogger)
{
	for (int i = 0; i < liczba_obiektow; i++)
	{
		if (obiekt[i].wsp_x <= -obiekt[i].szerokosc)
		{
			obiekt[i].wsp_x = 2 * SCREEN_WIDTH;
		}
		if (!kolizja)
		{
			DrawSurface(screen, obiekt[i].sciezka, obiekt[i].wsp_x--, obiekt[i].wsp_y);
			Przesun_Froggera(screen, frogger, obiekt[i], true);
		}
		else
		{
			DrawSurface(screen, obiekt[i].sciezka, obiekt[i].wsp_x, obiekt[i].wsp_y);
		}
	}
}

// rysuje wszystkie obiekty poruszaj¹ce siê w praw¹ stronê ekranu
void Rysuj_Jadace_W_Prawo(SDL_Surface* screen, obiekt_t obiekt[], int liczba_obiektow, bool kolizja, obiekt_t* frogger)
{
	for (int i = 0; i < liczba_obiektow; i++)
	{
		if (obiekt[i].wsp_x >= 2 * SCREEN_WIDTH)
		{
			obiekt[i].wsp_x = -obiekt[i].szerokosc;
		}
		if (!kolizja)
		{
			DrawSurface(screen, obiekt[i].sciezka, obiekt[i].wsp_x++, obiekt[i].wsp_y);
			Przesun_Froggera(screen, frogger, obiekt[i], false);
		}
		else
		{
			DrawSurface(screen, obiekt[i].sciezka, obiekt[i].wsp_x, obiekt[i].wsp_y);
		}
	}
}

// sprawdza czy nast¹pi³a kolizja z samochodzikami
void Kolizja_Samochodziki(obiekt_t frogger, obiekt_t obiekt[], int liczba_obiektow, bool* kolizja)
{
	int lewy_x_frogger, prawy_x_frogger, dolny_y_frogger, gorny_y_frogger;
	Tworz_Wspolrzedne_Obiektow(frogger, &lewy_x_frogger, &prawy_x_frogger, &gorny_y_frogger, &dolny_y_frogger);

	for (int i = 0; i < liczba_obiektow; i++)
	{
		int lewy_x_obiekt, prawy_x_obiekt, dolny_y_obiekt, gorny_y_obiekt;
		Tworz_Wspolrzedne_Obiektow(obiekt[i], &lewy_x_obiekt, &prawy_x_obiekt, &gorny_y_obiekt, &dolny_y_obiekt);


		if (prawy_x_frogger >= lewy_x_obiekt && prawy_x_frogger <= prawy_x_obiekt
			&& dolny_y_frogger >= gorny_y_obiekt && dolny_y_frogger <= dolny_y_obiekt)
		{
			*kolizja = true;
		}

		if (lewy_x_frogger >= lewy_x_obiekt && lewy_x_frogger <= prawy_x_obiekt
			&& dolny_y_frogger >= gorny_y_obiekt && dolny_y_frogger <= dolny_y_obiekt)
		{
			*kolizja = true;
		}

		if (prawy_x_frogger >= lewy_x_obiekt && prawy_x_frogger <= prawy_x_obiekt
			&& gorny_y_frogger >= gorny_y_obiekt && gorny_y_frogger <= dolny_y_obiekt)
		{
			*kolizja = true;
		}

		if (lewy_x_frogger >= lewy_x_obiekt && lewy_x_frogger <= prawy_x_obiekt
			&& gorny_y_frogger >= gorny_y_obiekt && gorny_y_frogger <= dolny_y_obiekt)
		{
			*kolizja = true;
		}
	}
}

// sprawdza czy nast¹pi³a kolizja z wod¹
void Kolizja_Wodna(obiekt_t frogger, obiekt_t obiekt[], int liczba_obiektow, bool* kolizja)
{
	int lewy_x_frogger, prawy_x_frogger, dolny_y_frogger, gorny_y_frogger;
	Tworz_Wspolrzedne_Obiektow(frogger, &lewy_x_frogger, &prawy_x_frogger, &gorny_y_frogger, &dolny_y_frogger);
	int srodek_y_frogger = frogger.wsp_y;
	bool* kolizje = (bool*)malloc(liczba_obiektow * sizeof(bool));

	for (int i = 0; i < liczba_obiektow; i++)
	{
		kolizje[i] = false;
		int lewy_x_obiekt, prawy_x_obiekt, dolny_y_obiekt, gorny_y_obiekt;
		Tworz_Wspolrzedne_Obiektow(obiekt[i], &lewy_x_obiekt, &prawy_x_obiekt, &gorny_y_obiekt, &dolny_y_obiekt);

		if (srodek_y_frogger >= gorny_y_obiekt && srodek_y_frogger <= dolny_y_obiekt)
		{
			if (lewy_x_frogger > prawy_x_obiekt || prawy_x_frogger < lewy_x_obiekt
				|| dolny_y_frogger < gorny_y_obiekt || gorny_y_frogger > dolny_y_obiekt)
			{
				kolizje[i] = true;
			}
		}
	}
	for (int i = 0; i < liczba_obiektow; i++)
	{
		if (!kolizje[i])
		{
			return;
		}
	}
	*kolizja = true;
}

// sprawdza czy frogger nie wyszed³ poza ekran
void Kolizja_Z_Ekranem(obiekt_t frogger, bool* kolizja)
{
	int lewy_x_frogger, prawy_x_frogger, dolny_y_frogger, gorny_y_frogger;
	Tworz_Wspolrzedne_Obiektow(frogger, &lewy_x_frogger, &prawy_x_frogger, &gorny_y_frogger, &dolny_y_frogger);

	if (lewy_x_frogger < 0 || prawy_x_frogger > SCREEN_WIDTH || dolny_y_frogger > BOARD_HEIGHT)
	{
		*kolizja = true;
	}
}

// sprawdza czy frogger wskoczy³ do siedliska, ewentualnie rysuje obrazek froggera i resetuje pozycjê froggera
void Parkuj_Froggera(SDL_Surface* screen, obiekt_t* frogger, obiekt_t obiekt[], gniazdo_t gniazdo[], int spots, bool wolne[], bool* kolizja, double* worldTime, int* zycia)
{
	for (int i = 0; i < spots; i++)
	{
		if (!wolne[i])
		{
			DrawSurface(screen, obiekt[i].sciezka, obiekt[i].wsp_x, obiekt[i].wsp_y);
		}
	}
	if (frogger->wsp_y < 62)
	{
		bool parking=false;
		int licznik = 0;
		for (int i = 0; i < spots; i++)
		{
			if (frogger->wsp_x >= gniazdo[i].lewy_x && frogger->wsp_x <= gniazdo[i].prawy_x && wolne[i] == true)
			{
				if (licznik == 0)
				{
					licznik++;
					(*zycia)++;
					wolne[i] = false;
					parking = true;
					frogger->wsp_x = FROGGER_START_X;
					frogger->wsp_y = FROGGER_START_Y;
					DrawSurface(screen, frogger->sciezka, frogger->wsp_x, frogger->wsp_y);
					*worldTime = 0;
				}
				else
				{
					break;
				}
			}
		}
		if (!parking)
		{
			*kolizja = true;
		}
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	srand(time(NULL));
	bool kolizja = false;
	bool wolne[SPOTS];
	int t1, t2, quit, frames, rc, calkowity_czas=0;
	int liczba_zyc = LIVES;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed, froggerSpeed;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	obiekt_t plansza;
	obiekt_t frogger, fro[SPOTS];
	obiekt_t auto1[AUTA_1], auto2[AUTA_2], auto3[AUTA_3], auto4[AUTA_4], auto5[AUTA_5];
	obiekt_t belka3[BELKA_3], belka2[BELKA_2], belka1[BELKA_1], zolwie3[ZOLWIE_3], zolwie2[ZOLWIE_2];
	obiekt_t lives[LIVES];
	gniazdo_t gniazdo[SPOTS];

	for (int i = 0; i < SPOTS; i++)
	{
		wolne[i] = true;
	}


//-----------tworzenie ekranu-------------------------------------------------------

		// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
		// komunikaty wypisywane printf-em trzeba w opcjach:
		// project -> szablon2 properties -> Linker -> System -> Subsystem
		// zmieniæ na "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Gra frogger");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("img/cs8x8.bmp");
	SprawdzBledy(charset, screen, scrtex, window, renderer);
	SDL_SetColorKey(charset, true, 0x000000);

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	//frames = 0;
	//fpsTimer = 0;
	//fps = 0;
	quit = 0;
	worldTime = 0;
	delta = 0;
	//distance = 0;
	//etiSpeed = 1;

	//-------------frogger i plansza-------------

	Ustaw_Wartosci_Jednemu(&plansza, SDL_LoadBMP("img/plansza.bmp"), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - SCREEN_UP, SCREEN_WIDTH, SCREEN_HEIGHT);
	SprawdzBledy(plansza.sciezka, screen, scrtex, window, renderer);
	Ustaw_Wartosci_Jednemu(&frogger, SDL_LoadBMP("img/frog.bmp"), FROGGER_START_X, FROGGER_START_Y, 20, 20);
	SprawdzBledy(frogger.sciezka, screen, scrtex, window, renderer);


	Ustaw_Wartosci_Jednemu(&fro[0], SDL_LoadBMP("img/frog.bmp"), (LEFT_LAND_SPOT + RIGHT_LAND_SPOT) / 2, 48, 20, 20);
	Ustaw_Wartosci_Jednemu(&fro[1], SDL_LoadBMP("img/frog.bmp"), (4 * SPOT_DIFF + LEFT_LAND_SPOT + 4 * SPOT_DIFF + RIGHT_LAND_SPOT) / 2, 48, 20, 20);
	Ustaw_Wartosci_Jednemu(&fro[2], SDL_LoadBMP("img/frog.bmp"), (8 * SPOT_DIFF + LEFT_LAND_SPOT + 8 * SPOT_DIFF + RIGHT_LAND_SPOT) / 2, 48, 20, 20);
	Ustaw_Wartosci_Jednemu(&fro[3], SDL_LoadBMP("img/frog.bmp"), (12 * SPOT_DIFF + LEFT_LAND_SPOT + 12 * SPOT_DIFF + RIGHT_LAND_SPOT) / 2 + JUSTIFY, 48, 20, 20);
	Ustaw_Wartosci_Jednemu(&fro[4], SDL_LoadBMP("img/frog.bmp"), (16 * SPOT_DIFF + LEFT_LAND_SPOT + 16 * SPOT_DIFF + RIGHT_LAND_SPOT) / 2 + JUSTIFY, 48, 20, 20);

	for (int i = 0; i < NESTS_COUNT; i++)
	{
		SprawdzBledy(fro[i].sciezka, screen, scrtex, window, renderer);
	}

	//-------------samochodziki-----------------

	Ustaw_Wartosci_Obiektom(auto1, AUTA_1, SDL_LoadBMP("img/auto1.bmp"), SCREEN_HEIGHT / 2 + START_HEIGHT - SCREEN_UP, 25, 25, 70);
	SprawdzBledyGrupie(auto1, AUTA_1, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(auto2, AUTA_2, SDL_LoadBMP("img/auto2.bmp"), SCREEN_HEIGHT / 2 + START_HEIGHT - DIFF - SCREEN_UP, 26, 25, 70);
	SprawdzBledyGrupie(auto2, AUTA_2, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(auto3, AUTA_3, SDL_LoadBMP("img/auto3.bmp"), SCREEN_HEIGHT / 2 + START_HEIGHT - 2 * DIFF - SCREEN_UP, 29, 25, 80);
	SprawdzBledyGrupie(auto3, AUTA_3, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(auto4, AUTA_4, SDL_LoadBMP("img/auto4.bmp"), SCREEN_HEIGHT / 2 + START_HEIGHT - 3 * DIFF - SCREEN_UP, 25, 25, 70);
	SprawdzBledyGrupie(auto4, AUTA_4, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(auto5, AUTA_5, SDL_LoadBMP("img/auto5.bmp"), SCREEN_HEIGHT / 2 + START_HEIGHT - 4 * DIFF - SCREEN_UP, 63, 25, 120);
	SprawdzBledyGrupie(auto5, AUTA_5, screen, scrtex, window, renderer);


	//------------przeszkody wodne--------------

	Ustaw_Wartosci_Obiektom(belka2, BELKA_2, SDL_LoadBMP("img/belka2.bmp"), SCREEN_HEIGHT / 2 - START_HEIGHT + DIFF / 2 - SCREEN_UP, 126, 25, 180);
	SprawdzBledyGrupie(belka2, BELKA_2, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(zolwie2, ZOLWIE_2, SDL_LoadBMP("img/zolwie2.bmp"), SCREEN_HEIGHT / 2 - START_HEIGHT + 3 * DIFF / 2 - SCREEN_UP, 72, 25, 140);
	SprawdzBledyGrupie(zolwie2, ZOLWIE_2, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(belka3, BELKA_3, SDL_LoadBMP("img/belka3.bmp"), SCREEN_HEIGHT / 2 - START_HEIGHT + 5 * DIFF / 2 - SCREEN_UP, 215, 25, 330);
	SprawdzBledyGrupie(belka3, BELKA_3, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(belka1, BELKA_1, SDL_LoadBMP("img/belka1.bmp"), SCREEN_HEIGHT / 2 - START_HEIGHT + 7 * DIFF / 2 - SCREEN_UP, 103, 25, 170);
	SprawdzBledyGrupie(belka1, BELKA_1, screen, scrtex, window, renderer);

	Ustaw_Wartosci_Obiektom(zolwie3, ZOLWIE_3, SDL_LoadBMP("img/zolwie3.bmp"), SCREEN_HEIGHT / 2 - START_HEIGHT + 9 * DIFF / 2 - SCREEN_UP, 121, 25, 240);
	SprawdzBledyGrupie(zolwie3, ZOLWIE_3, screen, scrtex, window, renderer);


	//---------------¿ycia----------------------

	for (int i = 0; i < LIVES; i++)
	{
		Ustaw_Wartosci_Jednemu(&lives[i], SDL_LoadBMP("img/life.bmp"), i * LIVES_DIFF + LIVES_DIFF, LIVES_Y, 25, 25);
		SprawdzBledy(lives[i].sciezka, screen, scrtex, window, renderer);
	}


	//-------------gniazda----------------------

	for (int i = 0; i < SPOTS; i++)
	{
		gniazdo[i].lewy_x = 4 * i * SPOT_DIFF + LEFT_LAND_SPOT;
		gniazdo[i].prawy_x = 4 * i * SPOT_DIFF + RIGHT_LAND_SPOT;
		//DrawSurface(screen, fro[i].sciezka, gniazdo[i].lewy_x, 48);
	}



//---------------gra----------------------------

	while (!quit)
	{
		while (liczba_zyc)
		{
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			//		SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			t2 = SDL_GetTicks();

			// w tym momencie t2-t1 to czas w milisekundach,
			// jaki uplyna³ od ostatniego narysowania ekranu
			// delta to ten sam czas w sekundach
			//delta = (t2 - t1) * 0.001;
			//t1 = t2;

			//worldTime += delta;

			//calkowity_czas = (int)(worldTime);

			//distance += etiSpeed * delta;

			/*for (int i = 0; i < liczba_zyc; i++)
			{
				DrawSurface(screen, lives[0].sciezka, i * LIVES_DIFF + LIVES_DIFF, LIVES_Y);
			}*/

			//SDL_FillRect(screen, NULL, czarny);

		//---------plansza-----------------------------------

			DrawSurface(screen, plansza.sciezka, plansza.wsp_x, plansza.wsp_y);


		//---------przeszkody wodne---------------------------

			Rysuj_Jadace_W_Lewo(screen, belka2, BELKA_2, kolizja, &frogger);
			Rysuj_Jadace_W_Prawo(screen, zolwie2, ZOLWIE_2, kolizja, &frogger);
			Rysuj_Jadace_W_Lewo(screen, belka3, BELKA_3, kolizja, &frogger);
			Rysuj_Jadace_W_Prawo(screen, belka1, BELKA_1, kolizja, &frogger);
			Rysuj_Jadace_W_Lewo(screen, zolwie3, ZOLWIE_3, kolizja, &frogger);


		//--------samochodziki----------------------------------

			Rysuj_Jadace_W_Lewo(screen, auto1, AUTA_1, kolizja, &frogger);
			Rysuj_Jadace_W_Prawo(screen, auto2, AUTA_2, kolizja, &frogger);
			Rysuj_Jadace_W_Lewo(screen, auto3, AUTA_3, kolizja, &frogger);
			Rysuj_Jadace_W_Prawo(screen, auto4, AUTA_4, kolizja, &frogger);
			Rysuj_Jadace_W_Lewo(screen, auto5, AUTA_5, kolizja, &frogger);

		
		//----------¿ycia i czas---------------------------------------------

			DrawRectangle(screen, 0, BOARD_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT-BOARD_HEIGHT, czarny, czarny);
			for (int i = 0; i < liczba_zyc; i++)
			{
				if (i >= LIVES)
				{
					DrawSurface(screen, lives[0].sciezka, i * LIVES_DIFF + LIVES_DIFF, LIVES_Y);
				}
				else
				{
					DrawSurface(screen, lives[0].sciezka, i * LIVES_DIFF + LIVES_DIFF, LIVES_Y);
				}
			}

			int dlugosc_paska = PASEK_CZASU - calkowity_czas*PROPORCJA;
			
			if (dlugosc_paska > CZAS_KRYTYCZNY*PROPORCJA)
			{
				DrawRectangle(screen, SCREEN_WIDTH - 220, SCREEN_HEIGHT - 60, dlugosc_paska, 40, czerwony, zielony);
			}
			else if (dlugosc_paska <= CZAS_KRYTYCZNY*PROPORCJA && dlugosc_paska > 0)
			{
				DrawRectangle(screen, SCREEN_WIDTH - 220, SCREEN_HEIGHT - 60, dlugosc_paska, 40, czerwony, czerwony);
			}
			else if (dlugosc_paska <= 0)
			{
				kolizja = true;
				worldTime = 0;
			}

		//---------frogger-----------------------------------------

			DrawSurface(screen, frogger.sciezka, frogger.wsp_x, frogger.wsp_y);

			//fpsTimer += delta;
			//if (fpsTimer > 0.5) {
			//	fps = frames * 2;
			//	frames = 0;
			//	fpsTimer -= 0.5;
			//};


		//---------kolizje-----------------------------------------

			Kolizja_Wodna(frogger, zolwie3, ZOLWIE_3, &kolizja);
			Kolizja_Wodna(frogger, belka1, BELKA_1, &kolizja);
			Kolizja_Wodna(frogger, belka3, BELKA_3, &kolizja);
			Kolizja_Wodna(frogger, zolwie2, ZOLWIE_2, &kolizja);
			Kolizja_Wodna(frogger, belka2, BELKA_2, &kolizja);

			Kolizja_Samochodziki(frogger, auto1, AUTA_1, &kolizja);
			Kolizja_Samochodziki(frogger, auto2, AUTA_2, &kolizja);
			Kolizja_Samochodziki(frogger, auto3, AUTA_3, &kolizja);
			Kolizja_Samochodziki(frogger, auto4, AUTA_4, &kolizja);
			Kolizja_Samochodziki(frogger, auto5, AUTA_5, &kolizja);

			Kolizja_Z_Ekranem(frogger, &kolizja);

			Parkuj_Froggera(screen, &frogger, fro, gniazdo, SPOTS, wolne, &kolizja, &worldTime, &liczba_zyc);


		//---obs³uga zdarzeñ (o ile jakieœ zasz³y)-----

			if (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
				
					//-----komunikat o pauzie--------

					if (event.key.keysym.sym == SDLK_p)
					{
						delta = (t2 - t1) * 0.001;
						worldTime += delta;
						while (1)
						{
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);

							DrawRectangle(screen, 100, BOARD_HEIGHT / 2 - (BOARD_HEIGHT - 350) / 2, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 400, bialy, czarny);
							sprintf(text, "PAUSE");
							DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, BOARD_HEIGHT / 2 + 20, text, charset);

							if (SDL_PollEvent(&event))
							{
								if (event.type == SDL_KEYDOWN)
								{
									if (event.key.keysym.sym == SDLK_p)
									{
										t2 = SDL_GetTicks();
										t1 = t2;
										break;
									}
								}
							}
						}
					}


					//-----komunikat o koñcu gry--------

					else if (event.key.keysym.sym == SDLK_q)
					{
						delta = (t2 - t1) * 0.001;
						worldTime += delta;
						while (1)
						{
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);

							DrawRectangle(screen, 100, BOARD_HEIGHT / 2 - (BOARD_HEIGHT - 350) / 2, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 400, bialy, czarny);
							sprintf(text, "QUIT GAME? T/N");
							DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, BOARD_HEIGHT / 2 + 20, text, charset);

							if (SDL_PollEvent(&event))
							{
								if (event.type == SDL_KEYDOWN)
								{
									if (event.key.keysym.sym == SDLK_t)
									{
										/*SDL_DestroyTexture(scrtex);
										SDL_DestroyRenderer(renderer);
										SDL_DestroyWindow(window);

										SDL_Quit();*/
										liczba_zyc = 0;
										quit = 1;
										break;
									}
									else if (event.key.keysym.sym == SDLK_n)
									{
										t2 = SDL_GetTicks();
										t1 = t2;
										break;
									}
								}
							}
						}
					}


					//------poruszanie froggera oraz drobne korekty, gdy jest pomiêdzy drog¹ a wod¹

					else if (event.key.keysym.sym == SDLK_UP)
					{
						if (frogger.wsp_y == BOARD_HEIGHT / 2 + 25)
						{
							frogger.wsp_y = BOARD_HEIGHT / 2 - 5;
							frogger.wsp_x = SCREEN_WIDTH / 2;
						}
						else if (frogger.wsp_y <= UPPER_MIDDLE_EDGE)
						{
							frogger.wsp_y -= VERTICAL_SPEED_1;
						}
						else
						{
							frogger.wsp_y -= VERTICAL_SPEED_2;
						}
						if (frogger.wsp_y >= UPPER_MIDDLE_EDGE && frogger.wsp_y <= LOWER_MIDDLE_EDGE)
						{
							frogger.wsp_y = BOARD_HEIGHT / 2 - 5;
							frogger.wsp_x = SCREEN_WIDTH / 2;
						}
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						if (frogger.wsp_y == BOARD_HEIGHT / 2 - 5)
						{
							frogger.wsp_y = BOARD_HEIGHT / 2 + 25;
							frogger.wsp_x = SCREEN_WIDTH / 2;
						}
						else if (frogger.wsp_y <= UPPER_MIDDLE_EDGE)
						{
							frogger.wsp_y += VERTICAL_SPEED_1;
						}
						else
						{
							frogger.wsp_y += VERTICAL_SPEED_2;
						}
						if (frogger.wsp_y >= UPPER_MIDDLE_EDGE && frogger.wsp_y <= LOWER_MIDDLE_EDGE)
						{
							frogger.wsp_y = BOARD_HEIGHT / 2 + 25;
							frogger.wsp_x = SCREEN_WIDTH / 2;
						}
					}
					else if (event.key.keysym.sym == SDLK_RIGHT) frogger.wsp_x += HORIZONTAL_SPEED;
					else if (event.key.keysym.sym == SDLK_LEFT) frogger.wsp_x -= HORIZONTAL_SPEED;
					break;
				case SDL_KEYUP:
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};

			//frames++;
			Sleep(5);
			t2 = SDL_GetTicks();
			delta = (t2 - t1) * 0.001;
			t1 = t2;
			worldTime += delta;
			calkowity_czas = (int)(worldTime);

		//--repozycja, gdy nast¹pi³a kolizja----------

			while (kolizja)
			{
				liczba_zyc--;
				kolizja = false;
				worldTime = 0;
				frogger.wsp_x = FROGGER_START_X;
				frogger.wsp_y = FROGGER_START_Y;
			};
		}
	//-------------komunikat o koñcu gry-------------------------------------------

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		DrawRectangle(screen, 100, BOARD_HEIGHT/2 - (BOARD_HEIGHT - 400)/2, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 400, bialy, czarny);
		sprintf(text, "GAME OVER!!!");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, BOARD_HEIGHT / 2 + 20, text, charset);
		sprintf(text, "Zakonczyc gre? T/N");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, BOARD_HEIGHT / 2 + 45, text, charset);

	//------------ T-koniec gry, N-graj dalej---------------------------------------------------------------

		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				{
					if (event.key.keysym.sym == SDLK_n)
					{
						liczba_zyc = LIVES;
						worldTime = 0;
						kolizja = false;
						frogger.wsp_x = FROGGER_START_X;
						frogger.wsp_y = FROGGER_START_Y;
						for (int i = 0; i < SPOTS; i++)
						{
							wolne[i] = true;
						}
						break;
					}
					else if (event.key.keysym.sym == SDLK_t)
					{
						quit = 1;
						break;
					}
				}
			}
		}
	}

	// zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};
