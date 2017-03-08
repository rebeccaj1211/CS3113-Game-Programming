#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STD_IMAGE_IMPLEMENTATION
#include <Windows.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <random>
#include <cstdlib>
#include <ctime>
#include "time.h"
//#include "assert.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;

// SDL & rendering objects
SDL_Window* displayWindow;
GLuint font;
GLuint sprite;
Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;
class Entity;
float texture_coords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f }; 

enum GameState { MAIN_MENU, GAME_LEVEL, GAME_WON, GAME_LOST };
bool game = true;
float lastFrameTicks = 0.0f;
float elapsed;
float timeSinceLastFire = 0.0f;
float timeSinceLastEnemyFire = 0.0f;
bool moveleft = false;
bool moveright = false;
bool bullet = false;

int state;
ShaderProgram* program;

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}

	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
GLuint LoadTexture(const char* image_path) {
	SDL_Surface* surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}
class Entity {
public:
	float positionx;
	float positiony;
	float top;
	float bottom;
	float left;
	float right;
	float speedx;
	float speedy;
	Matrix entityMatrix;
	float u;
	float v;
	float width;
	float height;
	float size = 1.0f;

	Entity() {}

	Entity(float x, float y, float spriteU, float spriteV, float spriteWidth, float spriteHeight, float dx, float dy) {
		positionx = x;
		positiony = y;
		speedx = dx;
		speedy = dy;
		entityMatrix.identity();
		entityMatrix.Translate(x, y, 0);
		top = y + 0.05f * size*3;
		bottom = y - 0.05f * size*3;
		left = x - 0.05f * size*3;
		right = x + 0.05f * size*3;

		u = spriteU / 1024.0f;
		v = spriteV / 1024.0f;
		width = spriteWidth / 1024.0f;
		height = spriteHeight / 1024.0f;
	}

	void draw() {
		entityMatrix.identity();
		entityMatrix.Translate(positionx, positiony, 0);
		entityMatrix.Scale(2.0f, 2.0f, 1.0f);
		program->setModelMatrix(entityMatrix);

		vector<float> vertexData;
		vector<float> texCoordData;
		float text_x = u;
		float text_y = v;
		vertexData.insert(vertexData.end(), {
			(-0.1f * size), 0.1f * size,
			(-0.1f * size), -0.1f * size,
			(0.1f * size), 0.1f * size,
			(0.1f * size), -0.1f * size,
			(0.1f * size), 0.1f * size,
			(-0.1f * size), -0.1f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			text_x, text_y,
			text_x, text_y + height,
			text_x + width, text_y,
			text_x + width, text_y + height,
			text_x + width, text_y,
			text_x, text_y + height,
		});

		glUseProgram(program->programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, sprite);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}

};

Entity player;
vector<Entity> enemies;
vector<Entity> playerbullets;
vector<Entity> enemybullets;

