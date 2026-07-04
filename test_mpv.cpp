#include <iostream>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    char *audio_devices = mpv_get_property_string(ctx, "audio-device-list");
    std::cout << (audio_devices ? audio_devices : "NULL") << std::endl;
    mpv_free(audio_devices);
    mpv_terminate_destroy(ctx);
    return 0;
}
