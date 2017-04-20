#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <time.h>
#include <vector>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <stdlib.h>
#include <SDL_mixer.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

using namespace std;
SDL_Window* displayWindow;
bool shot = false;

void DrawText(ShaderProgram* program, int font, string text, float size, float spacing) {

	float texture_size = 1.0 / 16.0f;

	vector<float> vertexData;
	vector<float> texCoordData;

	for (int i = 0; i < text.size(); ++i) {

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
	glBindTexture(GL_TEXTURE_2D, font);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

GLuint LoadTexture(const char* filePath) {

	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image.\n";
		//assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}

// Spritesheet
class SS {
public:
	SS(unsigned int textureID, float size) : textureID(textureID), size(size){
		width = 500.0 / 1024.0;
		height = 500.0 / 1024.0;
	}
	SS(unsigned int textureID, float u, float v, float width, float height, float size)
		: textureID(textureID), u(u / 1024), v(v / 1024), width(width / 1024), height(height / 1024), size(size) {}

	float getSize(){ return size; }
	void Draw(ShaderProgram* program) {

		glBindTexture(GL_TEXTURE_2D, textureID);

		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};

		float aspect = width / height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, -0.5f * size
		};

		glUseProgram(program->programID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}

	float u;
	float v;
	float size;
	float width;
	float height;
	unsigned int textureID;
};

// (Entity)
class GameObject {
public:
	GameObject(float x, float y, SS sprite) : x(x), y(y), sprite(sprite) {}

	SS sprite;
	float x;
	float y;
	float size = sprite.size;
	float speedx = 0.0;
	float speedy = 0.0;
	float gravity = 1.1;
	bool dead = false;

	bool cTop = false;
	bool cBottom = false;
	bool cLeft = false;
	bool cRight = false;

	bool isstatic = false;

};

// Container of items
vector<GameObject> enemies;
vector<GameObject> playerBullets;
vector<GameObject> enemyBullets;
vector<GameObject> blockies;

// Creates Enemies
void start(ShaderProgram* program, Matrix& modelMatrix, SS enemy) {
	/*GameObject enemyObj(-2.5, 5.3, enemy);
	enemyObj.speedx = 1.0;//0.5;
	enemyObj.speedy = 0.0;// 0.7;
	enemyObj.dead = false;
	modelMatrix.identity();
	modelMatrix.Translate(enemyObj.x, enemyObj.y, 0.0);
	program->setModelMatrix(modelMatrix);
	enemies.push_back(enemyObj);*/

	GameObject enemyObj1(2.5, 3.8, enemy);
	enemyObj1.speedx = 1.0;//0.5;
	enemyObj1.speedy = 0.0;// 0.7;
	enemyObj1.dead = false;
	modelMatrix.identity();
	modelMatrix.Translate(enemyObj1.x, enemyObj1.y, 0.0);
	program->setModelMatrix(modelMatrix);
	enemies.push_back(enemyObj1);

	GameObject enemyObj2(-1.75, 2.3, enemy);
	enemyObj2.speedx = 1.0;//0.5;
	enemyObj2.speedy = 0.0;// 0.7;
	enemyObj2.dead = false;
	modelMatrix.identity();
	modelMatrix.Translate(enemyObj2.x, enemyObj2.y, 0.0);
	program->setModelMatrix(modelMatrix);
	enemies.push_back(enemyObj2);

	GameObject enemyObj3(-1.75, -0.2, enemy);
	enemyObj3.speedx = 1.0;//0.5;
	enemyObj3.speedy = 0.0;// 0.7;
	enemyObj3.dead = false;
	modelMatrix.identity();
	modelMatrix.Translate(enemyObj3.x, enemyObj3.y, 0.0);
	program->setModelMatrix(modelMatrix);
	enemies.push_back(enemyObj3);
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	srand(time(NULL));
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	// Loading texture
	GLuint font = LoadTexture("font2.png");
	GLuint spriteTexture = LoadTexture("sheet.png");
	GLuint block = LoadTexture("houseBeige.png");

	SS player(spriteTexture, 211.0, 941.0, 99.0, 75.0, 0.4);
	SS enemy(spriteTexture, 224.0, 496.0, 103.0, 84.0, 0.3);
	SS playerBullet(spriteTexture, 856.0, 131.0, 9.0, 37.0, 0.3);
	SS enemyBullet(spriteTexture, 858.0, 475.0, 9.0, 37.0, 0.3);
	SS blocks(block, 0.3);

	enum GameState { GAME_STATE_TITLE, GAME_STATE_GAME, GAME_STATE_LOST, GAME_STATE_WON };
	GameState state = GAME_STATE_TITLE;

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0, 2.0, -1.0, 1.0);

	glUseProgram(program.programID);

	//start(&program, modelMatrix, enemy);

	// Initializing Objects
	GameObject playerObj(0.0f, -1.75f, player);
	playerObj.speedx = 1.75;
	playerObj.speedy = 2.1;
	playerObj.dead = false;

	GameObject playerBulletObj(0.0, -10.0, playerBullet);
	playerBulletObj.speedy = 0.0;
	playerBulletObj.dead = true;
	playerBullets.push_back(playerBulletObj);

	int numEnemiesAlive = enemies.size();
	float shootingDelay = 0.0;
	float playerSD = 0.0;
	float lastFrameTicks = 0.0;
	bool standing = true;
	float enemyDelay = 0.0;
	bool moving = false;
	bool moveup = true;
	bool fall = false;

	SDL_Event event;
	bool done = false;

	

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	Mix_Chunk *someSound;
	someSound = Mix_LoadWAV("hit.wav");
	Mix_Chunk *otherSound;
	otherSound = Mix_LoadWAV("begin.wav");
	Mix_Chunk *thirdSound;
	thirdSound = Mix_LoadWAV("die.wav");
	Mix_Music *music;
	music = Mix_LoadMUS("Elevator-music.mp3");
	Mix_PlayMusic(music, -1);
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		const Uint8* keys = SDL_GetKeyboardState(NULL);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;

		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);


		if (state == GAME_STATE_TITLE || state == GAME_STATE_LOST || state == GAME_STATE_WON) {
			if (keys[SDL_SCANCODE_RETURN]) {
				Mix_PlayChannel(-1, otherSound, 0);
				moveup = true;


				if (enemies.size() != 0){
					for (int i = 0; i < enemies.size(); ++i) {
						enemies.clear();
					}
				}
				else{
					start(&program, modelMatrix, enemy);
				}

				playerObj.x = 0.0;
				playerObj.y = -1.75;
				state = GAME_STATE_GAME;
			}
		}

		else if (state == GAME_STATE_GAME) {
			moving = false;
			for (int k = 0; k < 17; k++){

				GameObject blockObj(0.0, -10.0, blocks);
				blockObj.x = -3.2 + (k * blockObj.size);
				blockObj.y = -0.5;
				blockies.push_back(blockObj);
			}
			for (int k = 0; k < 17; k++){
				GameObject blockObj(0.0, -10.0, blocks);
				blockObj.x = -1.5 + (k * blockObj.size);
				blockObj.y = 2.0;
				blockies.push_back(blockObj);
			}

			for (int k = 0; k < 15; k++){

				GameObject blockObj(0.0, -10.0, blocks);
				blockObj.x = -3.2 + (k * blockObj.size);
				blockObj.y = 3.5;
				blockies.push_back(blockObj);
			}
			for (int k = 0; k < 15; k++){

				GameObject blockObj(0.0, -10.0, blocks);
				blockObj.x = -2.0 + (k * blockObj.size);
				blockObj.y = 5.0;
				blockies.push_back(blockObj);
			}

			for (int i = 0; i < blockies.size(); i++){
				modelMatrix.identity();
				modelMatrix.Translate(blockies[i].x, blockies[i].y, 0.0);
				program.setModelMatrix(modelMatrix);
				blockies[i].sprite.Draw(&program);
			}

			if (keys[SDL_SCANCODE_RIGHT]) {
				moving = true;
				if (playerObj.x < 3.25){
					//if (playerObj.cLeft){
					//playerObj.x += 0;
					//playerObj.y += elapsed * 1.1;
					//}
					//else{
					playerObj.x += elapsed * playerObj.speedx;
					//playerObj.y += elapsed * 1.1;
					//}
				}

			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				moving = true;
				if (playerObj.x > -3.25) {
					//if (playerObj.cRight){
					//playerObj.x += 0;
					//playerObj.y += elapsed * 1.1;
					//}
					//else{
					playerObj.x -= elapsed * playerObj.speedx;
					//playerObj.y += elapsed * 1.1;
					//}
				}
			}
			else if (keys[SDL_SCANCODE_UP]) {
				moving = true;
				moveup = true;

				if (!playerObj.cTop){
					playerObj.y += elapsed * playerObj.speedy;
				}
			}


			modelMatrix.identity();
			modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
			program.setModelMatrix(modelMatrix);
			playerObj.sprite.Draw(&program);

			if (keys[SDL_SCANCODE_W]) {
				if (playerSD > 0.15){
					Mix_PlayChannel(-1, someSound, 0);
					playerSD = 0.0;
					GameObject playerBulletObj(0.0, -10.0, playerBullet);
					playerBulletObj.speedy = 5.0;
					playerBulletObj.x = playerObj.x;
					playerBulletObj.y = playerObj.y;
					playerBulletObj.dead = false;
					playerBullets.push_back(playerBulletObj);
				}
			}
			if (keys[SDL_SCANCODE_A]) {
				if (playerSD > 0.15){
					Mix_PlayChannel(-1, someSound, 0);
					playerSD = 0.0;
					GameObject playerBulletObj(0.0, -10.0, playerBullet);
					playerBulletObj.speedx = -5.0;
					playerBulletObj.x = playerObj.x;
					playerBulletObj.y = playerObj.y;
					playerBulletObj.dead = false;
					playerBullets.push_back(playerBulletObj);
				}
			}
			if (keys[SDL_SCANCODE_D]) {
				if (playerSD > 0.15){
					Mix_PlayChannel(-1, someSound, 0);
					playerSD = 0.0;
					GameObject playerBulletObj(0.0, -10.0, playerBullet);
					playerBulletObj.speedx = 5.0;
					playerBulletObj.x = playerObj.x;
					playerBulletObj.y = playerObj.y;
					playerBulletObj.dead = false;
					playerBullets.push_back(playerBulletObj);
				}
			}
		}

		switch (state) {
		case GAME_STATE_TITLE:
			modelMatrix.identity();
			modelMatrix.Translate(-1.7f, 1.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "PLATFORMERS", 0.35f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.0f, 0.9f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "USE ARROW KEYS TO MOVE,", 0.20f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.0f, 0.6f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "AWD TO FIRE", 0.20f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.0f, 0.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "YOU CAN'T SHOOT THROUGH WALLS", 0.20f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.0f, -0.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "BUT YOUR ENEMIES CAN", 0.20f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.4f, -1.3f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "BE CAREFUL. DON'T TOUCH YOUR ENEMIES", 0.19f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.0f, -1.8f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "PRESS ENTER TO START", 0.25f, 0.0f);
			break;
		case GAME_STATE_LOST:
			viewMatrix.identity();
			modelMatrix.identity();
			modelMatrix.Translate(-1.25, 0.4, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "YOU LOSE", 0.25f, 0.00f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.0, 0.2, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "PRESS [ENTER] TO START", 0.25f, 0.0f);
			break;
		case GAME_STATE_WON:
			viewMatrix.identity();
			modelMatrix.identity();
			modelMatrix.Translate(-1.25, 0.4, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "YOU WIN", 0.25f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.0, 0.2, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "PRESS [ENTER] TO START", 0.25f, 0.0f);

			break;
		case GAME_STATE_GAME:
			//if (keys[SDL_SCANCODE_UP]) {

			//moveup = true;

			//if (playerObj.cTop){
			//moveup = false;
			//}
			//if (!playerObj.cTop){
			//playerObj.y += elapsed * playerObj.speedy;

			//}
			//}
			float transx = -playerObj.x;
			float transy = -playerObj.y;
			if (playerObj.x < 4.0 && playerObj.x > -4.0){
				transx = 0.0;
			}
			if (playerObj.y < -2.0 || playerObj.y > 5.5){
				transy = -5.5;
			}

			viewMatrix.identity();
			viewMatrix.Scale(1.0, 1.0, 1.0);
			viewMatrix.Translate(transx, transy, 0.0);
			program.setViewMatrix(viewMatrix);
			playerObj.cTop = false;
			playerObj.cBottom = false;
			playerObj.cLeft = false;
			playerObj.cRight = false;
			for (int i = 0; i < enemies.size(); i++){
				enemies[i].cTop = false;
				enemies[i].cBottom = false;
				enemies[i].cLeft = false;
				enemies[i].cRight = false;
			}

			modelMatrix.identity();
			modelMatrix.Translate(-3.3, 1.8, 0.0);
			program.setModelMatrix(modelMatrix);

			for (int i = 0; i < blockies.size(); ++i){
				if ((!(blockies[i].x + (blockies[i].size / 2) < playerObj.x - (playerObj.size / 1.5)
					|| blockies[i].x - (blockies[i].size / 2) > playerObj.x + (playerObj.size / 1.5)
					|| blockies[i].y + (blockies[i].size / 2) < playerObj.y - (playerObj.size / 1.5)
					|| blockies[i].y - (blockies[i].size / 2) > playerObj.y + (playerObj.size / 1.5)
					))){
					//moveup = true;


					if ((blockies[i].x - (blockies[i].size / 2) <= playerObj.x + (playerObj.size / 2))){

						playerObj.cRight = true;
						blockies[i].cLeft = true;
						//moveup = true;
					}
					//if (blockies[i].y + (blockies[i].size / 2) >= playerObj.y - (playerObj.size / 2)){
					if (blockies[i].y > playerObj.y){
						playerObj.cTop = true;
						blockies[i].cBottom = true;
						float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
						playerObj.y -= 0;

						//moveup = true;
					}
					if (blockies[i].x + (blockies[i].size / 2) >= playerObj.x - (playerObj.size / 2)){
						playerObj.cLeft = true;
						blockies[i].cRight = true;
						moveup = true;

					}
					//if (blockies[i].y + (blockies[i].size / 2) <= playerObj.y + (playerObj.size / 2)){
					if (blockies[i].y < playerObj.y){
						playerObj.cBottom = true;
						blockies[i].cTop = true;
						playerObj.y = blockies[i].y + blockies[i].size;
						moveup = true;
						moving = false;
						float penetration = fabs(blockies[i].y + (blockies[i].size / 2) - playerObj.y + (playerObj.size / 2));
						playerObj.y += penetration + 0.01;


					}

				}
			}
			for (int j = 0; j < enemies.size(); j++){

				if ((!(playerObj.x + (playerObj.size / 2) < enemies[j].x - (enemies[j].size / 1.5)
					|| playerObj.x - (playerObj.size / 2) > enemies[j].x + (enemies[j].size / 1.5)
					|| playerObj.y + (playerObj.size / 2) < enemies[j].y - (enemies[j].size / 1.5)
					|| playerObj.y - (playerObj.size / 2) > enemies[j].y + (enemies[j].size / 1.5)
					))){
					Mix_PlayChannel(-1, thirdSound, 0);
					state = GAME_STATE_LOST;
				}
			}
			if (playerObj.y > -1.75){ // gravity
				if (!playerObj.cBottom && !moving){
					playerObj.cTop = false;
					moveup = false;
					playerObj.y -= playerObj.gravity *elapsed;
					fall = true;
					playerObj.isstatic = true;
				}

			}
			if (playerObj.y < -1.7){
				moveup = true;
			}

			for (int i = 0; i < enemies.size(); i++){
				if (enemies[i].y > -1.75){
					if (!enemies[i].cBottom){
						enemies[i].y -= enemies[i].speedy * elapsed;
					}
				}
				else{
					Mix_PlayChannel(-1, thirdSound, 0);
					state = GAME_STATE_LOST;
				}
			}



			modelMatrix.identity();
			modelMatrix.Translate(playerObj.x, playerObj.y, 0.0);
			program.setModelMatrix(modelMatrix);
			playerObj.sprite.Draw(&program);

			for (int i = 0; i < playerBullets.size(); ++i) {
				playerBullets[i].y += elapsed * playerBullets[i].speedy;
				playerBullets[i].x += elapsed * playerBullets[i].speedx;
				modelMatrix.identity();
				modelMatrix.Translate(playerBullets[i].x, playerBullets[i].y, 0.0);
				program.setModelMatrix(modelMatrix);
				if (!playerBullets[i].dead){
					playerBullets[i].sprite.Draw(&program);
				}
				else{
					playerBullets.erase(playerBullets.begin() + i);
				}
			}
			for (int i = 0; i < playerBullets.size(); i++){
				if (playerBullets[i].y >= 7.0 || playerBullets[i].x < -4.0 || playerBullets[i].x > 4.0) {
					playerBullets[i].dead = true;
					playerBullets[i].x = -10.0;
					playerBullets[i].y = -10.0;
					playerBullets[i].speedy = 0.0;
					playerBullets[i].speedx = 1.0;
				}
				for (int j = 0; j < blockies.size(); ++j){
					if ((!(blockies[j].x + (blockies[j].size / 2) < playerBullets[i].x - (playerBullets[i].size / 1.5)
						|| blockies[j].x - (blockies[j].size / 2) > playerBullets[i].x + (playerBullets[i].size / 1.5)
						|| blockies[j].y + (blockies[j].size / 2) < playerBullets[i].y - (playerBullets[i].size / 1.5)
						|| blockies[j].y - (blockies[j].size / 2) > playerBullets[i].y + (playerBullets[i].size / 1.5)
						))){

						playerBullets[i].dead = true;
						playerBullets[i].x = -10.0;
						playerBullets[i].y = -10.0;
						playerBullets[i].speedy = 0.0;
						playerBullets[i].speedx = 1.0;
					}
				}

				for (int j = 0; j < enemies.size(); ++j) {
					if (!(playerBullets[i].x + (playerBullets[i].size / 2) <= enemies[j].x - (enemies[j].size / 2)
						|| playerBullets[i].x - (playerBullets[i].size / 2) >= enemies[j].x + (enemies[j].size / 2)
						|| playerBullets[i].y + (playerBullets[i].size / 2) <= enemies[j].y - (enemies[j].size / 2)
						|| playerBullets[i].y - (playerBullets[i].size / 2) >= enemies[j].y + (enemies[j].size / 2))) {
						playerBullets[i].dead = true;
						playerBullets[i].x = 10.0;
						playerBullets[i].speedy = 0.0;
						enemies[j].y = 10.0;
						enemies[j].dead = true;
						numEnemiesAlive--;
					}
				}
			}
			for (int i = 0; i < enemies.size(); ++i) {
				if (!enemies[i].dead) {
					enemies[i].x += elapsed * enemies[i].speedx;
					modelMatrix.identity();
					modelMatrix.Translate(enemies[i].x, enemies[i].y, 0);
					program.setModelMatrix(modelMatrix);
					enemies[i].sprite.Draw(&program);
				}
			}

			for (int i = 0; i < enemies.size(); ++i) {
				if (enemies[i].x > 3.35) {
					//for (int i = 0; i < enemies.size(); ++i) {
					enemies[i].speedx = -0.75;
					//}
				}
				if (enemies[i].x < -3.35) {
					//for (int i = 0; i < enemies.size(); ++i) {
					enemies[i].speedx = 0.75;
					//}
				}
			}
			shootingDelay += elapsed;
			playerSD += elapsed;
			if (shootingDelay > 0.55) {
				if (enemies.size() != 0){
					int randomEnemy = rand() % enemies.size();
					while (enemies[randomEnemy].dead == true){
						randomEnemy = rand() % enemies.size();
					}

					shootingDelay = 0;
					GameObject enemyBulletObj(0.0, -10.0, enemyBullet);
					enemyBulletObj.speedy = 2.0;
					enemyBulletObj.x = enemies[randomEnemy].x;
					enemyBulletObj.y = enemies[randomEnemy].y - (enemyBulletObj.size / 2) - (enemies[randomEnemy].size / 2);
					enemyBulletObj.dead = false;
					enemyBullets.push_back(enemyBulletObj);
				}
				else{
					start(&program, modelMatrix, enemy);
				}
			}
			for (int i = 0; i < enemyBullets.size(); i++){
				enemyBullets[i].y -= elapsed * enemyBullets[i].speedy;
				modelMatrix.identity();
				modelMatrix.Translate(enemyBullets[i].x, enemyBullets[i].y, 0.0);
				program.setModelMatrix(modelMatrix);
				if (!enemyBullets[i].dead){
					enemyBullets[i].sprite.Draw(&program);
				}
				else{
					enemyBullets.erase(enemyBullets.begin() + i);
				}

				if (!(enemyBullets[i].x + (enemyBullets[i].size / 2) <= playerObj.x - (playerObj.size / 2)
					|| enemyBullets[i].x - (enemyBullets[i].size / 2) >= playerObj.x + (playerObj.size / 2)
					|| enemyBullets[i].y + (enemyBullets[i].size / 2) <= playerObj.y - (playerObj.size / 2)
					|| enemyBullets[i].y - (enemyBullets[i].size / 2) >= playerObj.y + (playerObj.size / 2))) {
					enemyBullets[i].y = -10.0;
					enemyBullets[i].speedy = 0.0;
					// we died
					playerObj.x = 0.0f;
					playerObj.y = -1.75f;
					numEnemiesAlive = enemies.size();
					Mix_PlayChannel(-1, thirdSound, 0);
					state = GAME_STATE_LOST;
				}
			}

			if (numEnemiesAlive == 0) {

				playerObj.x = 0.0f;
				playerObj.y = -1.75f;
				numEnemiesAlive = enemies.size();
				state = GAME_STATE_WON;
			}
			break;
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
};