var module = $loadlib("test")
var jstest = $loadlib("jstest")()
var test = module()
var name = jstest.name

print("test module:" + !!(test))
// print(Duktape.enc('jx', test))

print("jstest module:" + !!(jstest))
// print(Duktape.enc('jx', jstest))

print("io module:" + !!(io))
// print(Duktape.enc('jx', io))

print("THE NAME IS "+name)

print('-------------------------------------------')

/*
// --------------------------------------------------------

var OUT_FILE = "test.txt"

print('writing '+OUT_FILE)
var outputf = io.open(OUT_FILE, "w")

for (var i = 0; i < 24; i++) {
	for (var j = 0; j < i; j++) {
		outputf.puts('\t')
	}
	outputf.puts(test.hello())
	outputf.puts('\n')
}

outputf = null

// --------------------------------------------------------

var byteNumber = 100
var IN_FILE = "test.txt"

print('reading '+ byteNumber + ' bytes from ' + IN_FILE)
var inputf = io.open(IN_FILE, 'r')

print(inputf.gets())

inputf.rewind()

var bytes = inputf.read(byteNumber)

print("buffer: ", toString.call(bytes))
print("'" + bytes.toString() + "'")

print("next character: " + inputf.getc())
print('-------------------------------------------')

// --------------------------------------------------------

var IN_FILE = "test.txt"

var inputf = io.open(IN_FILE, 'r')

print("file size (in bytes): " + inputf.size())

var contents = inputf.readAll()
print("file length: " + contents.toString().length)

*/

// --------------------------------------------------------

/*
var FILENAME = "test.txt"

if (io.exists(FILENAME)) {
	print("file already exists: not creating")
} else {
	io.writeFile(FILENAME, "there is no spoon\nbiatches!")	
}

var contents = io.readFile(FILENAME)
print(FILENAME + ': ' + contents.toString().length + " bytes")
print(contents.toString())
*/

// --------------------------------------------------------

/*
if (empty) {
	var test_obj = { msg: "there is no spoon!", friday: true, jiggles: null }
	print("test_obj before calling empty: "+Duktape.enc('jx', test_obj))
	empty(test_obj)
	print("test_obj after calling empty: "+Duktape.enc('jx', test_obj))
}
print('-------------------------------------------')
*/

// --------------------------------------------------------

var _INTERNAL = {
	inter: 'exports.value = "Hello, World!"'
}

Duktape.modSearch = function (id, require, exports, module) {
	
	if (_INTERNAL[id]) return _INTERNAL[id]

	var filename = id+'.js'
	if (io.exists(filename)) {
		var contents = io.readFile(filename).toString()
		return contents
	}

	throw new Error('cannot find module: ' + id);
}


print('modLoaded = ' + Duktape.enc('jx',Duktape.modLoaded))

var mod = require('lol')
var inter = require('inter')

print('modLoaded = ' + Duktape.enc('jx',Duktape.modLoaded))

if (!mod) { print('could not load lol :(') }
else { 
	print('lol = ' + Duktape.enc('jx',mod) + '\n')
	mod.hello('Bob') 
}