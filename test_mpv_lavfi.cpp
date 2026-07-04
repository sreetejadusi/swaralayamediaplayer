#include <iostream>
#include <windows.h>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    mpv_set_option_string(ctx, "msg-level", "all=v");
    mpv_set_option_string(ctx, "af", "lavfi=[asetrate=48000*1.5,aresample=48000,atempo=1.0/1.5]");
    char *err = mpv_get_property_string(ctx, "af");
    std::cout << "AF: " << (err ? err : "NULL") << std::endl;
    return 0;
}
