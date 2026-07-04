#include <iostream>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    mpv_set_option_string(ctx, "msg-level", "all=v");
    mpv_set_option_string(ctx, "audio-pitch-correction", "no");
    mpv_set_option_string(ctx, "speed", "1.5");
    mpv_set_option_string(ctx, "af", "scaletempo=scale=0.66666");
    char *err = mpv_get_property_string(ctx, "af");
    std::cout << "AF: " << (err ? err : "NULL") << std::endl;
    return 0;
}
