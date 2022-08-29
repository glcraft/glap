#pragma once

#include "../common/expected.h"
#include "../common/error.h"
#include "../common/utils.h"
#include "../common/utf8.h"
#include "utils.h"
#include "biiterator.h"
#include "model.h"
namespace glap::v2 
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
        constexpr auto operator()(C&, utils::BiIterator<Iter> args) const -> PosExpected<C>;
    };
    template <class C>
    static constexpr auto parse_command = ParseCommand<C>{};

    template<class... Commands>
    class Parser {
        using NameCheck = NameChecker<Commands...>;
        static_assert(!NameCheck::has_duplicate_longname, "commands has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "commands has duplicate short name");
    public:
        constexpr auto parse(glap::utils::Iterable<std::string_view> auto args) const -> PosExpected<model::Program<Commands...>>;
    private:
         
    };
}

#include "parser.inl"