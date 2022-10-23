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
    template <class P>
    struct ParseArgument {
        constexpr auto operator()(P&, std::optional<std::string_view> value) const -> Expected<void>;
    };
    template <class C>
    static constexpr auto parse_argument = ParseArgument<C>{};

    template <class C>
    struct ParseCommand {
        template <class Iter>
        constexpr auto operator()(C&, utils::BiIterator<Iter> args) const -> PosExpected<void>;
    };
    template <class C>
    static constexpr auto parse_command = ParseCommand<C>{};

    enum class DefaultCommand {
        FirstDefined,
        None
    };

    template <class>
    class Parser
    {};
}

#include "impl/parser2.inl"