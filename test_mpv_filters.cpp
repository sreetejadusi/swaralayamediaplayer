#include <iostream>
#include <mpv/client.h>
int main() {
    mpv_handle *ctx = mpv_create();
    mpv_initialize(ctx);
    mpv_node node;
    mpv_get_property(ctx, "af", MPV_FORMAT_NODE, &node);
    return 0;
}
