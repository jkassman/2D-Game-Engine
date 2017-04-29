#include <stdio.h>
#include <iostream>
#include <math.h>
#include <algorithm>

#define _USE_MATH_DEFINES

#include "JDL.hpp"

#ifdef JDL_USE_SDL
#include <Windows.h>

#else
#include "gfx_j.h"
#endif

const double JDL::PRECISION = 0.1;

JDL::JDL()
{
}


JDL::~JDL()
{
}

bool JDL::doublesEqual(double d1, double d2)
{
    if ((d1 >= (d2 - PRECISION)) && (d1 <= (d2 + PRECISION)))
    {
        return true;
    }
    return false;
}

int JDL::roundi(double toRound)
{
    if (toRound < 0)
    {
        return (int) (toRound - 0.5);
    }
    else
    {
        return (int) (toRound + 0.5);
    }
}


//calculates the angle between the given point and the x axis.
double JDL::calculateTheta(double x, double y) {
    switch (location(x,y)) {
    case 1:
    case 2:
        return -M_PI/2 + atan(x/y);
    case 3:
    case 4:
        return M_PI/2 + atan(x/y);
    case 0:
        return 0;
    case -1:
       	return (x>0) ? 0:M_PI;
    case -2:
        return (y>0) ? -M_PI/2: M_PI/2;
    }
    return 0; //should never get here.
}

double JDL::randDouble(double rangeStart, double rangeEnd)
{
    double toReturn = (double) rand() / RAND_MAX;
    toReturn *= (rangeEnd - rangeStart);
    toReturn += rangeStart;
    return toReturn;
}

bool JDL::percentChance(double chance)
{
    if (chance < 0) 
    {
        std::cerr << "Negative percent given to percentChance! Always False!" 
                  << std::endl;
        return false;
    }
    if (chance > 100)
    {
        std::cerr << "chance over 100 given to percentChance! Always True!"
                  << std::endl;
        return true;
    }
    
    double random = randDouble(0, 100);
    if (random < chance) return true;
    return false;
}

double JDL::stringToDouble(std::string toConvert)
{
#ifdef JDL_USE_STL
    return stod(toConvert);
#else
    return atof(toConvert.c_str());
#endif
}

int JDL::stringToInt(std::string toConvert)
{
#ifdef JDL_USE_STD
    return stoi(toConvert);
#else
    return atoi(toConvert.c_str());
#endif
}

//return 0 if origin, -1 if x axis, -2 if y axis, or quadrant the point is in
int JDL::location(double x, double y) {
    bool xIsZero = doublesEqual(0, x);
    bool yIsZero = doublesEqual(0, y);
    if (xIsZero && yIsZero)
            return 0;
    if (yIsZero)
            return -1;
    if (xIsZero)
            return -2;
    if (y > 0)
            return (x > 0)? 1:2;
    else //if  y < 0
            return (x < 0)? 3:4;
}

