#include "window.h"
#include "spdlog/spdlog.h"
#include <spdlog/common.h>

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);

    if (argc != 2) {
        spdlog::error("pass a single media file as argument\n");
        return 1;
    }

    window::play(argv[1]);

    return 0;
}
