// sudo apt install libsdl2-dev libsdl2-image-dev

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

#include "logger.h"
extern Logger gLogger; // declare external logger instance

#include "sdl_ctx.h"


void SDLContext::startAutoInitialise()   
{
    if (autoInitThread.joinable()) 
    {
        stopAutoInitialise();
    }
    
    stopAutoInit = false;
    autoInitThread = std::thread([this]() 
    {
        const int MAX_TRIES = 10; // Set maximum number of attempts
        int attempts = 0;
        
        while (!stopAutoInit && attempts < MAX_TRIES) 
        {
            if (!initialized) 
            {
                gLogger.log("Auto-initialize: Attempting to initialize SDL... (Attempt " + 
                           std::to_string(attempts + 1) + "/" + std::to_string(MAX_TRIES) + ")");
                if (tryInitialise()) {
                    gLogger.log("Auto-initialize: SDL initialized successfully");
                    break;
                }
                attempts++;
            }
            else {
                break;
            }
            
            // Wait 5 seconds before next attempt
            for (int i = 0; i < 50 && !stopAutoInit; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (attempts >= MAX_TRIES && !initialized) {
            gLogger.log("Auto-initialize: Failed after " + std::to_string(MAX_TRIES) + " attempts. Giving up.");
        }
    });
}

void SDLContext::stopAutoInitialise() 
{
    stopAutoInit = true;
    if (autoInitThread.joinable()) {
        autoInitThread.join();
    }
}


