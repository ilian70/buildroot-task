
#include "redis_conn.h"
#include "sdl_ctx.h"


//-------------------------------------------------------------------
//* APP
class Application
{
public:
    // CFG
    struct Config
    {
        std::string_view RedisHostIP = "127.0.0.1";
        int RedisPort = 6379;

        std::string KEY = "current_image"; // redis key to monitor
        int RefreshTimeGET_sec = 2;
        std::string ImageFolder = "./images/";
        std::string ImageExtension = ".png";
        std::string ImagePrefix = "img";

        int scree_width = 800;
        int screen_height = 600;
        std::string WindowTitle = "Redis Image Viewer";
    };
    enum class LogLevel { Info,  Warn ,  Debug};

    // API
    Application( Config cfg);
    int parse_argv(int argc, char *argv[]);
    bool Initialise();
    void Run();
    void Shutdown();

private:
    void handleEvents(SDL_Event& e);
    void updateFromRedis();
    std::string formImagePath(std::string id);
private:
    Config config;
    SDLContext sdl;
    RedisConnect redis;
    bool quit = false;
    std::string crntImgName = "";

    LogLevel logLevel = LogLevel::Info; // Default log level
};
