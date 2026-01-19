// main entry point for the DropletAnalyzer CLI
#include <iostream>

#include "logging.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    logging::setup();
    if(const auto logger = logging::get_logger()) {
        logger->info("DropletAnalyzer CLI starting");
    }

    std::cout << "DropletAnalyzer CLI" << std::endl;
    return 0;
}
