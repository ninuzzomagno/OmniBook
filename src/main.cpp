#include "BasicIncludes.h"

int _newlib_heap_size_user = 192 * 1024 * 1024;

int main(int argc, char* argv[]) {
    
    OmniBook::init();
    OmniBook::mainloop();
    OmniBook::cleanup();

    return 0;
}