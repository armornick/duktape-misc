var fs = require('fs')

var LINE_MAX = 1024

function stripFile (filename, modname) {
	var contents = fs.readFileSync(filename).toString()
	// console.log(contents)
	// console.log('---------------------------------------------------')

	// contents = contents.replace(/\/\*.+?\*\//g, "").replace(/\/\/[^\n]*/g, "")
	contents = contents.replace(/\/\/[^\n]*/g, "")
		.replace(/[\t\r]+/g, "").replace(/[ \n]+/g, " ")
		.replace(/\\/g, "\\\\").replace(/\"/g, "\\\"")

	// console.log(contents)

	var output = "const char "+ modname +"[] = "
	// console.log(output)


	var start = 0
	while (start <= contents.length-1) {
		var n = contents.length - start
		if (n > LINE_MAX) { n = LINE_MAX }
		var finish = start + n

		while (contents[finish] == '\\') { finish-- }

		output += "\t\"" + contents.slice(start, finish) + "\"\n"

		start = finish++;
		// console.log(start)
	}

	output += ";"

	console.log(output)

}


stripFile('vendor/path.js', 'path')
// stripFile('calc.js', 'parser')