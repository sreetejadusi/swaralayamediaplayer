#include <iostream>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    mpv_set_property_string(ctx, "af", "rubberband=pitch-scale=1.5");
    char *err = mpv_get_property_string(ctx, "af");
    std::cout << "AF: " << (err ? err : "NULL") << std::endl;
    return 0;
}
