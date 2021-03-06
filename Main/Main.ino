//For Arduino Mega

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <URTouch.h>
#include <stdint.h>
#include "Vec2.h"
#include "Color_16.h"
#include "Board.h"
#include "GameStatus.h"

//Defining the display pins
#define TFT_CS 53
#define TFT_DC 9
#define TFT_MOSI 51
#define TFT_CLK 52
#define TFT_RST 22
#define TFT_MISO 50

//Defining the touch interface pin
#define t_SCK 26
#define t_CS 27
#define t_MOSI 28
#define t_MISO 29
#define t_IRQ 30

#define FLAG_BTN_PIN 33

//Change this according to the screen
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define BORDER_THICCNESS 10
#define BORDER_COLOR 0x001F //BLUE
#define NUMBER_BOMBS 15

uint16_t getCorrectedY(uint16_t Y)
{
	return (SCREEN_HEIGHT - Y) - 5;
}

Vec2<uint16_t> touchCoordinates;
GameStatus gameStatus = GameStatus::Playing;
bool isEndScreenDisplayed = false;
bool trueInputFlag = false;

//!!DO NOT USE PIN 2 FOR ANYTHING, IT IS USED AS THE SEED INPUT FOR THE RANDOM GENERATION OF BOMB POSITIONS!!

Adafruit_ILI9341 scrn = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
URTouch touchIF(t_SCK, t_CS, t_MOSI, t_MISO, t_IRQ);
Board brd(scrn, SCREEN_WIDTH, SCREEN_HEIGHT, BORDER_THICCNESS, NUMBER_BOMBS, gameStatus, Color_16{ (uint16_t)BORDER_COLOR });

void setup()
{
	pinMode(FLAG_BTN_PIN, INPUT);

	//Initializing the screen
	scrn.begin();
	scrn.setRotation(2);

	//Initilizing the touch interface
	touchIF.InitTouch(PORTRAIT);
	touchIF.setPrecision(PREC_EXTREME);

	//Setting up the game board
	Vec2<unsigned short int> bombCoords[NUMBER_BOMBS];
	randomSeed(analogRead(2));
	for (unsigned char i = 0; i < NUMBER_BOMBS; i++)
	{
		bool isDuplicate = false;
		Vec2<unsigned short int> thisBombPos{ (unsigned short int)random(0, NUMBER_CELLS_H), (unsigned short int)random(0, NUMBER_CELLS_V) };
		for (int j = 0; j < i; j++)
		{
			if (thisBombPos == bombCoords[j])
			{
				isDuplicate = true;
				--i;
				break;
			}
		}
		if (isDuplicate) continue;
		else
		{
			bombCoords[i] = thisBombPos;
		}
	}
	brd.Setup(bombCoords); 
}

void loop()
{
	if (gameStatus == GameStatus::Playing)
	{
		while (touchIF.dataAvailable())
		{
			touchIF.read();
			touchCoordinates = Vec2<uint16_t>{ touchIF.getX(), getCorrectedY(touchIF.getY()) };
			bool tempFlagBTNState = digitalRead(FLAG_BTN_PIN); // This is done so that we don't capture FLAG_BTN_PIN twice leading to some unexpected bugs
			if (tempFlagBTNState && !trueInputFlag)
			{
				trueInputFlag = true;
				brd.FlagInput(touchCoordinates);
			}
			else if (!tempFlagBTNState)
			{
				trueInputFlag = false;
				brd.TouchInput(touchCoordinates);
			}
			if (gameStatus == GameStatus::Lost || gameStatus == GameStatus::Won) break;
		}
		trueInputFlag = false;
	}
	else if (gameStatus == GameStatus::Lost && !isEndScreenDisplayed)
	{
		isEndScreenDisplayed = true;
		scrn.fillRect(BORDER_THICCNESS, BORDER_THICCNESS, SCREEN_WIDTH - BORDER_THICCNESS * 2, SCREEN_HEIGHT - BORDER_THICCNESS * 2, Color_16::RGB888_TO_RGB565(0xFF0000));
		scrn.setTextSize(4);
		scrn.setTextColor(0xFFFF);
		scrn.setCursor(SCREEN_WIDTH / 2 - 2 * 5 * 4 - 5, SCREEN_HEIGHT / 2 - 4 * 7 - 10);
		scrn.print("GAME");
		scrn.setCursor(SCREEN_WIDTH / 2 - 2 * 5 * 4 - 5, SCREEN_HEIGHT / 2 + 10);
		scrn.print("OVER");
	}
	else if (gameStatus == GameStatus::Won && !isEndScreenDisplayed)
	{ 
		isEndScreenDisplayed = true;
		scrn.fillRect(BORDER_THICCNESS, BORDER_THICCNESS, SCREEN_WIDTH - BORDER_THICCNESS * 2, SCREEN_HEIGHT - BORDER_THICCNESS * 2, Color_16::RGB888_TO_RGB565(0x00FF00));
		scrn.setTextSize(4);
		scrn.setTextColor(0xFFFF);
		scrn.setCursor(SCREEN_WIDTH / 2 - 2 * 3 * 4 - 5, SCREEN_HEIGHT / 2 - 4 * 7 - 10);
		scrn.print("YOU");
		scrn.setCursor(SCREEN_WIDTH / 2 - 2 * 3 * 4 - 5, SCREEN_HEIGHT / 2 + 10);
		scrn.print("WON");
	}
}