
#include "app.h"


// Main entry point
int main(int argc, char *argv[])
{
    Application::parse_argv(argc, argv); // anything from run args

    Application::Config cfg{Application::CfgFile}; // overrides from file

    Application app( cfg ); // instantiate with config

    

    if (!app.Initialise())
    {
        return 1;
    }

    app.Run(); // main loop (and waiting for events)

    app.Shutdown();

    return 0;
}