
#include "app.h"


// Main entry point
int main(int argc, char *argv[])
{
    Application::Config cfg; // if required change config defaults here

    Application app( cfg ); // instantiate with config

    app.parse_argv(argc, argv); // anything from run args

    if (!app.Initialise())
    {
        return 1;
    }

    app.Run();

    app.Shutdown();

    return 0;
}