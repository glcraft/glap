#pragma once

#include "../common/expected.h"
#include "../common/error.h"
#include "../common/utils.h"
#include "../common/utf8.h"
#include "utils.h"
#include "biiterator.h"
#include "model.h"
#include <type_traits>
#include <utility>
namespace glap 
{
    template <class P>
    struct ParseParameter {
        constexpr auto operator()(P&, std::optional<std::string_view> value) const -> Expected<void>;
    };
    template <class C>
    static constexpr auto parse_parameter = ParseParameter<C>{};

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

    template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
    class Parser {
        using NameCheck = NameChecker<Commands...>;
        static_assert(!NameCheck::has_duplicate_longname, "commands has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "commands has duplicate short name");
    public:
        static constexpr std::string_view longname = Name;
        static constexpr auto default_command = def_cmd;
        using default_command_type = std::conditional_t<def_cmd==DefaultCommand::None, Discard, std::tuple_element_t<0, std::tuple<Commands...>>>;
        constexpr auto parse(glap::utils::Iterable<std::string_view> auto args) const -> PosExpected<model::Program<Commands...>>;
    private:
         
    };
}

#include "parser.inl"