#ifdef JDL_USE_SDL
bool JDL::init(int width, int height, const char* title)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}

				//Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
				// Global Font initialization
				gFont = TTF_OpenFont("OCRAEXT.TTF", 12);
				if (gFont == NULL)
				{
					printf("Failed to load OCR Extended font! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}
	return success;
}


void JDL::setDrawColor(Uint8 r, Uint8 g, Uint8 b)
{
	SDL_SetRenderDrawColor(gRenderer, r, g, b, 255);
	myDrawColor[0] = r;
	myDrawColor[1] = g;
	myDrawColor[2] = b;
}

void JDL::setBackColor(Uint8 r, Uint8 g, Uint8 b)
{
	myBackColor[0] = r;
	myBackColor[1] = g;
	myBackColor[2] = b;
}

void JDL::clear()
{
	SDL_SetRenderDrawColor(gRenderer, myBackColor[0], myBackColor[1], myBackColor[2], 0xFF);
	SDL_RenderClear(gRenderer);
	SDL_SetRenderDrawColor(gRenderer, myDrawColor[0], myDrawColor[1], myDrawColor[2], 0xFF);
}

void JDL::line(int x1, int y1, int x2, int y2)
{
	SDL_RenderDrawLine(gRenderer, x1, y1, x2, y2);
}

void JDL::circle( int n_cx, int n_cy, int radius)
{
	// if the first pixel in the screen is represented by (0,0) (which is in sdl)
	// remember that the beginning of the circle is not in the middle of the pixel
	// but to the left-top from it:

	double error = (double)-radius;
	double x = (double)radius - 0.5;
	double y = (double)0.5;
	double cx = n_cx - 0.5;
	double cy = n_cy - 0.5;

	while (x >= y)
	{
		SDL_RenderDrawPoint( gRenderer, (int)(cx + x), (int)(cy + y));
		SDL_RenderDrawPoint(gRenderer, (int)(cx + y), (int)(cy + x));

		if (x != 0)
		{
			SDL_RenderDrawPoint(gRenderer, (int)(cx - x), (int)(cy + y));
			SDL_RenderDrawPoint(gRenderer, (int)(cx + y), (int)(cy - x));
		}

		if (y != 0)
		{
			SDL_RenderDrawPoint(gRenderer, (int)(cx + x), (int)(cy - y));
			SDL_RenderDrawPoint(gRenderer, (int)(cx - y), (int)(cy + x));
		}

		if (x != 0 && y != 0)
		{
			SDL_RenderDrawPoint(gRenderer, (int)(cx - x), (int)(cy - y));
			SDL_RenderDrawPoint(gRenderer, (int)(cx - y), (int)(cy - x));
		}

		error += y;
		++y;
		error += y;

		if (error >= 0)
		{
			--x;
			error -= x;
			error -= x;
		}
	}
}

//(x,y) is bottom left corner
void JDL::fillRect(int x, int y, int w, int h)
{
	SDL_Rect fillRect = { x, y-h, w, h};
	SDL_RenderFillRect(gRenderer, &fillRect);
}

void JDL::point(int x, int y)
{
	SDL_RenderDrawPoint(gRenderer, x, y);
}

//(x,y) is bottom left corner
//returns 0 if failed. returns width of text if successful.
int JDL::text(int x, int y, const char* textureText)
{
	int width = 1000; //dangerous dummy thingy, no wrapping yet.
	//Get rid of preexisting texture
	//free();
	SDL_Color textColor = { myDrawColor[0], myDrawColor[1], myDrawColor[2] };
	SDL_Texture* mTexture;
	mTexture = NULL;

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(gFont, textureText, textColor, width);
	if (textSurface != NULL)
	{
		//Create texture from surface pixels
		//textVector.push_back(SDL_CreateTextureFromSurface(gRenderer, textSurface));
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}

	//Return success
	if (mTexture == NULL)
		return 0;

	//render the text to a certain portion of the screen
	SDL_Rect viewport;
	viewport.x = x;
	viewport.y = y-textSurface->h;
	viewport.w = textSurface->w;
	viewport.h = textSurface->h;
	//SDL_RenderSetViewport(gRenderer, &viewport);
	SDL_RenderCopy(gRenderer, mTexture, NULL, &viewport);
	
	//Reset viewport to entire screen
	/*viewport.x = 0;
	viewport.y = 0;
	viewport.w = SCREEN_WIDTH;
	viewport.h = SCREEN_HEIGHT;
	SDL_RenderSetViewport(gRenderer, &viewport);*/

	saveTexture(mTexture);
	//delete[] mTexture;
	return textSurface->w;
}

void JDL::saveTexture(SDL_Texture* toSave)
{
	textList.push_back(toSave);
}

void JDL::freeTextures()
{
	std::list <SDL_Texture*>::iterator it;
	for (it = textList.begin(); it != textList.end(); ++it)
	{
		SDL_DestroyTexture(*it);
	}
	textList.erase(textList.begin(), textList.end());
}

//returns the character that was pressed,
//or 1 on left click and 3 on right click (for consistancy with gfx_j.h)
//0 is unrecognized event
char JDL::wait(int *x, int *y)
{
	SDL_Event e;
	while (1)
	{
		SDL_WaitEvent(&e);
		SDL_GetMouseState(x, y);
		if (e.type == SDL_TEXTINPUT)
		{
			return (char)e.text.text[0];
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				return 1;
			}
			else if (e.button.button == SDL_BUTTON_RIGHT)
			{
				return 3;
			}
		}
	}
}

