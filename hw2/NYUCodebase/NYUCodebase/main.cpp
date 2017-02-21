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
	//glBindTexture(GL_TEXTURE_2D, fontTexture);

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
	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER "fragment.glsl");

	GLuint left = LoadTexture(RESOURCE_FOLDER"left.png");
	GLuint right = LoadTexture(RESOURCE_FOLDER"right.png");
	GLuint white = LoadTexture(RESOURCE_FOLDER"white.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
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
	srand(NULL);
	bool done = false;
	//GLuint font = LoadTexture(RESOURCE_FOLDER"font1.png");
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

		//left paddle
		glBindTexture(GL_TEXTURE_2D, white);
		modelMatrix.identity();
		modelMatrix.Translate(0.0, left_y_pos, 0.0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		//glBindTexture(GL_TEXTURE_2D, white);

		float vertices[] = { -3.6f, -0.8f, -3.6f, 0.0f, -3.3f, 0.0f, -3.6f, -0.8f, -3.3f, 0.0f, -3.3f, -0.8f };

		float t2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t2);
		glEnableVertexAttribArray(program.texCoordAttribute);
		if (keys[SDL_SCANCODE_W] && left_y_pos < 2.0) {
			left_y_pos += elapsed * 2;
		}
		else if (keys[SDL_SCANCODE_S] && left_y_pos > -1.25) {
			left_y_pos -= elapsed * 2;
		}

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		//right paddle
		glBindTexture(GL_TEXTURE_2D, white);
		modelMatrix.identity();
		modelMatrix.Translate(0.0, right_y_pos, 0.0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		

		float vertices1[] = { 3.6f, -0.8f, 3.6f, 0.0f, 3.3f, 0.0f, 3.6f, -0.8f, 3.3f, 0.0f, 3.3f, -0.8f };
		float t3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t3);
		glEnableVertexAttribArray(program.texCoordAttribute);
		if (keys[SDL_SCANCODE_UP] && right_y_pos < 2.0) {
			right_y_pos += elapsed * 2;
		}
		else if (keys[SDL_SCANCODE_DOWN] && right_y_pos > -1.25) {
			right_y_pos -= elapsed * 2;
		}

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		//ball
		glBindTexture(GL_TEXTURE_2D, white);
		modelMatrix.identity();
		modelMatrix.Translate(ball_pos_x, ball_pos_y, 0.0);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		/*if (ball_pos_x < -3.32) { // left paddle lost
		//ball_pos_x += 2.2 *elapsed;
		//ball_pos_y += 0;
		modelMatrix.identity();
		//modelMatrix.Translate(-2.5, -1.0, 0.0);
		program.setModelMatrix(modelMatrix);
		GLuint font = LoadTexture(RESOURCE_FOLDER"font1.png");
		//DrawText(&program, font, "right paddle won! press Space to end game", 0.25 , 0.0);
		//modelMatrix.identity();
		modelMatrix.identity();
		//modelMatrix.Translate(-2.5, -1.0, 0.0);
		program.setModelMatrix(modelMatrix);

		if (keys[SDL_SCANCODE_SPACE]) {
		done = true;
		}
		//done == true;

		}*/


		/*if (ball_pos_x > 3.2) { // right paddle lost
		done = true;
		}*/

		if (b == 0) {
			ball_pos_x += 1.25 * elapsed;
			ball_pos_y -= 1.25 * elapsed;
			r = 0;
		}

		if (ball_pos_y > 2.0 || b == 1) { // off top wall
			b = 1;
			if (r == 0) {
				ball_pos_x -= 1.35 * elapsed;
				ball_pos_y -= 1.25 * elapsed;
			}
			if (r == 1) {
				ball_pos_x += 1.3 * elapsed;
				ball_pos_y -= 1.25 * elapsed;
			}
			if (r == 2) {
				ball_pos_x += 1.24 * elapsed;
				ball_pos_y -= 1.25 * elapsed;
			}
			if (r == 3) {
				ball_pos_x -= 1.27 * elapsed;
				ball_pos_y -= 1.25 * elapsed;
			}
			r1 = rand() % 4;
			r2 = rand() % 4;
			r3 = rand() % 4;
		}
		if (ball_pos_y < -2.0 || b == 2) { // off bottom wall
			b = 2;
			if (r1 == 0) {
				ball_pos_x += 1.24 * elapsed;
				ball_pos_y += 1.4 * elapsed;
			}
			if (r1 == 1) {
				ball_pos_x -= 1.27 * elapsed;
				ball_pos_y += 1.25*elapsed;
			}
			if (r1 == 2) {
				ball_pos_x -= 1.3 * elapsed;
				ball_pos_y += 1.25*elapsed;
			}
			if (r1 == 3) {
				ball_pos_x -= 1.35 * elapsed;
				ball_pos_y += 1.25*elapsed;
			}
			r2 = rand() % 4;
			r3 = rand() % 4;
		}
		if ((ball_pos_x < -3.31 && ball_pos_x > -3.33 && ball_pos_y < left_y_pos + 0.4 && ball_pos_y > left_y_pos - 0.7) || b == 3) {
			b = 3;
			if (r2 == 0) {
				ball_pos_x += 1.6 * elapsed;
				ball_pos_y -= 1.3 * elapsed;
			}
			if (r2 == 1) {
				ball_pos_x += 1.3 * elapsed;
				ball_pos_y += 1.4 * elapsed;
			}
			if (r2 == 2) {
				ball_pos_x += 1.7 * elapsed;
				ball_pos_y -= 1.5 * elapsed;
			}
			if (r2 == 3) {
				ball_pos_x += 1.2 * elapsed;
				ball_pos_y += 1.7 *sin(3.1415926 / 180)* elapsed;
			}
			r = rand() % 4;
			r1 = rand() % 4;
			r3 = rand() % 4;
		}
		if ((ball_pos_x > 3.15 && ball_pos_x < 3.17 && ball_pos_y < right_y_pos + 0.4 && ball_pos_y > right_y_pos - 0.7) || b == 4) {
			b = 4;
			if (r3 == 0) {
				ball_pos_x -= 1.8 * elapsed;
				ball_pos_y += 1.7 * elapsed;
			}
			if (r3 == 1) {
				ball_pos_x -= 1.6 * elapsed;
				ball_pos_y -= 1.3 * elapsed;
			}
			if (r3 == 2) {
				ball_pos_x -= 1.2 * elapsed;
				ball_pos_y += 1.5 * elapsed;
			}
			if (r3 == 3) {
				ball_pos_x -= 1.6 * elapsed;
				ball_pos_y -= 1.6 * elapsed;
			}
			r = rand() % 4;
			r1 = rand() % 4;
			r2 = rand() % 4;
		}

		float vertices2[] = { 0.15f, -0.15f, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.15f, 0.15f, -0.15f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);

		float t4[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t4);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		if (ball_pos_x < -3.32) { // left paddle lost
			
			glBindTexture(GL_TEXTURE_2D, right);

			float v9[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, v9);
			glEnableVertexAttribArray(program.positionAttribute);

			float t9[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t9);
			glEnableVertexAttribArray(program.texCoordAttribute);

			modelMatrix.identity();

			modelMatrix.Translate(-2.5, 1.0, 0);
			angle += 30.0f*elapsed;
			modelMatrix.Rotate(-1 * 2.0f*angle*(3.1415926 / 180));

			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
		}
		if (ball_pos_x > 3.2) { // right paddle lost
			OutputDebugString("saf");
			modelMatrix.identity();

			modelMatrix.Translate(-2.5, 1.0, 0);
			angle += 30.0f*elapsed;
			modelMatrix.Rotate(-1 * 2.0f*angle*(3.1415926 / 180));

			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			glBindTexture(GL_TEXTURE_2D, left);

			float v10[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, v10);
			glEnableVertexAttribArray(program.positionAttribute);


			float t10[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, t10);
			glEnableVertexAttribArray(program.texCoordAttribute);
			
			glDisableVertexAttribArray(program.positionAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}
