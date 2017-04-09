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
#include <algorithm>
#include <Windows.h>


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


class Vector{
public:
	Vector(){}
	Vector(float fx, float fy, float fz):x(fx),y(fy),z(fz){}
	float length()const{
		return (sqrtf(pow(x, 2) + pow(y, 2) + pow(z, 2)));
	}
	void normalize(){
		if (length() > 0){
			x = x / length();
			y = y / length();
			z = z / length();
		}
	}
	Vector operator* (const Matrix &mat){
		Vector vec;
		//vec.x = vec.x * mat.m[0][0] + vec.y * mat.m[1][0] + vec.z * mat.m[2][0] + mat.m[3][0];
		//vec.y = vec.x * mat.m[0][1] + vec.y * mat.m[1][1] + vec.z * mat.m[2][1] + mat.m[3][1];
		//vec.z = vec.x * mat.m[0][2] + vec.y * mat.m[1][2] + vec.z * mat.m[2][2] + mat.m[3][2]; // these are 0
		vec.x = this->x * mat.m[0][0] + this->y * mat.m[1][0] + this->z * mat.m[2][0] + mat.m[3][0];
		vec.y = this->x * mat.m[0][1] + this->y * mat.m[1][1] + this->z * mat.m[2][1] + mat.m[3][1];
		return vec;
	}

	float x;
	float y;
	float z;
};
bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2, Vector &penetration);
bool penetrationSort(const Vector &p1, const Vector &p2);
bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points, Vector &penetration);

class Entity{
public:
	Entity(){}
	Entity(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5, float x6, float y6){
		vert.push_back(Vector(x1, y1, 0));
		vert.push_back(Vector(x2, y2, 0));
		vert.push_back(Vector(x3, y3, 0));
		vert.push_back(Vector(x4, y4, 0));
		vert.push_back(Vector(x5, y5, 0));
		vert.push_back(Vector(x6, y6, 0));
		pos.x = x1 + x2 / 2;
		pos.y = y2 + y3 / 2;
		pos.z = 0;
		velx = 0;
		vely = 0;
		//scale = Vector(1, 1, 1);
	}
	Matrix matrix;
	Vector pos;
	Vector scale;
	float rotation;
	float velx;
	float vely;

	std::vector<Vector> vert;
	std::vector<Vector> world;
	Vector pen;

	void boaderCollision(){
		if (pos.x > 6 || pos.x < -6){
			velx = -velx;
			if (pos.x > 5){
				pos.x -= 0.1;
			}
			else{
				pos.x += 0.1;
			}
			//vely = -vely;
		}
		if (pos.y > 3 || pos.y < -3){
			//velx = -velx;
			vely = -vely;
			if (pos.y > 2){
				pos.x -= 0.1;
			}
			else{
				pos.y += 0.1;
			}
		}
		
	}
	void SATboarder(Entity top, Entity bottom, Entity left, Entity right){
		if (checkSATCollision(world, top.world, pen)) {
			//pen.normalize();
			//enemy1.pos.x += (pen.x/4);
			pos.y -= 0.1;
			velx = -velx;
			vely = -vely;
		}
		if (checkSATCollision(world, bottom.world, pen)) {
			//pen.normalize();
			//enemy1.pos.x += (pen.x/4);
			pos.y += 0.1;
			velx = -velx;
			vely = -vely;
		}
		if (checkSATCollision(world, left.world, pen)) {
			//pen.normalize();
			//enemy1.pos.x += (pen.x/4);
			pos.x += 0.1;
			velx = -velx;
			vely = -vely;
		}
		if (checkSATCollision(world, right.world, pen)) {
			//pen.normalize();
			//enemy1.pos.x += (pen.x/4);
			pos.x -= 0.1;
			velx = -velx;
			vely = -vely;
		}
	}
	void worldCoord(std::vector<Vector> &worldCoord){
		worldCoord.resize(vert.size());
		for (int i = 0; i < vert.size(); i++) {
			float x = vert[i].x;
			float y = vert[i].y;
			worldCoord[i].x = matrix.m[0][0] * x + matrix.m[1][0] * y + matrix.m[3][0];
			worldCoord[i].y = matrix.m[0][1] * x + matrix.m[1][1] * y + matrix.m[3][1];
		}
	}
	void makeWorld(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5, float x6, float y6){
		world.resize(6);
		world[0].x = x1;
		world[0].y = y1;
		world[1].x = x2;
		world[1].y = y2;
		world[2].x = x3;
		world[2].y = y3;
		world[3].x = x4;
		world[3].y = y4;
		world[4].x = x5;
		world[4].y = y5;
		world[5].x = x6;
		world[5].y = y6;
		}
	void update(float elapsed){
		boaderCollision();
		pos.x += velx * elapsed;
		pos.y += vely * elapsed;		
		
	}
	void render(ShaderProgram * program, GLuint texture){
		//identity
		//transformations
		//program.setModelMatrix
		//draw
		matrix.identity();
		matrix.Scale(scale.x, scale.y, scale.z);
		matrix.Translate(pos.x, pos.y, pos.z);
		matrix.Rotate(rotation);
		worldCoord(world);
		float myArr[] = { vert[0].x, vert[0].y, vert[1].x, vert[1].y, vert[2].x, vert[2].y, vert[3].x, vert[3].y, vert[4].x, vert[4].y, vert[5].x, vert[5].y };
		
		program->setModelMatrix(matrix);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, myArr);
		glEnableVertexAttribArray(program->positionAttribute);

		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
		//glDisableVertexAttribArray(program->texCoordAttribute);
		}

};
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

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2, Vector &penetration) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];

	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p >= 0) {
		return false;
	}

	float penetrationMin1 = e1Max - e2Min;
	float penetrationMin2 = e2Max - e1Min;

	float penetrationAmount = penetrationMin1;
	if (penetrationMin2 < penetrationAmount) {
		penetrationAmount = penetrationMin2;
	}

	penetration.x = normalX * penetrationAmount;
	penetration.y = normalY * penetrationAmount;

	return true;
}

