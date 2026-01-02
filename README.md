# Xen

Xen is a loosely-typed, imperative scripting language written in [ANSI C](https://en.wikipedia.org/wiki/C11_(C_standard_revision)). Its syntax closesly resembles C-like languages and the bytecode compiler and VM is a mere 70Kb in total. Its primary purpose is to serve as a learning project for myself and a fun side project to work on in my free time. **It is not designed with the intent of being a serious, production-ready language.**

If you want to try the current version of Xen for yourself, you can download [precompiled binaries](https://github.com/jakerieger/Xen/releases/latest) or download and compile the source code yourself. You can also checkout [Xen's "Get Started" guide](https://jakerieger.github.io/Xen/get-started.html) for a quick guide on getting up and running with programming in Xen.

## Examples

Example code can be found in the [examples](examples) directory.


### Strings and IO
```js
include io;
include string;

var greet = "Hello";
var name = "John";
var msg = string.join(greet, ", ", name);

io.println(msg);

io.print("Enter a number: ");
var num = number(io.readline());

for (var i in 0..num) {
    io.print(i, " ");
}
io.println("");
```

### Math

```js
include io;
include math;

fn rad_2_deg(rad) {
    return rad * 180 / math.PI;
}

fn deg_2_rad(deg) {
    return 180 / math.PI;
}

io.println("Radians to Degrees: ", rad_2_deg(math.PI / 4));
io.println("Degrees to Radians: ", deg_2_rad(90));
```

### Arrays and Iterators

```js
include io;
include array;

var items = [2, 4, 6, 8, 10];
for (var i in items) {
    items[i]++;
}
io.println("Items: ", items);

while (array.len(items) > 0) {
    var i = array.pop(items);
    io.println("Popped: ", i);
}
```

## License

**Xen** is licensed under the [ISC license](LICENSE).