//warning: eats non-text or non-mouseclick events.
int JDL::event_waiting()
{
    int numEvents = 0;
    SDL_Event e;
    SDL_PumpEvents();
    numEvents = SDL_PeepEvents(&e, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

    //eat all events that we don't want.
    while ( (numEvents > 0) && (e.type != SDL_TEXTINPUT) && (e.type != SDL_MOUSEBUTTONDOWN))
    {
        SDL_WaitEvent(&e);
        numEvents = SDL_PeepEvents(&e, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
    }
    return numEvents;
}

void JDL::sleep(double seconds)
{
    Sleep(roundi(1000*seconds));
}

//click is 0 for none, 1 for left, 2 for right
//3 for fire, 4 for move (draw outline)
//click 401 to 599 is reserved for scroll
//600-603 is for arrow keys
//-3 is quit
void JDL::clicked(int &x, int &y, int &click)
{
	click = 0;
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			click = -3;
			break;
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				click = 1;
			}
			else if (e.button.button ==SDL_BUTTON_RIGHT)
			{
				click = 2;
			}
			break;
		}
		else if (e.type == SDL_MOUSEWHEEL)
		{
			
			click = e.wheel.y + 500;
			if (click > 599)
			{
				click = 599;
			}
			if (click < 401)
			{
				click = 401;
			}
			while (SDL_PeepEvents(&e, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) != 0)
			{
				if (e.type == SDL_MOUSEWHEEL)
				{
					SDL_PollEvent(&e);
					click += e.wheel.y;
					if (click > 599)
					{
						click = 599;
						break;
					}
					if (click < 401)
					{
						click = 401;
						break;
					}
				}
				else
				{
					break; //NOTE RETURN HERE
				}
			}
			break;
		}
		else if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.scancode == MIDWAY_FIRE_KEY)
			{
				click = 3;
				break;
			}
			else if (e.key.keysym.scancode == MIDWAY_MOVE_KEY)
			{
				click = 4;
				break;
			}
			else if (e.key.keysym.scancode == SDL_SCANCODE_UP)
			{
				click = 600;
				break;
			}
			else if (e.key.keysym.scancode == SDL_SCANCODE_DOWN)
			{
				click = 601;
				break;
			}
			else if (e.key.keysym.scancode == SDL_SCANCODE_LEFT)
			{
				click = 602;
				break;
			}
			else if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
			{
				click = 603;
				break;
			}
		}
	}
	SDL_GetMouseState(&x, &y);
}


void JDL::getMouseLocation(int &x, int &y)
{
    SDL_GetMouseState(&x, &y);
}

void JDL::flush()
{
	SDL_RenderPresent(gRenderer);
	freeTextures();
}

