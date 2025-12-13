
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
        std::string RedisHostIP = "192.168.0.100"; //"127.0.0.1";
        int RedisPort = 6379;

        std::string KEY = "ImageId"; // redis key to monitor
        int RefreshTimeGET_sec = 2;
        std::string ImageFolder = "./images/";
        std::string ImageExtension = ".png";
        std::string ImagePrefix = "img";

        int screen_width = 800;
        int screen_height = 600;
        std::string WindowTitle = "Redis Image Viewer";

        Config( std::string file );
        
        bool loadFromFile(const std::string &filename);
    };
    enum class LogLevel { Info,  Warn ,  Debug};

    // API
    Application( Config cfg);    
    bool Initialise();
    void Run();
    void Shutdown();

    static int parse_argv(int argc, char *argv[]);

private:
    void handleEvents(SDL_Event& e);
    void updateFromRedis();
    void sendHeartbeat();
    void handleRemoteCommands();
    std::string formImagePath(std::string id);
private:
    Config config;
    SDLContext sdl;
    RedisConnect redis;
    bool quit = false;
    std::string crntImgName = "";
public:
    static inline LogLevel logLevel = LogLevel::Info; // Default log level
    // Default to system-installed config; can be overridden via --config
    static inline std::string CfgFile = "/etc/redis-image-viewer/config.json";
};
