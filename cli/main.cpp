// main entry point for the DropletAnalyzer CLI
#include <iostream>

#ifdef WITH_DCAM_SDK
void droplet_dcam_link_check();
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::cout << "DropletAnalyzer CLI" << std::endl;

#ifdef WITH_DCAM_SDK
    droplet_dcam_link_check();
#endif

    return 0;
}
