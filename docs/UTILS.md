# Utils

## Names

### Definition

```cpp
/// In namespace glap
template <StringLiteral LongName, auto ShortName = discard>
struct Names {
    static constexpr std::string_view longname = LongName;
    static constexpr std::optional<char32_t> shortname = impl::optional_value<char32_t, ShortName>;
};
```

### Description

This class is used to define argument long name and short name in the model.

Long name is required, whereas short name is optional.

Long name is a utf8 string, write the string character in the template.
Short name on the other hand is a unicode code point (or [`discard`](#discard)). This means it expects a number, not a 
string. Use `''` annotation to define the character easily. 

You can define one and only unicode code point in a short name. Composed code points like multiple accented character 
(for example [*glitched*](https://lingojam.com/GlitchTextGenerator) strings) or color specified emojis 
(for example 👩🏿🧓🏻) won't work.

## Handle errors

## Discard

### Definition

```cpp
/// In namespace glap
struct Discard {};
static constexpr Discard discard = {};
```

### Description

The type `Discard` and value `discard` is used to apply the default behaviour or to discard something in templates.
For example, Resolvers and Validators may be discarded so no resolution nor validations is processed.

## Value

### Definition

```cpp
template <auto Resolver = discard, auto Validator = discard>
struct Value {
    using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
    static constexpr auto resolver = Resolver;
    static constexpr auto validator = Validator;
    std::optional<value_type> value;
};
```

### Description

This is the base model to capture the value of [Input](#single-parameter-argument) and 
[Parameter](#single-parameter-argument).

`Resolver` is either a function or [*discard*](#discard). Here is The list of signature expected :
* `(std::string_view) -> T` where T is whatever type.
* `(std::string_view) -> tl::expected<T, void>` where T is whatever type. If an error is raised, an error is raised 
by the parser

The type of `value` is based on the output of the `Resolver`.

`Validator` is either a function or [*discard*](#discard). The signature expected is `(std::string_view) -> bool`. When
`Validator` returns false, an error is raised by the parser.

By default, there is no value. It accepts only one value. It means if a value is set whereas a value is already setted, 
an error is raised by the parser.

## Container
### Definition
### Description