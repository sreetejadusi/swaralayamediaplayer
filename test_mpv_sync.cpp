#include <iostream>
#include <windows.h>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    mpv_set_option_string(ctx, "audio-pitch-correction", "no");
    mpv_set_option_string(ctx, "speed", "1.5");
    mpv_set_option_string(ctx, "af", "scaletempo=scale=0.66666");
    mpv_command_string(ctx, "loadfile \"D:/swaralayaplayer/build/test.mp4\"");
    Sleep(5000);
    return 0;
}
