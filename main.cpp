
#include <thread>
#include <chrono>

#include "redis_conn.h"
#include "sdl_ctx.h"
#include "print.h"

// Parse command line arguments
enum class LogLevel {
    Info = 1,
    Warn = 2,
    Debug = 3
};

LogLevel logLevel = LogLevel::Info; // Default log level

int parse_argv(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--loglevel" || arg == "-l")
        {
            if (i + 1 < argc)
            {
                logLevel = static_cast<LogLevel>(std::stoi(argv[++i]));
            }
        }
    }
    return 0;
}

// App config
class Config
{
public:
    std::string_view RedisHostIP = "127.0.0.1";
    int RedisPort = 6379;

    std::string KEY = "current_image"; // redis key to monitor
    int RefreshTimeGET_sec = 2;
    std::string ImageFolder = "./images/";
    std::string ImageExtension = ".png";

    int scree_width = 800;
    int screen_height = 600;
    std::string WindowTitle = "Redis Image Viewer";

    Config() = default;
};

//-------------------------------------------------------------------
//* APP

int main(int argc, char *argv[])
{
    parse_argv(argc, argv);
    println("Log level set to: ", (int)logLevel);

    Config config; // may be extended to parse from file/args

    // SDL
    SDLContext sdl(config.scree_width, config.screen_height);

    if (!sdl.Init(config.WindowTitle))
    {
        println("Failed to initialize SDL!");
        return 1;
    }
    println("Successfully initialized SDL.");

    // Database
    RedisConnect redis(config.RedisHostIP, config.RedisPort);
    if (!redis.connect())
    {
        println("Failed to connect to Redis server!");
        return 1;
    }
    println("Connected to Redis server at ", config.RedisHostIP, ":", config.RedisPort);

    // Main Loop
    SDL_Event e;
    bool quit = false;
    std::string crntImgName = "";

    while (!quit)
    {
        // Handle (input) events
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            {
                quit = true;
            }
        }

        // 3. Redis Query
        static auto last_check = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_check).count() >= config.RefreshTimeGET_sec)
        {
            auto imgName = redis.getValueOfKey(std::string(config.KEY));

            if(logLevel > LogLevel::Info) println("Redis key '", config.KEY, "' value: ", imgName);
            
            if (!imgName.empty() && imgName != crntImgName)
            {
                if (logLevel > LogLevel::Info) println("New image requested: ", imgName);

                auto fullPath = config.ImageFolder + imgName + config.ImageExtension;

                if (sdl.displayImage(fullPath))
                {
                    crntImgName = imgName;
                }
            }
            last_check = now;
        }

        // Optional: brief delay to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 4. Cleanup
    redis.disconnect();
    sdl.close();

    return 0;
}