// main entry point for the DropletAnalyzer CLI
#include <iostream>

#include "logging.h"

#ifdef WITH_DCAM_SDK
void droplet_dcam_link_check();
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    logging::setup();
    if(const auto logger = logging::get_logger()) {
        logger->info("DropletAnalyzer CLI starting");
    }

    std::cout << "DropletAnalyzer CLI" << std::endl;

#ifdef WITH_DCAM_SDK
    droplet_dcam_link_check();
#endif

    return 0;
}
