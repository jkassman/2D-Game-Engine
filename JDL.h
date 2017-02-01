/*
	Jacob's DirectMedia Layer
*/
#ifndef JDL_H
#define JDL_H

#define MIDWAY_FIRE_KEY SDL_SCANCODE_F
#define MIDWAY_MOVE_KEY SDL_SCANCODE_M

#include <SDL.h>
#undef main
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

#include <list>

class JDL
{
public:
	JDL();
	~JDL();
	
	static bool init(int width, int height, char* title);
	static void close();
	static void setDrawColor(Uint8 r, Uint8 g, Uint8 b);
	static void setBackColor(Uint8 r, Uint8 g, Uint8 b);
	static void line(int x1, int y1, int x2, int y2);
	static void circle(int x, int y, int r);
	static void rect(int x, int y, int width, int height); //x and y is top left corner
	static void point(int x, int y);
	static int text(int x, int y, const char* textureText);
	static void fillRect(int x, int y, int w, int h);
	static char wait(int *x, int *y);

	//this eats all events. Consider changing when hotkeys are added
	static void clicked(int &x, int &y, int &click); //click is -1 for quit, 0 for none, 1 for left, 2 for right
	static void clear();
	static void flush();
	static void getInt(int &toChange, int x, int y);
	static void getDouble(double &toChange, int x, int y);
	static void fillPoly(std::vector<int> toDraw);
	static bool insidePoly(const std::vector<int> & polyPoints, double x, double y); //PolyPoints has positive y as up and positive x as right, and 0,0 inside

private:
	//the following functions are used only in getInt and getDouble. Possibly bad form, but I thought it was better than typing the same thing 10 times.
	static void numericKeyBlock(int digit, std::string digitString, std::string & toDisplay, int & toReturn, int & digitNum, bool &changed, int sign);
	static void numericKeyBlockDouble(int digit, std::string digitString, std::string & toDisplay,
		double & toReturn, int & digitNum, bool &changed, int sign);
	//bool loadFromRenderedText(char* textureText, int width);
	static std::list <SDL_Texture*> textList;
	static void saveTexture(SDL_Texture*);
	static void freeTextures();

	//Screen Dimensions
	static int SCREEN_WIDTH;
	static int SCREEN_HEIGHT;

	static Uint8 myDrawColor[3];
	static Uint8 myBackColor[3];

	//SDL Window, Renderer, (and font)
	static SDL_Window* gWindow;
	static SDL_Renderer* gRenderer;
	static TTF_Font* gFont;
};

#endif
