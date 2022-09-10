#ifdef __INTELLISENSE__
#include "help.h"
#endif

#include "model.h"
#include "parser.h"
namespace glap::v2 {
    namespace impl {
        template <class ...Others>
        struct FindByName 
        {};
        template <class T1, class T2, class ...Others>
        struct FindByName<T1, T2, Others...>
        {
            using type = std::conditional_t<
                T1::name == T2::name,
                T2,
                typename FindByName<T1, Others...>::type
            >;
        };
        template <class T1, class T2>
        struct FindByName<T1, T2> 
        {
            static_assert(T1::name == T2::name, "Name not found");
            using type = T2;
        };

        
    }

    template<StringLiteral Name, help::model::IsDescription Desc, class ...CommandsDesc>
    class Help<help::model::Program<Name, Desc, CommandsDesc...>>
    {
        using program_t = help::model::Program<Name, Desc, CommandsDesc...>;
        template <class>
        struct GetHelp
        {};
        template<DefaultCommand def_cmd, class... Commands> 
        struct GetHelp<Parser<def_cmd, Commands...>>
        {
            using parser_t = Parser<def_cmd, Commands...>;
            [[nodiscard]] constexpr std::string operator()() const noexcept {
                if constexpr(help::model::IsFullDescription<Desc>)
                    return glap::format("{} - {}\n\n{}", Name, program_t::short_description, program_t::long_description);
                else
                    return glap::format("{} - {}", Name, program_t::short_description);
            }
        };
    public:
        template<class P> 
        static constexpr auto get_help = GetHelp<P>{};
    };
}