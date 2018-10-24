#ifndef PAINTER_HEADER
#define PAINTER_HEADER

#include <inttypes.h>
#include <stdio.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include "dmath.h"

class Painter
{
	private:
	Vec ref;
	real scale;
	real invScale;

	public:
	Painter(uintptr_t w, uintptr_t h)
	{
		ref.zero();
		scale = 100;
		invScale = real(1.0) / scale;

		SDL_Init(SDL_INIT_VIDEO);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_SetVideoMode(w, h, 0, SDL_OPENGL | SDL_HWSURFACE);

		glShadeModel(GL_SMOOTH);
		glClearColor(0, 0, 0, 0);
		glEnable(GL_DEPTH_TEST);
		
		//glEnable(GL_POINT_SMOOTH);
		//glEnable(GL_LINE_SMOOTH);
		glPointSize(1);

		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(40, w/(real)h, 1, 100);//near/far plane

		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		gluLookAt(3, 3, -5, 0, 0, 0, 0, 1, 0);
	}

	void setScale(const Vec &min, const Vec &max)
	{
		ref = min;

		Vec size = max - min;
		scale = size.x > size.y ? size.x : size.y;
		scale = scale > size.z ? scale : size.z;

		invScale = real(1.0) / scale;
	}

	void paint(Boid *boids, uintptr_t nBoids)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(1,0,0);
		glBegin(GL_POINTS);

		for(uintptr_t i = 0; i < nBoids; ++i) {
			Vec pos = boids[i].pos - ref;
			real x = pos.x * invScale;
			real y = pos.y * invScale;
			real z = pos.z * invScale;

			glVertex3f(x,y,z);
		}

		glEnd();

		SDL_GL_SwapBuffers();
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					exit(0);
					break;
			}
		}
	}

	~Painter()
	{
		SDL_Quit();
	}
};

#endif /* PAINTER_HEADER */
