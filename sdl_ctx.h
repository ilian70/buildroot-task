// sudo apt install libsdl2-dev libsdl2-image-dev

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <atomic>
#include <string>
#include <thread>

class SDLContext {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int width;
    int height;
    std::string title{"SDL Window"};
    std::atomic<bool> driverFound{false};
    std::thread autoInitThread;
    std::atomic<bool> stopAutoInit{false};
    int drawMode{0}; // 0 = DRM, 1 = SDL Blit, 2 = direct framebuffer
    int rgbOrder{0}; // 0 = RGB, 1 = BGR
    int autoInit{0}; // 0 = off, 1 = on

    bool tryInitialise();

    void startAutoInitialise();
    void stopAutoInitialise();
public:
    SDLContext(int w = 640, int h = 480);
    ~SDLContext();

    bool Initialise(std::string title, int drawMode, int rgbOrder, int autoInit);
    bool DisplayImage(const std::string& image_path);
    void Shutdown();
    bool isInitialized() const { return driverFound; }
};

extern bool DirectFramebufferWrite(SDL_Surface *loadedSurface, int rgbOrder); //= 0 RGB, 1 BGR