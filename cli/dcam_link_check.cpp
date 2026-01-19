#ifdef WITH_DCAM_SDK

#include <dcamapi4.h>

void droplet_dcam_link_check() {
    volatile auto init_fn = &dcamapi_init;
    (void)init_fn;
}

#endif
