console.log("Hello, World!")
console.log('current application: ', process.argv[0])
console.log("platform: ", process.platform, " -- arch: ", process.arch, "\n")

/*
process.stdout.write('args: ')
process.argv.forEach(function (arg) {
	process.stdout.write(arg + " ")
})
process.stdout.write('\n')

console.log(process.cwd())
// process.chdir('..')
// console.log(process.cwd())

console.log('TEST=', process.getenv('TEST'))
process.setenv('TEST', 'there is no spoon')
console.log('TEST=', process.getenv('TEST'))
process.setenv('TEST', 'oh wait, I found one')
console.log('TEST=', process.getenv('TEST'))

console.log('----------------------------')
*/

// ------------------------------------------------------------

var os = require('os')
console.log("temporary directory: ", os.tmpdir())

console.log('----------------------------')

// ------------------------------------------------------------

var fs = require('fs')

// fs.stat('lol.js', function (err, stats) {
// 	if (err) {
// 		console.log(err)
// 	} else {
// 		console.log(Duktape.enc('jx', stats))
// 	}
// })

// fs.mkdir('didnotexist', function (err) {
// 	if (err) {
// 		console.log(err)
// 	}
// })

// fs.readdir('build', function (err, files) {
// 	if (err) console.log(err)
	
// 	files.forEach(function (file) {
// 		console.log('* ', file)
// 	})
// })

// console.log("current process path: ", fs.realpathSync(process.argv[0]))

fs.appendFile('message.txt', 'data to append', function (err) {
	if (err) throw err;
	console.log('The "data to append" was appended to file!');
});

fs.readFile('main.js', function (err, data) {
	if (err) throw err;
	fs.writeFile('main2.js', data, function (err) {
		if (err) throw err;
		console.log('File successfully copied.');
	});
});