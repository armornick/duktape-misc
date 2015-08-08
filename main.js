/*
var module = $loadlib("test")

print("module test:")
print(module)

var test = module()

print("test module return:")
print(Duktape.enc('jx', test))

print(test.hello())
print(test.addTwo(1, 1))

print("loaded c libraries")
print(Duktape.enc('jx', Duktape['$$CLIBS']))
*/

print("io module:")
print(Duktape.enc('jx', io))

io.stdout.puts("hello, world!\n")

var inputs = io.stdin.gets()
// print(inputs.replace("\n", ""))

var f = io.open("test.txt","w")
f.puts(inputs)

var f = null;
print("after setting f to null");