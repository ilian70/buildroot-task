#include <chrono>
#include <thread>

#include "app.h"


Application::Application( Config cfg)
    : config(cfg),
        sdl(config.scree_width, config.screen_height),
        redis(config.RedisHostIP, config.RedisPort)
{
}

int Application::parse_argv(int argc, char *argv[])
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

bool Application::Initialise()
{
    println("Log level set to: ", (int)logLevel);

    if (!sdl.Initialise(config.WindowTitle))
    {
        println("Failed to initialize SDL!");
        return false;
    }
    println("Successfully initialized SDL.");

    if (!redis.Connect())
    {
        println("Failed to connect to Redis server!");
        return false;
    }
    println("Connected to Redis server at ", config.RedisHostIP, ":", config.RedisPort);

    return true;
}

void Application::Run()
{
    SDL_Event e;

    while (!quit)
    {
        handleEvents(e);
        updateFromRedis();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Application::Shutdown()
{
    redis.Disconnect();
    sdl.Shutdown();
}

void Application::handleEvents(SDL_Event& e)
{
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
}

std::string Application::formImagePath( std::string id )
{
    return config.ImageFolder + config.ImagePrefix + id + config.ImageExtension;
}

void Application::updateFromRedis()
{
    static auto last_check = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_check).count() >= config.RefreshTimeGET_sec)
    {
        auto id = redis.GetString(std::string(config.KEY));

        if (!id.empty() && id != crntImgName)
        {
            if (sdl.DisplayImage(formImagePath(id)))
            {
                crntImgName = id;
            }
        }
        last_check = now;
    }
}