bool penetrationSort(const Vector &p1, const Vector &p2) {
	return p1.length() < p2.length();
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points, Vector &penetration) {
	std::vector<Vector> penetrations;
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}
		Vector penetration;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
		if (!result) {
			return false;
		}
		penetrations.push_back(penetration);
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		Vector penetration;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);

		if (!result) {
			return false;
		}
		penetrations.push_back(penetration);
	}

	std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
	penetration = penetrations[0];

	Vector e1Center;
	for (int i = 0; i < e1Points.size(); i++) {
		e1Center.x += e1Points[i].x;
		e1Center.y += e1Points[i].y;
	}
	e1Center.x /= (float)e1Points.size();
	e1Center.y /= (float)e1Points.size();

	Vector e2Center;
	for (int i = 0; i < e2Points.size(); i++) {
		e2Center.x += e2Points[i].x;
		e2Center.y += e2Points[i].y;
	}
	e2Center.x /= (float)e2Points.size();
	e2Center.y /= (float)e2Points.size();

	Vector ba;
	ba.x = e1Center.x - e2Center.x;
	ba.y = e1Center.y - e2Center.y;

	if ((penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f) {
		penetration.x *= -1.0f;
		penetration.y *= -1.0f;
	}

	return true;
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
	GLuint playerimg = LoadTexture("blue.png");
	GLuint block = LoadTexture("houseBeige.png");
	GLuint block2 = LoadTexture("green.png");


	enum GameState { GAME_STATE_TITLE, GAME_STATE_GAME, GAME_STATE_LOST, GAME_STATE_WON };
	GameState state = GAME_STATE_TITLE;

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0, 2.0, -1.0, 1.0);

	glUseProgram(program.programID);


	// Initializing Objects
	Entity player(1,1,-1,1,-1,-1,-1,-1,1,-1,1,1);
	Entity enemy1(1, 1, -1, 1, -1, -1, -1, -1, 1, -1, 1, 1);
	Entity enemy2(1, 1, -1, 1, -1, -1, -1, -1, 1, -1, 1, 1);
	
	player.scale.x = 0.5;
	player.scale.y = 0.5;
	enemy1.scale.x = 0.5;
	enemy1.scale.y = 0.5;
	enemy2.scale.x = 0.5;
	enemy2.scale.y = 0.5;

	player.velx = -1;
	player.vely = 1;
	enemy1.velx = -1;
	enemy1.vely = -1;
	enemy2.velx = -1;
	enemy2.vely = -1;

	Entity top;
	top.makeWorld(4, 3, -4, 3, -4, 2, -4, 2, 4, 2, 4, 3);
	Entity bottom;
	bottom.makeWorld(4, -2, -4, -2, -4, -3, -4, -3, 4, -3, 4, -2);
	Entity left;
	left.makeWorld(-4, 3, -5, 3, -5, -3, -5, -3, -4, -3, -4, 3);
	Entity right;
	right.makeWorld(5, 3, 4, 3, 4, -3, 4, -3, 5, -3, 5, 3);

	Vector pen;
	

	float lastFrameTicks = 0.0;
	SDL_Event event;
	bool done = false;
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
			player.update(FIXED_TIMESTEP);
			enemy1.update(FIXED_TIMESTEP);
			enemy2.update(FIXED_TIMESTEP);
		}
	
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		
		float angle = 30 * elapsed;
		enemy1.rotation += -1 * 2.0f*angle*(3.1415926 / 180);
		enemy2.rotation += -1 * 2.0f*-angle*(3.1415926 / 180);

		player.update(fixedElapsed);// , enemy1, enemy2);
		enemy1.update(fixedElapsed);// , player, enemy2);
		enemy2.update(fixedElapsed);//, player, enemy2);
		
		player.worldCoord(player.world);
		enemy1.worldCoord(enemy1.world);
		enemy2.worldCoord(enemy2.world);
		if (checkSATCollision(player.world, enemy1.world, pen)) {
			//pen.normalize();
			
			player.pos.x += (pen.x / 2) *1.01;
			player.pos.y += (pen.y / 2) *1.01;

			enemy1.pos.x -= (pen.x / 2) *1.01;
			enemy1.pos.y -= (pen.y / 2) *1.01;
			
			player.velx = -player.velx;
			player.vely = -player.vely;

			enemy1.velx = -enemy1.velx;
			enemy1.vely = -enemy1.vely;
		}
		if (checkSATCollision(player.world, enemy2.world, pen)) {
			//pen.normalize();
			
			player.pos.x += (pen.x / 2) *1.01;
			player.pos.y += (pen.y / 2) *1.01;

			enemy2.pos.x -= (pen.x / 2) *1.01;
			enemy2.pos.y -= (pen.y / 2) *1.01;

			player.velx = -player.velx;
			player.vely = -player.vely;

			enemy2.velx = -enemy2.velx;
			enemy2.vely = -enemy2.vely;
		}
		if (checkSATCollision(enemy1.world, enemy2.world, pen)) {
			//pen.normalize();
			
			enemy1.pos.x += (pen.x / 2 ) *1.01;
			enemy1.pos.y -= (pen.y / 2) *1.01;

			enemy2.pos.x -= (pen.x / 2) *1.01;
			enemy2.pos.y += (pen.y / 2) *1.01;

			enemy1.velx = -enemy1.velx;
			enemy1.vely = -enemy1.vely;

			enemy2.velx = -enemy2.velx;
			enemy2.vely = -enemy2.vely;
		}
		
		//player.SATboarder(top, bottom, left, right);
		enemy1.SATboarder(top, bottom, left, right);
		enemy2.SATboarder(top, bottom, left, right);
				
		player.render(&program, playerimg);
		enemy1.render(&program, block2);
		enemy2.render(&program, block);
		/*if (state == GAME_STATE_TITLE || state == GAME_STATE_LOST || state == GAME_STATE_WON) {
			if (keys[SDL_SCANCODE_RETURN]) {
				player.pos.x = 0.0;
				player.pos.y = 0.0;
				state = GAME_STATE_GAME;
			}
		}

		else if (state == GAME_STATE_GAME) {
				if (keys[SDL_SCANCODE_RIGHT]) {					
					if (player.pos.x < 3.25){
							player.pos.x += elapsed * player.velx;
					}
				}
				else if (keys[SDL_SCANCODE_LEFT]) {
					
					if (player.pos.x > -3.25) {
						player.pos.x -= elapsed * player.velx;
					}
				}
				else if (keys[SDL_SCANCODE_UP]) {
					if (player.pos.y < 3.25){
						player.pos.y += elapsed * player.vely;
					}
							
				}

				modelMatrix.identity();
				modelMatrix.Translate(player.pos.x, player.pos.y, 0.0);
				program.setModelMatrix(modelMatrix);
				//player.sprite.Draw(&program);			
		}

		switch (state) {
		case GAME_STATE_TITLE:
			modelMatrix.identity();
			modelMatrix.Translate(-1.7f, 1.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "ASTROIDS", 0.35f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.0f, 0.9f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "USE ARROW KEYS TO MOVE,", 0.20f, 0.0f);
			
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

			modelMatrix.identity();
			modelMatrix.Translate(player.pos.x, player.pos.y, 0.0);
			program.setModelMatrix(modelMatrix);
			//playerObj.sprite.Draw(&program);

			
			break;
		}*/
		
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
