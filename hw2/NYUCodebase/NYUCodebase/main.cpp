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
#include "time.h"
//#include "assert.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;
SDL_Window* displayWindow;
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
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
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;
		//float texture_x = (float)((int)text[i] % 16) / 16.0f;
		//float texture_y = (float)((int)text[i] / 16) / 16.0f;

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
	glBindTexture(GL_TEXTURE_2D, fontTexture);

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

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	SDL_Event event;
	glViewport(0, 0, 640, 360);
	//ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint left = LoadTexture(RESOURCE_FOLDER"left.png");
	GLuint right = LoadTexture(RESOURCE_FOLDER"right.png");
	GLuint yellow = LoadTexture(RESOURCE_FOLDER"yellow.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	//projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	projectionMatrix.setOrthoProjection(0.0, 7.0, 0.0, 4.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;
	float angle = 0.0f;
	float left_y_pos = 0.0;
	float right_y_pos = 0.0;
	float ball_pos_x = 0.0;
	float ball_pos_y = 0.0;
	int b = 0;
	int r = 0;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	srand((int)time(NULL));
	bool done = false;
	float prev_ball_x = 0.0;
	float prev_ball_y = 0.0;
	GLuint font = LoadTexture(RESOURCE_FOLDER"font2.png");
	bool game = true;


	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					angle = 0.0;
				}
			}
		}
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		float startX, startY, endX, endY;
		float speed = 100;

		glClear(GL_COLOR_BUFFER_BIT);
		modelMatrix.identity();
		modelMatrix.Translate(1.0, 2.0, 0);
		program.setModelMatrix(modelMatrix);

		DrawText(&program, font, "use S and W to move left paddle", 0.15, 0.0);
		modelMatrix.identity();
		modelMatrix.Translate(0.4, 1.5, 0);
		program.setModelMatrix(modelMatrix); 
			glClear(GL_COLOR_BUFFER_BIT);
			modelMatrix.identity();
			modelMatrix.Translate(1.0, 2.0, 0);
			program.setModelMatrix(modelMatrix);

			DrawText(&program, font, "use S and W to move left paddle", 0.15, 0.0);
			modelMatrix.identity();
			modelMatrix.Translate(0.4, 1.5, 0);
			program.setModelMatrix(modelMatrix);

			DrawText(&program, font, "use up and down arrow to move right paddle", 0.15, 0.0);

			//left paddle
			glBindTexture(GL_TEXTURE_2D, yellow);
			modelMatrix.identity();
			modelMatrix.Translate(0.0, left_y_pos, 0.0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			float vertices[] = { 0.0f, 0.0f, 0.0f, 0.8, 0.2f, 0.8f, 0.0f, 0.0, 0.2f, 0.8f , 0.2f, 0.0f };

			float t2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t2);
			glEnableVertexAttribArray(program.texCoordAttribute);
			if (keys[SDL_SCANCODE_W] && left_y_pos < 3.2) {
				left_y_pos += elapsed * 2;
			}
			else if (keys[SDL_SCANCODE_S] && left_y_pos > 0.0) {
				left_y_pos -= elapsed * 2;
			}

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);

			//right paddle
			glBindTexture(GL_TEXTURE_2D, yellow);
			modelMatrix.identity();
			modelMatrix.Translate(0.0, right_y_pos, 0.0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			float vertices1[] = { 6.8f, 0.0f, 6.8f,0.8f, 7.0f, 0.8f, 6.8f, 0.0f, 7.0f, 0.8f, 7.0f, 0.f };
			float t3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t3);
			glEnableVertexAttribArray(program.texCoordAttribute);
			if (keys[SDL_SCANCODE_UP] && right_y_pos < 3.2) {
				right_y_pos += elapsed * 2;
			}
			else if (keys[SDL_SCANCODE_DOWN] && right_y_pos > 0.0) {
				right_y_pos -= elapsed * 2;
			}

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);

			//ball
			glBindTexture(GL_TEXTURE_2D, yellow);
			modelMatrix.identity();
			modelMatrix.Translate(ball_pos_x, ball_pos_y, 0.0);

			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			if (b == 0) {
				prev_ball_x = ball_pos_x;
				prev_ball_y = ball_pos_y;
				ball_pos_x -= 1.3 * elapsed;
				ball_pos_y -= 1.3 * elapsed;
				r = 0;
			}
			if (ball_pos_y >= 0.6 || b == 1) { // off top wall
				OutputDebugString("top");
				b = 1;
				if (ball_pos_x <= prev_ball_x) {//coming from right
					OutputDebugString("r");
					if (r == 0) {
						OutputDebugString("0");
						ball_pos_x -= 1.75 * elapsed;
						ball_pos_y -= 1.85 * elapsed;
					}
					if (r == 1) {
						OutputDebugString("1");
						ball_pos_x -= 1.7 * elapsed;
						ball_pos_y -= 1.75 * elapsed;
					}
				}
				else if (ball_pos_x >= prev_ball_x) { // coming from left
					OutputDebugString("l");
					if (r == 0) {
						OutputDebugString("0");
						ball_pos_x += 1.85 * elapsed;
						ball_pos_y -= 1.75 * elapsed;
					}
					if (r == 1) {
						OutputDebugString("1");
						ball_pos_x += 1.84 * elapsed;
						ball_pos_y -= 2.2 * elapsed;
					}
				}

				r1 = (int)(rand() * 10) % 2;
				r2 = (int)(rand() * 10) % 2;
				r3 = (int)(rand() * 10) % 2;
			}
			if (ball_pos_y < -3.4 || b == 2) { // off bottom wall
				b = 2;
				if (ball_pos_x < prev_ball_x) {//comes from right
					OutputDebugString("right");
					prev_ball_x = ball_pos_x;
					prev_ball_y = ball_pos_y;
					if (r1 == 0) {
						OutputDebugString("ro");
						ball_pos_x -= 2.17 * elapsed;
						ball_pos_y += 2.05*elapsed;
					}
					else {//if (r1  == 1) {
						OutputDebugString("r1");
						ball_pos_x -= 1.75 * elapsed;
						ball_pos_y += 1.95*elapsed;
					}
				}
				if (ball_pos_x >= prev_ball_x) {
					OutputDebugString("left");
					prev_ball_x = ball_pos_x;
					prev_ball_y = ball_pos_y;
					if (r1 == 0) {
						OutputDebugString("l0");
						prev_ball_x = ball_pos_x;
						prev_ball_y = ball_pos_y;
						ball_pos_x += 2.24 * elapsed;
						ball_pos_y += 2.2 * elapsed;
					}

					if (r1 == 1) {
						OutputDebugString("l1");
						ball_pos_x += 2.03 * elapsed;
						ball_pos_y += 2.25*elapsed;
					}
				}
				r2 = (int)(rand() * 10) % 2;
				r3 = (int)(rand() * 10) % 2;
			}

			if ((ball_pos_x < -3.2 && ball_pos_x > -3.5  && ball_pos_y < left_y_pos - 2.7 && ball_pos_y > left_y_pos - 3.5) || b == 3) { //left paddle
				b = 3;

				if (ball_pos_y < prev_ball_y) { //coming from top
					OutputDebugString("leftpaddletop");
					prev_ball_x = ball_pos_x;
					prev_ball_y = ball_pos_y;
					if (r2 == 0) {
						ball_pos_x += 1.8 * elapsed;
						ball_pos_y -= 2.3 * elapsed;
					}
					if (r2 == 1) {
						ball_pos_x += 1.7 * elapsed;
						ball_pos_y -= 1.8 * elapsed;
					}
				}
				if (ball_pos_y >= prev_ball_y) { //coming form bottom
					OutputDebugString("leftpaddlebottom");
					prev_ball_x = ball_pos_x;
					prev_ball_y = ball_pos_y;
					if (r2 == 0) {
						ball_pos_x += 2.3 * elapsed;
						ball_pos_y += 2.04 * elapsed;
					}

					if (r2 == 1) {
						ball_pos_x += 2.2 * elapsed;
						ball_pos_y += 1.79 * elapsed;
					}
				}
				r = rand() % 2;
				r1 = rand() % 2;
				r3 = rand() % 2;
			}
			if ((ball_pos_x > 3.35 && ball_pos_x < 3.45 && ball_pos_y < right_y_pos - 2.6 && ball_pos_y > right_y_pos - 3.5) || b == 4) {
				if (ball_pos_y <= prev_ball_y) { // comes from top
					prev_ball_x = ball_pos_x;
					prev_ball_y = ball_pos_y;
					OutputDebugString("41");
					if (r3 == 0) {
						OutputDebugString("41a");
						ball_pos_x -= 1.86 * elapsed;
						ball_pos_y -= 2.13 * elapsed;
					}
					//if (r3 == 1) {
					else {
						OutputDebugString("41b");

						ball_pos_x -= 1.96 * elapsed;
						ball_pos_y -= 1.9 * elapsed;
					}
				}

				else if (ball_pos_y >= prev_ball_y) { // comes from bottom
					prev_ball_x = ball_pos_x;
					prev_ball_y = ball_pos_y;
					OutputDebugString("42");
					if (r3 == 0) {
						OutputDebugString("42a");

						ball_pos_x -= 1.8 * elapsed;
						ball_pos_y += 1.94 * elapsed;
					}

					if (r3 == 1) {
						OutputDebugString("42b");

						ball_pos_x -= 1.8 * elapsed;
						ball_pos_y += 1.9 * elapsed;
					}
				}

				r = rand() % 2;
				r1 = rand() % 2;
				r2 = rand() % 2;
			}

			float vertices2[] = { 3.4f, 3.4f, 3.4f, 3.6f, 3.6f, 3.6f, 3.4f, 3.4f, 3.6f, 3.6f, 3.6f, 3.4f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
			glEnableVertexAttribArray(program.positionAttribute);

			float t4[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t4);
			glEnableVertexAttribArray(program.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);

			if (ball_pos_x < -3.32) { // left paddle lost

				modelMatrix.identity();
				modelMatrix.Translate(1.6, 3.0, 0);
				program.setModelMatrix(modelMatrix);

				DrawText(&program, font, "right paddle won!", 0.25, 0.0);

				modelMatrix.identity();
				modelMatrix.Translate(0.9, 1.0, 0);
				program.setModelMatrix(modelMatrix);

				DrawText(&program, font, "press space to end game!", 0.25, 0.0);
				if (keys[SDL_SCANCODE_SPACE]) {
					done = true;
				}

			}
			if (ball_pos_x > 3.6) { // right paddle lost

				modelMatrix.identity();
				modelMatrix.Translate(1.6, 3.0, 0);
				program.setModelMatrix(modelMatrix);


				DrawText(&program, font, "left paddle won!", 0.25, 0.0);

				modelMatrix.identity();
				modelMatrix.Translate(0.9, 1.0, 0);
				program.setModelMatrix(modelMatrix);

				DrawText(&program, font, "press space to end game!", 0.25, 0.0);
				if (keys[SDL_SCANCODE_SPACE]) {
					done = true;
				}
			}
		
		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}
