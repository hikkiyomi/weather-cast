#include <iostream>

#include "include/api_handler.h"

int main(int, char**) {
    Handler handler("config.omfl");
    handler.Request();
}
