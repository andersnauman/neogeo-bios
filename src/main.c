#include "bios.h"

__attribute__((used, section(".text._start")))
void _start(void) {
    init();
}