void JDL::close()
{
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

//x and y is top left corner
void JDL::rect(int x, int y, int width, int height)
{
	line(x, y, x + width, y);
	line(x + width, y, x + width, y + height);
	line(x + width, y + height, x, y + height);
	line(x, y + height, x, y);
}

void JDL::getInt(int &toChange, int x, int y)
{
	bool changed = true;
	int toReturn = 0;
	int sign = 1;
	int digitNum = 0;
	std::string toDisplay = " ";
	SDL_Event e;

	while (true) //will exit by return
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					return;
				}
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_0:	case SDL_SCANCODE_KP_0:
					numericKeyBlock(0, "0", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_1:	case SDL_SCANCODE_KP_1:
					numericKeyBlock(1, "1", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_2:	case SDL_SCANCODE_KP_2:
					numericKeyBlock(2, "2", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_3:	case SDL_SCANCODE_KP_3:
					numericKeyBlock(3, "3", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_4:	case SDL_SCANCODE_KP_4:
					numericKeyBlock(4, "4", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_5:	case SDL_SCANCODE_KP_5:
					numericKeyBlock(5, "5", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_6:	case SDL_SCANCODE_KP_6:
					numericKeyBlock(6, "6", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_7:	case SDL_SCANCODE_KP_7:
					numericKeyBlock(7, "7", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_8:	case SDL_SCANCODE_KP_8:
					numericKeyBlock(8, "8", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_9:	case SDL_SCANCODE_KP_9:
					numericKeyBlock(9, "9", toDisplay, toReturn, digitNum, changed, sign);
					break;
				default:
					break;
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
				{
					toReturn = toReturn / 10;
					if (digitNum == 0 && sign == -1)
					{
						sign = 1;
						toDisplay.pop_back();
					}
					if (digitNum > 0)
					{
						toDisplay.pop_back();
						digitNum--;
					}
					changed = true;
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_MINUS || e.key.keysym.scancode == SDL_SCANCODE_KP_MINUS
					&& digitNum == 0)
				{
					if (sign != -1 && digitNum == 0)
					{
						toDisplay = "-";
						sign = -1;
						changed = true;
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_RETURN || e.key.keysym.scancode == SDL_SCANCODE_RETURN2
					|| e.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)
				{
					if (digitNum > 0)
					{
						toChange = toReturn * sign;
					}
					return;
				}
			}
		}
		if (changed)
		{
			JDL::setDrawColor(0, 0, 0);
			//std::cout << "Trying to draw Text" << std::endl;
			int clearWidth = 7 * (digitNum + 1);
			if (clearWidth < 30) clearWidth = 30;
			JDL::fillRect(x, y, clearWidth, 15);
			JDL::setDrawColor(255, 0, 255);
			if (toDisplay.size() == 0)
			{
				toDisplay = " ";
			}
			JDL::text(x, y, toDisplay.c_str());
			JDL::flush();
			changed = false;
		}
  		Sleep(30);
	}
}

//I think this works now, but if I had to do again, I would only create the string, then
//convert that string to a double at the last step.
void JDL::getDouble(double &toChange, int x, int y)
{
	bool changed = true;
	double toReturn = 0;
	int sign = 1;
	int digitNum = 0;
	std::string toDisplay = " ";
	SDL_Event e;

	while (true) //will exit by return
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					return;
				}
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_0:	case SDL_SCANCODE_KP_0:
					numericKeyBlockDouble(0, "0", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_1:	case SDL_SCANCODE_KP_1:
					numericKeyBlockDouble(1, "1", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_2:	case SDL_SCANCODE_KP_2:
					numericKeyBlockDouble(2, "2", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_3:	case SDL_SCANCODE_KP_3:
					numericKeyBlockDouble(3, "3", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_4:	case SDL_SCANCODE_KP_4:
					numericKeyBlockDouble(4, "4", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_5:	case SDL_SCANCODE_KP_5:
					numericKeyBlockDouble(5, "5", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_6:	case SDL_SCANCODE_KP_6:
					numericKeyBlockDouble(6, "6", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_7:	case SDL_SCANCODE_KP_7:
					numericKeyBlockDouble(7, "7", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_8:	case SDL_SCANCODE_KP_8:
					numericKeyBlockDouble(8, "8", toDisplay, toReturn, digitNum, changed, sign);
					break;
				case SDL_SCANCODE_9:	case SDL_SCANCODE_KP_9:
					numericKeyBlockDouble(9, "9", toDisplay, toReturn, digitNum, changed, sign);
					break;
				default:
					break;
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_PERIOD || e.key.keysym.scancode == SDL_SCANCODE_KP_PERIOD)
				{
					if (digitNum > 0)
					{
						digitNum = -1;
						toDisplay += ".";
						changed = true;
					}
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
				{
					int temp;
					if (digitNum >= 0)
					{
						temp = (int)toReturn;
					}
					else
					{
						temp = (int)(toReturn * pow(10, -(digitNum + 1)));
					}
					
					temp = temp / 10;
					if (digitNum >= 0)
					{
						toReturn = temp;
					}
					else
					{
						toReturn = (double)temp / pow(10, -(digitNum+2));
					}
					if (digitNum == 0 && sign == -1)
					{
						sign = 1;
						toDisplay.pop_back();
					}
					if (digitNum > 0)
					{
						toDisplay.pop_back();
						digitNum--;
					}
					if (digitNum < 0)
					{
						toDisplay.pop_back();
						digitNum++;
						if (digitNum == 0)
						{
							digitNum = log10(toReturn) + 1;
						}
					}
					changed = true;
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_MINUS || e.key.keysym.scancode == SDL_SCANCODE_KP_MINUS
					&& digitNum == 0)
				{
					if (sign != -1 && digitNum == 0)
					{
						toDisplay = "-";
						sign = -1;
						changed = true;
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_RETURN || e.key.keysym.scancode == SDL_SCANCODE_RETURN2
					|| e.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)
				{
					if (digitNum != 0)
					{
						toChange = toReturn * sign;
					}
					return;
				}
			}
		}
		if (changed)
		{
			JDL::setDrawColor(0, 0, 0);
			//std::cout << "Trying to draw Text" << std::endl;
			int clearWidth = 7 * (toDisplay.size() + 1);
			if (clearWidth < 30) clearWidth = 30;
			JDL::fillRect(x, y, clearWidth, 15);
			JDL::setDrawColor(255, 0, 255);
			if (toDisplay.size() == 0)
			{
				toDisplay = " ";
			}
			JDL::text(x, y, toDisplay.c_str());
			JDL::flush();
			changed = false;
		}
		Sleep(30);
	}
}

void JDL::numericKeyBlock(int digit, std::string digitString, std::string & toDisplay, int & toReturn, int & digitNum, bool &changed, int sign)
{
	if (digitNum == 0 && sign != -1)
	{
		toDisplay = digitString;
	}
	else
	{
		toDisplay += digitString;
	}
	toReturn *= 10;
	toReturn += digit;
	digitNum++;
	changed = true;
}

void JDL::numericKeyBlockDouble(int digit, std::string digitString, std::string & toDisplay, double & toReturn,
	int & digitNum, bool &changed, int sign)
{
	if (digitNum == 0 && sign != -1)
	{
		toDisplay = digitString;
	}
	else
	{
		toDisplay += digitString;
	}
	if (digitNum >= 0)
	{
		toReturn *= 10;
		toReturn += digit;
		digitNum++;
	}
	else
	{
		toReturn += (double) digit * pow(10, digitNum);
		digitNum--;
	}
	changed = true;
}

//x and y are in the same coordinate system as polyPoints.
//PolyPoints has positive y as up and positive x as right
//0,0 must be inside!
bool JDL::insidePoly(const std::vector<int> & polyPoints, double x, double y)
{
	int nextX, nextY;
	double m, yLine, yInt;
	for (size_t i = 0; i < (polyPoints.size()); i += 2)
	{
		if (i + 3 >= polyPoints.size())
		{
			nextX = 0; nextY = 1;
		}
		else
		{
			nextX = i + 2; nextY = i + 3;
		}
		if (polyPoints[i] - polyPoints[nextX] != 0)
		{
			//calculate the slope
			m = (double)(polyPoints[i + 1] - polyPoints[nextY]) / (polyPoints[i] - polyPoints[nextX]);
			//calculate the y intercept
			yInt = -m*polyPoints[i] + polyPoints[i + 1];
			yLine = m*x + yInt;
			if (yInt > 0)
			{
				if (y > yLine)
				{
					return false;
				}
			}
			else
			{
				if (y < yLine)
				{
					return false;
				}
			}
		}
		else
		{
			if (polyPoints[i] > 0)
			{
				if (x > polyPoints[i])
				{
					return false;
				}
			}
			else
			{
				if (x < polyPoints[i])
				{
					return false;
				}
			}
		}
	}
	return true;
}

//uses the same code that ship's inside function does.
//toDraw's points are in the screen's coordinates (-y up, +x right)
//order of toDraw matters!
//note: makes a copy and modifies it so that all ys are negative what they were. consider updating
void JDL::fillPoly(std::vector<int> toDraw)
{
	int centerX = 0, centerY = 0;
	int bigX = -2000000000, smallX = 2000000000, bigY = -2000000000, smallY = 2000000000;
	for (size_t i = 0; i < toDraw.size(); ++i)
	{
		if (i % 2) //it's a y coordinate
		{
			toDraw[i] = -toDraw[i];
			centerY += toDraw[i];
			if (toDraw[i] < smallY)
			{
				smallY = toDraw[i];
			}
			if (toDraw[i] > bigY)
			{
				bigY = toDraw[i];
			}
		}
		else //it's an x coordinate
		{
			centerX += toDraw[i];
			if (toDraw[i] < smallX)
			{
				smallX = toDraw[i];
			}
			if (toDraw[i] > bigX)
			{
				bigX = toDraw[i];
			}
		}
	}
	centerX = centerX / ((int)toDraw.size() / 2);
	centerY = centerY / ((int) toDraw.size() / 2);
	for (size_t i = 0; i < toDraw.size(); ++i)
	{
		if (i % 2) //it's a y coordinate
		{
			toDraw[i] -= centerY;
		}
		else //it's an x coordinate
		{
			toDraw[i] -= centerX;
		}
	}
	//std::cout << "smallY is " << smallY << " and bigY is " << bigY << std::endl;
	//std::cout << "centerX is " << centerX << " and centerY is " << centerY << std::endl;
	for (int x = smallX - centerX; x < bigX - centerX; ++x)
	{
		for (int y = smallY - centerY; y < bigY - centerY; ++y)
		{
			if (insidePoly(toDraw, x, y))
			{
				//std::cout << "(" << x << "," << y << ")" << std::endl;
				point(x + centerX, -(y + centerY));
			}
		}
	}
}

SDL_Window* JDL::gWindow = NULL;
SDL_Renderer* JDL::gRenderer = NULL;
TTF_Font* JDL::gFont = NULL;

Uint8 JDL::myDrawColor[] = { 0xFF, 0xFF, 0xFF };
Uint8 JDL::myBackColor[] = { 0, 0, 0 };
std::list <SDL_Texture*> JDL::textList;

#else

bool JDL::init(int width, int height, const char *title)
{
    gfx_open(width, height, title);
    return true;
}

void JDL::flush()
{
    gfx_flush();
}

void JDL::line(int x1, int y1, int x2, int y2)
{
    gfx_line(x1, y1, x2, y2);
}

void JDL::clear()
{
    gfx_clear();
}

char JDL::wait(int *x, int *y)
{
    char toReturn;
    toReturn = gfx_wait();
    *x = gfx_xpos();
    *y = gfx_ypos();
    return toReturn;
}

void JDL::getMouseLocation(int &x, int &y)
{
    gfx_update_cursor();
    x = gfx_xNow();
    y = gfx_yNow();
}

int JDL::event_waiting()
{
    return gfx_event_waiting();
}

void JDL::sleep(double seconds)
{
    usleep(roundi(1000000*seconds));
}

void JDL::circle(int x, int y, int r)
{
    gfx_circle(x, y, r);
}

void JDL::point(int x, int y)
{
    gfx_point(x, y);
}

//TODO: incompatible, doesn't return width;
int JDL::text(int x, int y, const char *text)
{
    gfx_text(x, y, text);
    return 1;
}

void JDL::setDrawColor(int r, int g, int b)
{
    gfx_color(r, g, b);
}

#endif
int JDL::SCREEN_WIDTH = 800;
int JDL::SCREEN_HEIGHT = 800;

