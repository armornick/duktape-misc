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