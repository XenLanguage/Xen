# Changelog

## v0.2.0 (January 2, 2026)

This is the update that brings the bulk of Xen's features to life. Support has been added for:

- Control flow (`if`/`else`, `for`/`while`) blocks
- `and` and `or` keywords
- Namespaces and object properties
- Package imports (`include` keyword)
- Recursion
- Compound operators (`+=`, `/=`, `%=`, etc.)
- Iteration via C-style or range-based loops (`for(def; cond; iter)` or `for(var in min..max)`)
- Pre- and post-fix operators for incrementing and decrementing values (`++x`, `x++`, `--x`, `x--`)

A lot of work has also been spent developing Xen's standard library, which now boasts four modules: `io`, `math`, `string`, and `datetime`. Documentation for these will be coming in the near future. These can be
imported and used in Xen scripts like so:

```
include io;

io.println("Hello, Xen!");
```
