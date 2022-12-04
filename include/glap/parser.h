#pragma once

#ifndef GLAP_MODULE
#include "core/base.h"
#include "core/expected.h"
#include "core/error.h"
#include "core/utils.h"
#include "core/utf8.h"
#include "core/utils.h"
#include "core/biiterator.h"
#include "model.h"
#include <type_traits>
#include <utility>
#endif

GLAP_EXPORT namespace glap
{
    template <class>
    class Parser
    {};
    template <class T>
    inline constexpr auto parser = Parser<T>{};
}

#ifndef GLAP_MODULE
#include "impl/parser2.inl"
#endif