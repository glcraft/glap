# glap (0.1.0) : gly's command line parser

A UTF-8 command line parser written in C++20 respecting conventionals shell arguments.

*Note : as the library is still in development, the API may change in the future.*

## Notable features

### Up to date library

The library is written in C++20 with language features like concepts, ranges, views... which make the library easier to use and maintain.

### Compile time evaluated model

The model to define the command line parser is defined in templates. All verification and definition of the model is evaluated during the compilation.

### Low heap memory usage

Every string used by the library is a view. Take a look [here](#something-about-lifetime) for more details.

### Error analysis and validators

Several error types is catch and returned for a better analysis or the arguments. Validators over input and argument values can be set up to check parameter during parsing.

Moreover, the library doesn't use exception but expected class in return instead.

### Tested using Google Test

A complete test program is available in "tests" to check the parser. 38 tests are run to checks different features of the parser. As I'm writing this doc, all of the 38 tests passed successfully !

### UTF-8 compliant

If you want to use emoji or non latin alphabet in long or short name, you can !

## Documentation

The whole documentation is available at [docs](docs) folder

## TODO

- [ ] 2.1 - Help generator
- [ ] 2.2 - Executor (handle invokable objects to execute)
- [ ] 2.2 - Subcommands
- [ ] Help: color text
- [ ] Help: locales
- [ ] constexpr hashmap