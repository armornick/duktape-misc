print("io module:" + !!(io))
// print(Duktape.enc('jx', io))

print("os module:" + !!(os))
print(Duktape.enc('jx', os))

print('--------------------------------------')

// --------------------------------------------------------

/*
package.preload = {
	inter: 'exports.value = "Hello, World!"'
}


print('modLoaded = ' + Duktape.enc('jx',Duktape.modLoaded))

var mod = require('lol')
var inter = require('inter')

print('modLoaded = ' + Duktape.enc('jx',Duktape.modLoaded))

if (!mod) { print('could not load lol :(') }
else { 
	print('lol = ' + Duktape.enc('jx',mod) + '\n')
	print(inter.value)
	mod.hello('Bob') 
}
*/

// --------------------------------------------------------

print("TEMP: ", os.getenv('TEMP'))
print("TMP: ", os.getenv('TMP'))
print("tmpname: ", os.tmpname())
print("shell? ", os.execute())
print('where notepad?')
print("result: ", os.execute('where notepad'))
print("USERPROFILE: ", os.getenv('USERPROFILE'))