// sudo apt install libsdl2-dev libsdl2-image-dev

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>

#include "logger.h"
extern Logger gLogger; // declare external logger instance


class SDLContext {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int width;
    int height;

public:
    SDLContext(int w = 640, int h = 480) 
        : window(nullptr), renderer(nullptr), texture(nullptr), width(w), height(h) {}
    
    ~SDLContext() {
        Shutdown();
    }

    bool Initialise(std::string title) 
    {
        // Try multiple video drivers in order of preference
        const char* x11 = getenv("DISPLAY");
        const char* wayland = getenv("WAYLAND_DISPLAY");
        Uint32 win_flags = SDL_WINDOW_SHOWN;
        
        // List of video drivers to try in order
        const char* drivers[] = {"kmsdrm", "fbdev", "directfb", "x11", "wayland", "dummy", nullptr};
        bool initialized = false;
        
        // Don't force any driver - let SDL auto-detect first
        if (SDL_Init(SDL_INIT_VIDEO) >= 0) 
        {
            gLogger.log("SDL initialized with auto-detected driver: " + std::string(SDL_GetCurrentVideoDriver()));
            initialized = true;
        } 
        else 
        {
            gLogger.log("SDL auto-detection failed: " + std::string(SDL_GetError()));
            
            // Try each driver explicitly
            for (int i = 0; drivers[i] != nullptr && !initialized; i++) 
            {
                gLogger.log("Trying SDL video driver: " + std::string(drivers[i]));
                
                SDL_setenv("SDL_VIDEODRIVER", drivers[i], 1);
                if (SDL_Init(SDL_INIT_VIDEO) >= 0) {
                    gLogger.log("Success with driver: " + std::string(drivers[i]));
                    initialized = true;
                    break;
                } 
                else {
                    gLogger.log("Driver " + std::string(drivers[i]) + " failed: " + std::string(SDL_GetError()));
                    SDL_Quit();
                }
            }
        }
        
        if (!initialized) {
            gLogger.log("All SDL video drivers failed!");
            return false;
        }
        
        // Set fullscreen for headless systems (no X11/Wayland)
        if (!x11 && !wayland && strcmp(SDL_GetCurrentVideoDriver(), "dummy") != 0) {
            win_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        window = SDL_CreateWindow(title.c_str(),
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  width, height,
                                  win_flags);
        if (window == nullptr) {
            gLogger.log("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            gLogger.log("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
            return false;
        }

        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            gLogger.log("SDL_image could not initialize! IMG_Error: " + std::string(IMG_GetError()));
            return false;
        }

        return true;
    }

    bool DisplayImage(const std::string& image_path) {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        SDL_Surface* loadedSurface = IMG_Load(image_path.c_str());
        if (loadedSurface == nullptr) {
            gLogger.log("Unable to load image " + image_path + "! IMG_Error: " + std::string(IMG_GetError()));
            return false;
        }

        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
        
        if (texture == nullptr) {
            gLogger.log("Unable to create texture from " + image_path + "! SDL_Error: " + std::string(SDL_GetError()));
            return false;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        return true;
    }

    void Shutdown() 
    {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        if (renderer != nullptr) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        IMG_Quit();
        SDL_Quit();
    }
};