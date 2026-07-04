#include <iostream>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    mpv_set_option_string(ctx, "msg-level", "all=v");
    mpv_command_string(ctx, "loadfile D:/swaralayaplayer/build/test.mp3");
    mpv_set_property_string(ctx, "af", "rubberband=pitch-scale=1.5");
    char *err = mpv_get_property_string(ctx, "af");
    std::cout << "AF: " << (err ? err : "NULL") << std::endl;
    return 0;
}
