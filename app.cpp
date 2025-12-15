#include <chrono>
#include <thread>

#include "logger.h"
#include "print.h"

#include "app.h"

Logger gLogger; // global logger instance

// The Application
Application::Application( Config cfg )
    : config(cfg),
        sdl(config.screen_width, config.screen_height),
        redis(config.RedisHostIP, config.RedisPort)
{
}

//static 
int Application::parse_argv(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if ((arg == "--loglevel" || arg == "-l") && i + 1 < argc)
        {
            logLevel = static_cast<LogLevel>(std::stoi(argv[++i]));
        }
        else if ((arg == "--config" || arg == "-c") && i + 1 < argc)
        {
            CfgFile = argv[++i];
        }
    }
    return 0;
}



bool Application::Initialise( bool continueOnFail )
{
    bool loggerOpened = gLogger.Open( config.LogFile );
    if (!loggerOpened) {
        println("Failed to open log file: ", config.LogFile);
    }
    gLogger.log("Log level set to: ", (int)logLevel);

    //1 SDL
    if (!sdl.Initialise(config.WindowTitle, config.DrawMode, config.RGBOrder, config.SDLAutoInit))
    {
        gLogger.log("Failed to initialize SDL!");

        if (!continueOnFail) {
            return false;
        }
    }
    gLogger.log("Initialized SDL ", sdl.isInitialized() ? "OK" : "ERROR");

    //2 DB / NET
    bool redisConn = redis.Connect();
    if ( ! redisConn )
    {
        gLogger.log("Failed to connect to Redis server!");
    }
    else {
        gLogger.log("Connected to Redis server OK");
    }

    gLogger.log("Application initialization DONE.");

    return true; // NOTE: continue even if SDL or Redis failed
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
    static auto last_heartbeat = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    // Send heartbeat every 5 seconds
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count() >= 5)
    {
        sendHeartbeat();
        last_heartbeat = now;
    }

    // Check for remote commands
    handleRemoteCommands();

    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_check).count() >= config.RefreshTimeGET_sec)
    {
        auto id = redis.GetString(std::string(config.KEY));

        if (!id.empty() && id != crntImgName)
        {
            bool ok = sdl.DisplayImage(formImagePath(id));
            if (ok)
            {
                crntImgName = id;
            }

            println(ok? "OK":"ERR", " (display) Polled image: img",  id, ".png");
        }
        last_check = now;
    }
}

void Application::sendHeartbeat()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
    
    redis.SetString("App:Heartbeat", std::string(timestamp));
    
    // Also set configuration values for remote console
    redis.SetString("Config:RedisHost", config.RedisHostIP);
    redis.SetString("Config:RedisPort", std::to_string(config.RedisPort));
    redis.SetString("Config:ImageFolder", config.ImageFolder);
    redis.SetString("Config:RefreshInterval", std::to_string(config.RefreshTimeGET_sec));
    redis.SetString("Config:ScreenWidth", std::to_string(config.screen_width));
    redis.SetString("Config:ScreenHeight", std::to_string(config.screen_height));
}

void Application::handleRemoteCommands()
{
    auto command = redis.GetString("App:Command");
    
    if (!command.empty())
    {
        println("Received remote command: ", command);
        
        if (command == "refresh") // Force refresh of current image
        {
            auto id = redis.GetString(std::string(config.KEY));
            if (!id.empty())
            {
                bool ok = sdl.DisplayImage(formImagePath(id));
                if ( ok )
                {
                    crntImgName = id;
                }

                println(ok? "OK" : "ERR", " (display) Refreshed image: img",  id, ".png");
            }
        }
        else if (command == "status") // Send status response
        {
            std::string status = "Running - Current Image: " + crntImgName;
            redis.SetString("App:Response", status);
        }
        
        // Clear the command after processing
        redis.Delete("App:Command");
    }
}