print('hello, world!')

if (!argv) {
	print('argv not found :(')
} else {
	for (var i = 0; i < argv.length; i++) {
		print('arg ',i,': ',argv[i])
	}
}