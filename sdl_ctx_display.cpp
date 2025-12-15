#include "sdl_ctx.h"

#include "logger.h"
extern Logger gLogger; // declare external logger instance


bool SDLContext::DisplayImage(const std::string& image_path) 
{
    if (texture != nullptr) 
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    
    SDL_Surface* loadedSurface = IMG_Load(image_path.c_str());
    
    if (loadedSurface == nullptr) 
    {
        gLogger.log("Unable to load image " + image_path + "! IMG_Error: " + std::string(IMG_GetError()));
        return false;
    }

    if( driverFound && drawMode == 0 )
    {
        // kmsdrm 
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        
        if (texture == nullptr) {
            gLogger.log("Unable to create texture from " + image_path + "! SDL_Error: " + std::string(SDL_GetError()));
            return false;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    else if( drawMode == 1)
    {
        // Direct framebuffer access (bypass SDL rendering)
        SDL_Rect dstrect = {0, 0, loadedSurface->w, loadedSurface->h};

        SDL_Surface* screenSurface = SDL_GetWindowSurface(window);
        // Option A: Use SDL_BlitSurface
        SDL_BlitSurface(loadedSurface, NULL, screenSurface, NULL);        
        SDL_UpdateWindowSurface(window);
    }
    else if( drawMode == 2)
    {
        // Direct framebuffer write
        DirectFramebufferWrite(loadedSurface, rgbOrder);
    }


    SDL_FreeSurface(loadedSurface);


    return true;
}
