/*
	Laurentiu Nedelcu
*/

#include <iostream>
#include <omp.h>
#include <chrono>
#include <cstring>

#define WIDTH 3840
#define HEIGHT 2160
#define SIZE_CHUNK 5

#define EXPERIMENT_ITERATIONS 1000

typedef unsigned char uchar;

struct _uchar3 {
	uchar x;
	uchar y;
	uchar z;
} __attribute__ ((aligned (4)));

using uchar3 = _uchar3;

struct _uchar4 {
    uchar x;
    uchar y;
    uchar z;
    uchar w;
};

using uchar4 = _uchar4;

bool checkResults(uchar4* rgba, uchar3* grb, int size) {

    bool correct = true;

    for (int i=0; i < size; ++i) {
        correct &= rgba[i].x == grb[i].y;
        correct &= rgba[i].y == grb[i].x;
        correct &= rgba[i].z == grb[i].z;
        correct &= rgba[i].w == 255;
    }

    return correct;
}

void convertGRB2RGBA(uchar3* grb, uchar4* rgba, int width, int height) {
    for (int x=0; x<width; ++x) {
    	for (int y=0; y<height; ++y) {	
            rgba[width * y + x].x = grb[width * y + x].y;
            rgba[width * y + x].y = grb[width * y + x].x;
            rgba[width * y + x].z = grb[width * y + x].z;
            rgba[width * y + x].w = 255;
	}
    }
}

void convertGRB2RGBA_2(uchar3* grb, uchar4* rgba, int width, int height) {
	int y, x;
	for (y=0; y<height; ++y) {
    	for (x=0; x<width; ++x) {	
            rgba[width * y + x].x = grb[width * y + x].y;
            rgba[width * y + x].y = grb[width * y + x].x;
            rgba[width * y + x].z = grb[width * y + x].z;
            rgba[width * y + x].w = 255;
	    }
    }
}

void convertGRB2RGBA_2_optional(uchar3* grb, uchar4* rgba, int width, int height) {
	int pos;
	int total = width*height;
	for (pos=0; pos < total; ++pos) {
		rgba[pos].x = grb[pos].y;
		rgba[pos].y = grb[pos].x;
		rgba[pos].z = grb[pos].z;
		rgba[pos].w = 255;
	}
	
}

void convertGRB2RGBA_3(uchar3* grb, uchar4* rgba, int width, int height) {
	int x, y;
	#pragma omp parallel for \
	schedule(auto) \
	private(x, y) firstprivate(grb, width, height, rgba)
	for (y=0; y<height; ++y) {
    	for (x=0; x<width; ++x) {	
            rgba[width * y + x].x = grb[width * y + x].y;
            rgba[width * y + x].y = grb[width * y + x].x;
            rgba[width * y + x].z = grb[width * y + x].z;
            rgba[width * y + x].w = 255;
	    }
    }
}

int main() {

    uchar3 *h_grb;
    uchar4 *h_rgba;

    int bar_widht = HEIGHT/3;

    // Alloc and generate BRG bars.
    h_grb = (uchar3*)malloc(sizeof(uchar3)*WIDTH*HEIGHT);
    for (int i=0; i < WIDTH * HEIGHT; ++i) {
        if (i < bar_widht) { h_grb[i] = { 255, 0, 0 }; }
        else if (i < bar_widht*2) { h_grb[i] = { 0, 255, 0 }; }
        else { h_grb[i] = { 0, 0, 255 }; }
    }

    // Alloc RGBA pointers
    h_rgba = (uchar4*)malloc(sizeof(uchar4)*WIDTH*HEIGHT);

	// Time point representing the current time
    auto t1 = std::chrono::high_resolution_clock::now();
    #pragma omp parallel
    #pragma omp critical
    {
        std::cout << "I am the thread number " << omp_get_thread_num() << std::endl;
    }
    #pragma omp for
    for (int i=0; i<EXPERIMENT_ITERATIONS; ++i) {    
		convertGRB2RGBA_2(h_grb, h_rgba, WIDTH, HEIGHT);
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    
    std::cout << "convertGRB2RGBA time for " << EXPERIMENT_ITERATIONS \
    << " iterations = "<< duration << "us" << std::endl;
    
    bool ok = checkResults(h_rgba, h_grb, WIDTH*HEIGHT);

    if (ok) {
        std::cout << "Executed!! Results OK." << std::endl;
    } else {
        std::cout << "Executed!! Results NOT OK." << std::endl;
    }

    return 0;

}
