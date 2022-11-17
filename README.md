# glap (2.0) : gly's command line parser

A UTF-8 command line parser written in C++20 respecting conventionals shell arguments.

*Note : as the library is still in development, the API may change in the future.*

## Notable features

### Compile time evaluated model

The model to define the command line parser is defined in templates. All definition and verification of the model is evaluated during the compilation.

### Up to date library

The library is written in C++20 with its new language features like concepts, ranges, views... which make the library easier to use and maintain.

### Flexible

The model is easy to update or to personnalize by the user himself.

### Error analysis and validators

Several error types is catch and returned for a better analysis or the arguments. Validators over input and argument values can be set up to check parameter during parsing.

Moreover, the library doesn't use exception but *expected* class in return instead.

### Tested using Google Test

A complete test program is available in "tests" to check the parser. As I'm writing this doc, all of the 50 tests passed successfully !

### UTF-8 text

The library works with UTF-8 text. So if you want to use emoji or non latin alphabet in long or short name, you can !

## Documentation

The whole documentation is available at [docs](docs) folder, let's check it out !

## TODO

- [ ] Help generator
  - [ ] Base
  - [ ] Colored text
  - [ ] Locales
- [ ] Executor (handle invokable objects to execute)
- [ ] Subcommands
- [ ] constexpr hashmap for  long names