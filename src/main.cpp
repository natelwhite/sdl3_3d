#include "Renderer.hpp"
#include "Materials.hpp"
#include <math.h>
#include <iostream>

Context* Context::self = 0;

int main() {
	// calculate tangent points
	const int rad { 1 };
	const int num_tangent_points { 128 };
	SDL_FPoint tangent_points[num_tangent_points];
	SDL_FColor tangent_colors[num_tangent_points];
	for (double i = 0; i < num_tangent_points; i++) {
		SDL_FPoint result;
		auto calcTangent = [](const double &degrees, const double &radius) -> SDL_FPoint {
			const double PI { 3.14159265 };
			return SDL_FPoint {
				static_cast<float>(cos(degrees * PI / 180.0) * radius),
				static_cast<float>(sin(degrees * PI / 180.0) * radius)
			};
		};
		auto calcColor = [](const float &deg) -> SDL_FColor {
			return SDL_FColor {deg * 255, deg * 255, deg * 255, 255};
		};

		double deg;
		if (i == 0) {
			deg = 0;
		} else {
			deg = (360.0 / num_tangent_points) * i;
		}
		tangent_points[static_cast<int>(i)] = calcTangent(deg, rad);
		tangent_colors[static_cast<int>(i)] = calcColor(deg / 360);
	}

	Renderer renderer {640, 480};
	const size_t VERTEX_COUNT { num_tangent_points * 3 };
	const char *vert_shader { "PositionColor.vert" }, *frag_shader { "SolidColor.frag" };
	VertexBufferMat mat {vert_shader, frag_shader, VERTEX_COUNT};

	// push vertices
	SDL_FPoint origin {0, 0};
	PositionColorVertex *buffer { mat.open() };
	int buffer_index {};
	PositionColorVertex vert;
	for (int i = 0; i < num_tangent_points; i++) {
		vert = {
			tangent_points[i].x,
			tangent_points[i].y,
			0,
			static_cast<Uint8>(tangent_colors[i].r),
			static_cast<Uint8>(tangent_colors[i].g),
			static_cast<Uint8>(tangent_colors[i].b),
			static_cast<Uint8>(tangent_colors[i].a),
		};
		buffer[buffer_index++] = vert;

		vert = { origin.x, origin.y, 0, 0, 0, 0, 255 };
		buffer[buffer_index++] = vert;

		if (i + 1 < num_tangent_points) {
			vert = {
				tangent_points[i + 1].x,
				tangent_points[i + 1].y,
				0,
				static_cast<Uint8>(tangent_colors[i + 1].r),
				static_cast<Uint8>(tangent_colors[i + 1].g),
				static_cast<Uint8>(tangent_colors[i + 1].b),
				static_cast<Uint8>(tangent_colors[i + 1].a),
			};
		} else {
			vert = {
				tangent_points[0].x,
				tangent_points[0].y,
				0,
				255,
				255,
				255,
				255,
			};
		}
		buffer[buffer_index++] = vert;
	}
	mat.upload();
	buffer = nullptr;

	// main loop
	bool quit = false;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_EVENT_QUIT:
				quit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				switch(e.key.key) {
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_R:
					mat.refresh();
					break;
				}
			}
		}
		mat.draw();
	}
	return 0;
}
