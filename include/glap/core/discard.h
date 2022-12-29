#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#endif

GLAP_EXPORT namespace glap
{
    struct Discard {};
    inline constexpr Discard discard = {};
}