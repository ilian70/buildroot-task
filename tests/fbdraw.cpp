#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// gcc -o fbdraw fbdraw.cpp $(sdl2-config --cflags --libs)


// Screen dimensions (these will usually be overridden by the framebuffer's actual mode)
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    int success = 1; // Flag to track initialization success

    // 1. **CRITICAL STEP**: Force SDL to use the Framebuffer (FBDev) video driver.
    // Without this, SDL will try X11, Wayland, or other available drivers first.
    // The driver name can sometimes be 'fbcon' or 'directfb' depending on the system/build.
    if (SDL_setenv("SDL_VIDEODRIVER", "fbdev", 1) < 0) {
        fprintf(stderr, "Warning: Could not set SDL_VIDEODRIVER to fbdev. Continuing...\n");
    } else {
        printf("Set SDL_VIDEODRIVER=fbdev successfully.\n");
    }

    // 2. Initialize SDL2 (only need the video subsystem)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = 0;
    } else {
        printf("SDL Initialized successfully.\n");
        
        // 3. Create a Window (This is the framebuffer surface)
        // SDL_WINDOW_FULLSCREEN_DESKTOP is often helpful to ensure it takes over the whole screen.
        window = SDL_CreateWindow(
            "FBDev Test",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP
        );

        if (window == NULL) {
            fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = 0;
        } else {
            // 4. Create a Renderer
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (renderer == NULL) {
                // If accelerated rendering fails, fall back to software rendering (slower, but works)
                fprintf(stderr, "Hardware acceleration failed. Trying software renderer. SDL Error: %s\n", SDL_GetError());
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
                if (renderer == NULL) {
                    fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                    success = 0;
                }
            }
        }
    }

    // 5. Drawing Loop (if initialization was successful)
    if (success && renderer != NULL) {
        printf("Drawing a red rectangle...\n");
        
        // --- Drawing Operations ---
        
        // Clear the screen to a dark gray
        SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
        SDL_RenderClear(renderer);

        // Set color to bright red
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
        
        // Define a rectangle in the middle of the screen
        SDL_Rect rect = {
            WINDOW_WIDTH / 4, 
            WINDOW_HEIGHT / 4, 
            WINDOW_WIDTH / 2, 
            WINDOW_HEIGHT / 2
        };

        // Draw the filled rectangle
        SDL_RenderFillRect(renderer, &rect);

        // Update the screen with the new drawing
        SDL_RenderPresent(renderer);
        
        // Wait a few seconds so you can see the result
        printf("Displaying for 5 seconds. Check your screen...\n");
        SDL_Delay(5000); 
    }

    // 6. Cleanup
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    
    printf("Program finished.\n");
    return success ? 0 : 1;
}