#define WIN32_LEAN_AND_MEAN
#include <string>
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <commdlg.h>
#include <basetsd.h>
#include <objbase.h>

#include "Box2D/Box2D.h"

#ifdef USE_DIRECTX_SDK
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\include\xinput.h>
#pragma comment(lib,"xinput.lib")
#elif (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <XInput.h>
#pragma comment(lib,"xinput.lib")
#else
#include <XInput.h>
#pragma comment(lib,"xinput9_1_0.lib")
#endif

#pragma comment(lib, "user32")
using namespace System;

XINPUT_STATE state;
const int NUMBER_OF_PLATFORMS = 60;
HANDLE hOut, newScreenBuffer;
CONSOLE_SCREEN_BUFFER_INFO SBInfo;
int FRAME_LENGTH = 1000 / 60;
std::string screenBuffer = "";

b2Vec2 gravity(0.0f, 100.0f);
b2World world(gravity);

float32 timeStep = 1.0f / 60.0f;
int32 velocityIterations = 6;
int32 positionIterations = 2;

void GoToXY(int column, int line)
{
	COORD coord;
	coord.X = column;
	coord.Y = line;
	if (!SetConsoleCursorPosition(hOut, coord)) //TODO handle error
	{
		std::cout << "AHHH";
	}
}

/*struct charNode
{
	char data;
	charNode *next;
};

class list //i did not wright this code, url to source:https://www.codementor.io/codementorteam/a-comprehensive-guide-to-implementation-of-singly-linked-list-using-c_plus_plus-ondlm5azr
{
private:
	charNode *head, *tail;
public:
	list()
	{
		head = NULL;
		tail = NULL;
	}
	void insert_position(int pos, int value)
	{
		charNode *pre = new charNode;
		charNode *cur = new charNode;
		charNode *temp = new charNode;
		cur = head;
		for (int i = 1; i<pos; i++)
		{
			pre = cur;
			cur = cur->next;
		}
		temp->data = value;
		pre->next = temp;
		temp->next = cur;
	}
	void display()
	{
		charNode *temp = new charNode;
		temp = head;
		while (temp != NULL)
		{
			std::cout << temp->data;
			temp = temp->next;
		}
	}
	//this list lacks the ability to shrink, doing so would cause issues, hence the feature is not supported
};*/

void displayCharecter(int x,int y, char c) {
	//width 240
	//height 84
	
	screenBuffer.at((fmax(fmin(x, Console::LargestWindowWidth-1), 0)) +
		             Console::LargestWindowWidth *
		            (fmax(fmin(y, Console::LargestWindowHeight-1), 0))) = c;
}

class Platform {
public:
	int x = 20;
	int y = 15;
	b2Body *groundBody;
	Platform() {
		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(x, y);
		groundBody = world.CreateBody(&groundBodyDef);
		b2PolygonShape groundBox;
		groundBox.SetAsBox(4.0f, 4.0f);
		groundBody->CreateFixture(&groundBox, 0.0f);
	}
	void fixPos() {
		groundBody->SetTransform(b2Vec2(x, y),0);
	}
	char fullHPPlatform[4][4] = {
	{ 'Ö','Ä','Ä','·' },
	{ 'º','²','²','º' },
	{ 'º','²','²','º' },
	{ 'Ó','Ä','Ä','½' },
	};
	void display() {
		for (int i = 0; i < 4; i++) {
			//GoToXY(x, y+i);
			for (int j = 0; j < 4; j++) {
				displayCharecter(x + j, y + i, fullHPPlatform[i][j]);
			}
		}
	}
};

class Player {
	b2Body *body;
public:
	Player(int x, int y) {
		
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(x, y);
		body = world.CreateBody(&bodyDef);
		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(2.0f, 4.0f);
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;
		body->CreateFixture(&fixtureDef);
	}
	
