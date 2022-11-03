#pragma once

#include "core/expected.h"
#include "core/error.h"
#include "core/utils.h"
#include "core/utf8.h"
#include "core/utils.h"
#include "core/biiterator.h"
#include "model.h"
#include <type_traits>
#include <utility>
namespace glap 
{
    template <class>
    class Parser
    {};
    template <class T>
    static constexpr auto parser = Parser<T>{};
}

#include "impl/parser2.inl"