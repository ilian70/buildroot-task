
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

LogLevel log_level = LogLevel::Info; // Default log level

int parse_argv(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--loglevel" || arg == "-l")
        {
            if (i + 1 < argc)
            {
                log_level = static_cast<LogLevel>(std::stoi(argv[++i]));
            }
        }
    }
    return 0;
}

// App config
class Config
{
public:
    std::string KEY = "current_image"; // redis key to monitor
    int REFRESH_TIME_SECONDS = 2;
    std::string IMAGE_FOLDER = "./images/";
    std::string IMAGE_EXTENSION = ".png";

    Config() = default;
};

//-------------------------------------------------------------------
//* APP

int main(int argc, char *argv[])
{
    parse_argv(argc, argv);
    println("Log level set to: ", (int)log_level);

    // 1. SDL
    SDLContext sdl(800, 600);

    if (!sdl.Init("Redis Image Viewer"))
    {
        println("Failed to initialize SDL!");
        return 1;
    }
    println("Successfully initialized SDL.");

    // Configuration
    constexpr std::string_view REDIS_HOST = "127.0.0.1";
    const int REDIS_PORT = 6379;

    RedisConnect redis(REDIS_HOST, REDIS_PORT);
    if (!redis.connect())
    {
        println("Failed to connect to Redis server!");
        return 1;
    }
    println("Connected to Redis server at ", REDIS_HOST, ":", REDIS_PORT);

    // 2. Main Loop
    SDL_Event e;
    bool quit = false;

    Config config; 
    std::string current_image_name = "";

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

        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_check).count() >= config.REFRESH_TIME_SECONDS)
        {
            std::string imageName = redis.getValueOfKey(std::string(config.KEY));

            if(log_level > LogLevel::Info) println("Redis key '", config.KEY, "' value: ", imageName);
            if (!imageName.empty() && imageName != current_image_name)
            {
                if (log_level > LogLevel::Info) println("New image requested: ", imageName);

                std::string full_path = config.IMAGE_FOLDER + imageName + config.IMAGE_EXTENSION;

                if (sdl.displayImage(full_path))
                {
                    current_image_name = imageName;
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