	float x = 0.0f;
	float y = 0.0f;
	float velX = 0.1f;
	float velY = 0.0f;
	CHAR_INFO playerSprite[32] =
	{
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 },
	{ 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 } , { 'a', 0x07 }
	};

	/*char playerSprite[8][4] = {
	{ ' ',' ',' ',' ' },
	{ ' ','_',' ',' ' },
	{ '|',' ','\\','_' },
	{ '\\','_','_','/' },
	{ ' ','|',' ',' ' },
	{ ' ','|',' ',' ' },
	{ ' ','|',' ',' ' },
	{ ' ','|',' ',' ' },

	};*/
	void impulse(float x, float y) {
		/*velX += x;//DEPRECIATED
		velY += y;*/
		body->ApplyForce(b2Vec2(x,y), b2Vec2(this->x, this->y), true);
	}

	void update() { //catches up the position of the sprite with the physics position of the object
		/*b2Vec2 position = body->GetPosition();
		x = position.x;
		y = position.y;*///TODO uncomment this
	}

	void display() {
		/*for (int i = 0; i < 8; i++) {//DEPRECIATED
			//GoToXY(x, y+i);
			for (int j = 0; j < 4; j++) {
				displayCharecter(x + j, y + i, playerSprite[i][j]);//
			}
		}*/
		COORD screenSize;
		screenSize.X = Console::LargestWindowWidth;
		screenSize.Y = Console::LargestWindowHeight;
		COORD orgin;
		orgin.X = 0;
		orgin.Y = 0;
		SMALL_RECT location;
		location.Left = x;
		location.Top = y;
		location.Right = x+4;
		location.Bottom = y+8;
		SetConsoleActiveScreenBuffer(newScreenBuffer);
		bool val = WriteConsoleOutput(newScreenBuffer, playerSprite, screenSize, orgin, &location);
		auto err = GetLastError(); 
		assert(val);
		SetConsoleActiveScreenBuffer(hOut);
	}
};

void update(Platform p[], Player &player) {
	world.Step(timeStep, velocityIterations, positionIterations);
	XInputGetState(0, &state);
	float x = ((float)state.Gamepad.sThumbLX<1500 && (float)state.Gamepad.sThumbLX>-1500) ? 0 :
		((float)state.Gamepad.sThumbLX / 32767);
	float y = ((float)state.Gamepad.sThumbLY<1500 && (float)state.Gamepad.sThumbLY>-1500) ? 0 :
		((float)state.Gamepad.sThumbLY / 32767);
	for (int i = 0; i < NUMBER_OF_PLATFORMS; i++) {
		p[i].display();
	}
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
	{
		player.impulse(0, -50000);
	}
	player.impulse(x*4500, 0);
	player.update();
	player.display();
	GoToXY(0, 0);
	printf(screenBuffer.c_str());
	memset(&screenBuffer[0],' ', screenBuffer.size() * sizeof screenBuffer[0]);
	std::fflush(stdout);
}

int main() { //main game loop
	newScreenBuffer = CreateConsoleScreenBuffer(GENERIC_READ |           // read/write access 
												GENERIC_WRITE,
												FILE_SHARE_READ |
												FILE_SHARE_WRITE,        // shared 
												NULL,                    // default security attributes 
												CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
												NULL);                   // reserved; must be NULL
	assert(newScreenBuffer != INVALID_HANDLE_VALUE);
	
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	if (XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		printf("yea");
	}
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	/*RECT c;
	GetWindowRect(console, &c);

	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, console);*/
	/*COORD Coord;
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, &Coord);*/
	//GAME INIT
	SetConsoleActiveScreenBuffer(newScreenBuffer);
	system("mode 650");
	std::string cmd = std::string("MODE CON COLS=") + std::to_string(Console::LargestWindowWidth) +
		std::string("  LINES=" + std::to_string(Console::LargestWindowHeight));
	screenBuffer.resize(Console::LargestWindowWidth * Console::LargestWindowHeight);
	system(cmd.c_str());
	SetConsoleActiveScreenBuffer(hOut);
	system("mode 650");
	HWND console = GetConsoleWindow();
	system(cmd.c_str());
	Platform pArr[NUMBER_OF_PLATFORMS];
	for (int i = 0; i < NUMBER_OF_PLATFORMS; i++) {
		pArr[i].x = 4 * i;
		pArr[i].y = 25;
		pArr[i].fixPos();
	}
	Player player(4,17);
	//GAME START
	while (true) {
		//std::thread game(update, 0); TODO: figure this out
		update(pArr, player);
		//Sleep(FRAME_LENGTH);
	}

	return(0);
}