void UpdateGameLevel(float elapsed) {

	if (moveleft) {
		if (player.positionx > -3.8) {
			player.positionx -= player.speedx * elapsed;
			player.left -= player.speedx * elapsed;
			player.right -= player.speedx * elapsed;
		}
	}
	else if (moveright) {
		if (player.positionx < 3.8) {
			player.positionx += player.speedx * elapsed;
			player.left += player.speedx * elapsed;
			player.right += player.speedx * elapsed;
		}
	}
	if (bullet) {
		if (timeSinceLastFire > 0.5f) {
			timeSinceLastFire = 0;
			playerbullets.push_back(Entity(player.positionx, player.positiony, 849.0f, 310.0f, 13.0f, 37.0f, 0, 4.0f));
		}
	}
	for (size_t i = 0; i < enemies.size(); i++) {
		enemies[i].positionx += enemies[i].speedx * elapsed;
		enemies[i].left += enemies[i].speedx * elapsed;
		enemies[i].right += enemies[i].speedx * elapsed;

		if ((enemies[i].right > 3.7f && enemies[i].speedx > 0) || (enemies[i].left < -3.7f && enemies[i].speedx < 0)) {
			for (size_t i = 0; i < enemies.size(); i++) {
				enemies[i].speedx = -enemies[i].speedx;
				enemies[i].positiony -= 0.2;
				enemies[i].top -= 0.2;
				enemies[i].bottom -= 0.2;
			}
		}

		if (enemies[i].bottom < player.top &&
			enemies[i].top > player.bottom &&
			enemies[i].left < player.right &&
			enemies[i].right > player.left) {
			state = GAME_LOST;
		}
		if (enemies[i].positiony <= player.positiony) {
			state = GAME_LOST;
		}
	}

	vector<int> removePlayerBullets;

	for (size_t i = 0; i < playerbullets.size(); i++) {
		playerbullets[i].positiony += playerbullets[i].speedy * elapsed;
		playerbullets[i].top += playerbullets[i].speedy * elapsed;
		playerbullets[i].bottom += playerbullets[i].speedy * elapsed;

		for (size_t j = 0; j < enemies.size(); j++) {
			if (enemies[j].bottom < playerbullets[i].top &&
				enemies[j].top > playerbullets[i].bottom &&
				enemies[j].left < playerbullets[i].right &&
				enemies[j].right > playerbullets[i].left) {
				enemies.erase(enemies.begin() + j);
				removePlayerBullets.push_back(i);
			}
		}
	}

	for (int i = 0; i < removePlayerBullets.size(); i++) {
		playerbullets.erase(playerbullets.begin() + removePlayerBullets[i] - i);
	}

	if (timeSinceLastEnemyFire > 0.4f) {
		timeSinceLastEnemyFire = 0;
		int shooter = rand() % enemies.size();
		enemybullets.push_back(Entity(enemies[shooter].positionx, enemies[shooter].positiony, 856.0f, 421.0f, 9.0f, 54.0f, 0, -2.0f));
	}

	for (size_t i = 0; i < enemybullets.size(); i++) {
		enemybullets[i].positiony += enemybullets[i].speedy * elapsed;
		enemybullets[i].top += enemybullets[i].speedy * elapsed;
		enemybullets[i].bottom += enemybullets[i].speedy * elapsed;
		if (enemybullets[i].bottom < player.top &&
			enemybullets[i].top > player.bottom &&
			enemybullets[i].left < player.right &&
			enemybullets[i].right > player.left) {
			state = GAME_LOST;
		}
	}

	if (enemies.size() == 0) {
		state = GAME_WON;
		
	}
}
void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state) {
	case MAIN_MENU:
		modelMatrix.identity();
		modelMatrix.Translate(-1.5f, 2.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, font, "SPACE INVADERS", 0.35f, 0.0f);

		modelMatrix.identity();
		modelMatrix.Translate(-3.0f, 0.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, font, "USE ARROW KEYS TO MOVE,", 0.25f, 0.0f);

		modelMatrix.identity();
		modelMatrix.Translate(-3.0f, 0.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, font, "SPACE TO FIRE", 0.25f, 0.0f);

		modelMatrix.identity();
		modelMatrix.Translate(-1.6f, -2.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, font, "PRESS SPACE TO START", 0.25f, 0.0f);
		break;
	case GAME_LEVEL:
		player.draw();
		for (size_t i = 0; i < enemies.size(); i++) {
			enemies[i].draw();
		}
		for (size_t i = 0; i < playerbullets.size(); i++) {
			playerbullets[i].draw();
		}

		for (size_t i = 0; i < enemybullets.size(); i++) {
			enemybullets[i].draw();
		}
		break;
	case GAME_WON:
		glClear(GL_COLOR_BUFFER_BIT);
		modelMatrix.identity();
		modelMatrix.Translate(-2.0f, 0.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, font, "You won!", 0.7f, 0.0f);
		game = false;
		break;
	case GAME_LOST:
		glClear(GL_COLOR_BUFFER_BIT);
		modelMatrix.identity();
		modelMatrix.Translate(-2.0f, 0.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, font, "You lost", 0.7f, 0.0f);
		game = false;
		break;
	}
	SDL_GL_SwapWindow(displayWindow);
}


int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	SDL_Event event;
	bool done = false;

	projectionMatrix.setOrthoProjection(-4.0, 4.0, -2.25f, 2.25f, -1.0f, 1.0f);
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	font = LoadTexture("font2.png");
	sprite = LoadTexture("sheet.png");
	player = Entity(0.0f, -2.0f, 535.0f, 150.0f, 33.0f, 26.0f, 3.0f, 0);
	for (int i = 0; i < 18; i++) {
		enemies.push_back(Entity(-2.5 + (i % 6) * 1.0, 2.0 - (i / 6 * 0.5), 143.0f, 293.0f, 104.0f, 84.0f, 1.0f, 0.03f));
	}

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				done = true;
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (state == MAIN_MENU) {
						state = GAME_LEVEL;
					}
					else {
						bullet = true;
					}
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT && player.left > -3.5f) {
					moveleft = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && player.right < 3.5f) {
					moveright = true;
				}
				break;
			case SDL_KEYUP:
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					moveleft = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					moveright = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					bullet = false;
				}
				break;
			}
		}
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		timeSinceLastFire += elapsed;
		timeSinceLastEnemyFire += elapsed;

		if (game) {
			if (state == GAME_LEVEL) {
				UpdateGameLevel(elapsed);
			}
			Render();
		}
	}
	SDL_Quit();
	return 0;
}