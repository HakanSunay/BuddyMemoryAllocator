//
// Created by Hakan Halil on 9.08.20.
//

#include "Exception.h"

Exception::Exception(const char *what) : runtime_error(what) {}
