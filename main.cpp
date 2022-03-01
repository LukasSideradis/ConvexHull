#include "SDL.h"
#include "SDL_image.h"
#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include <time.h>
#include <iostream>
#include <algorithm>

std::vector<SDL_Event>&GetFrameEvents() {
	static std::vector<SDL_Event> frame_events;
	return frame_events;
}

void Swap(std::vector<SDL_Point>* arr, int left, int right) {
	SDL_Point tmp = arr->at(right);
	arr->at(right) = arr->at(left);
	arr->at(left) = tmp;
}

void QuicksortX(std::vector<SDL_Point>* arr, int left, int right) {
	if (left < right) {
		int boundary = left;
		for (int i = left + 1; i < right; i++) {
			if (arr->at(i).x < arr->at(left).x) {
				Swap(arr, i, ++boundary);
			}
		}
		Swap(arr, left, boundary);
		QuicksortX(arr, left, boundary);
		QuicksortX(arr, boundary + 1, right);
	}
}

void QuicksortY(std::vector<SDL_Point>* arr, int left, int right) {
	if (left < right) {
		int boundary = left;
		for (int i = left + 1; i < right; i++) {
			if ((arr->at(i).x == arr->at(left).x) && (arr->at(i).y < arr->at(left).y)) {
				Swap(arr, i, ++boundary);
			}
		}
		Swap(arr, left, boundary);
		QuicksortY(arr, left, boundary);
		QuicksortY(arr, boundary + 1, right);
	}
}

std::vector<SDL_Point> ConvexHull(std::vector<SDL_Point> points) {
	// Sort the points so they're processed in the correct order
	QuicksortX(&points, 0, points.size());
	QuicksortY(&points, 0, points.size());

	std::vector<SDL_Point> upper;
	std::vector<SDL_Point> lower;
	// Compute the upper hull
	upper.emplace_back(points.at(0));
	upper.emplace_back(points.at(1));
	for (int i = 2; i < points.size(); i++) {
		upper.emplace_back(points.at(i));
		while (upper.size() > 2) {
			// if the determinant of | x3 - x1 | is negative, then the turn is left  oriented
			//						 | x2 - x1 | is positive, then the turn is right oriented
			//									 is zero    , then the 3 points are collinear and the redundant point is removed
			if ( (upper.at(upper.size() - 1).x - upper.at(upper.size() - 3).x) * (upper.at(upper.size() - 2).y - upper.at(upper.size() - 3).y)
			   - (upper.at(upper.size() - 1).y - upper.at(upper.size() - 3).y) * (upper.at(upper.size() - 2).x - upper.at(upper.size() - 3).x) <= 0) {
				upper.erase(upper.end() - 2);
			}
			else {
				break;
			}
		}	
	}
	// Compute the lower hull
	lower.emplace_back(points.at(points.size() - 1));
	lower.emplace_back(points.at(points.size() - 2));
	for (int i = points.size() - 3; i > -1; i--) {
		lower.emplace_back(points.at(i));
		while (lower.size() > 2) {
			// this method works the same for both (left -> right) and (right -> left)
			if ((lower.at(lower.size() - 1).x - lower.at(lower.size() - 3).x) * (lower.at(lower.size() - 2).y - lower.at(lower.size() - 3).y)
				- (lower.at(lower.size() - 1).y - lower.at(lower.size() - 3).y) * (lower.at(lower.size() - 2).x - lower.at(lower.size() - 3).x) <= 0) {
				lower.erase(lower.end() - 2);
			}
			else {
				break;
			}
		}
	}
	lower.pop_back();
	lower.erase(lower.begin());

	// Joins the two sub-hulls together
	std::vector<SDL_Point> convexHull = upper;
	convexHull.insert(convexHull.end(), lower.begin(), lower.end());
	return convexHull;
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);
	SDL_Window* mainWindow = SDL_CreateWindow("Convex Hull", 40, 40, 800, 800, SDL_WINDOW_SHOWN);
	SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Test data
	srand(time(NULL));
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<int> distr(50, 750);
	int random_number_1 = 0;
	int random_number_2 = 0;

	std::vector<SDL_Point> points;
	for (int i = 0; i < 1000; i++) {
		random_number_1 = distr(generator);
		random_number_2 = distr(generator);
		points.emplace_back(SDL_Point{ random_number_1, random_number_2 });
	}
	//

	std::vector<SDL_Point> convexHull = ConvexHull(points);

	bool quit = false;

	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			GetFrameEvents().emplace_back(e);
		}

		for (auto& ev : GetFrameEvents()) {
			if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_q) {
				quit = true;
			}
		}

		SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
		SDL_RenderClear(mainRenderer);

		// Draw the hull
		SDL_SetRenderDrawColor(mainRenderer, 0, 0, 255, 255);
		for (int i = 0; i < convexHull.size(); i++) {
			if (i == convexHull.size() - 1) {
				SDL_RenderDrawLine(mainRenderer, convexHull.at(i).x, convexHull.at(i).y, convexHull.at(0).x, convexHull.at(0).y);
			}
			else {
				SDL_RenderDrawLine(mainRenderer, convexHull.at(i).x, convexHull.at(i).y, convexHull.at(i + 1).x, convexHull.at(i + 1).y);
			}
		}

		// Draw the points
		SDL_SetRenderDrawColor(mainRenderer, 0, 255, 0, 255);
		for (auto& p : points) {
			SDL_RenderDrawPoint(mainRenderer, p.x, p.y);
		}

		SDL_RenderPresent(mainRenderer);

		SDL_Delay(1000.0 / 60.0);
	}

	return 0;
}