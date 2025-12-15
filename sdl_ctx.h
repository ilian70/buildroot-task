// sudo apt install libsdl2-dev libsdl2-image-dev

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>

class SDLContext {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int width;
    int height;
    bool initialized = false;

public:
    SDLContext(int w = 640, int h = 480);
    ~SDLContext();

    bool Initialise(std::string title);
    bool DisplayImage(const std::string& image_path);
    void Shutdown();
    bool isInitialized() const { return initialized; }
};

