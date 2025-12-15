// sudo apt install libsdl2-dev libsdl2-image-dev

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

#include "logger.h"
extern Logger gLogger; // declare external logger instance

#include "sdl_ctx.h"

// Construct
SDLContext::SDLContext(int w, int h) 
    : window(nullptr), renderer(nullptr), texture(nullptr), width(w), height(h) {}
    
SDLContext::~SDLContext() {
    Shutdown();
}

//---------------------------------------------------
//* INIT
bool SDLContext::Initialise(std::string title,int drawMode, int rgbOrder, int autoInit) 
{
    this->title = title; // settings
    this->drawMode = drawMode;
    this->rgbOrder = rgbOrder;
    this->autoInit = autoInit;

    gLogger.log("SDLContext::Init: title: ", title, 
                 ", drawMode: ", drawMode, 
                 ", rgbOrder: ", rgbOrder,
                 ", autoInit: ", autoInit);


    bool ok = tryInitialise();

    if( ! ok && autoInit == 1 )
        startAutoInitialise();
    else 
        stopAutoInitialise();

    return ok;
}

bool SDLContext::tryInitialise() 
{
    if(driverFound) 
    {
        Shutdown();
    }

    // IMG Loaders
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        gLogger.log("SDL_image could not initialize! IMG_Error: " + std::string(IMG_GetError()));
        return false;
    }


    // Try multiple video drivers in order of preference
    const char* x11 = getenv("DISPLAY");
    const char* wayland = getenv("WAYLAND_DISPLAY");
    Uint32 win_flags = SDL_WINDOW_SHOWN;
    
    // List of video drivers to try in order
    const char* drivers[] = {"fbcon","kmsdrm", "fbdev", "directfb", "x11", "wayland", "dummy", nullptr};
    
    // Don't force any driver - let SDL auto-detect first
    if (SDL_Init(SDL_INIT_VIDEO) >= 0) 
    {
        gLogger.log("SDL Init with auto-detected driver: " + std::string(SDL_GetCurrentVideoDriver()));
        driverFound = true;
    } 
    else 
    {
        gLogger.log("SDL auto-detection failed: " + std::string(SDL_GetError()));
        
        // Try each driver explicitly
        for (int i = 0; drivers[i] != nullptr && !driverFound; i++) 
        {
            gLogger.log("Trying SDL video driver: " + std::string(drivers[i]));
            
            SDL_setenv("SDL_VIDEODRIVER", drivers[i], 1);
            if (SDL_Init(SDL_INIT_VIDEO) >= 0) {
                gLogger.log("Success with driver: " + std::string(drivers[i]));
                driverFound = true;
                break;
            } 
            else {
                gLogger.log("Driver " + std::string(drivers[i]) + " failed: " + std::string(SDL_GetError()));
                SDL_Quit();
            }
        }
    }

    // Set fullscreen for headless systems (no X11/Wayland)
    if (!x11 && !wayland ) //&& strcmp(SDL_GetCurrentVideoDriver(), "dummy") != 0) 
    {
        win_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    window = SDL_CreateWindow(title.c_str(),
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                width, height,
                                win_flags);
    if (window == nullptr) {
        gLogger.log("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        gLogger.log("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }

    // int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    // if (!(IMG_Init(imgFlags) & imgFlags)) {
    //     gLogger.log("SDL_image could not initialize! IMG_Error: " + std::string(IMG_GetError()));
    //     return false;
    // }

    if (!driverFound) 
    {
        gLogger.log("All SDL video drivers failed!");
        return false;
    }

    return true;
}


void SDLContext::Shutdown() 
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

    driverFound = false;
}
