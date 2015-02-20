var cats = require('cat-ascii-faces')

var cat;
while ((cat = cats()) && new Buffer(cat).length > 18) {
	continue;
}

var spawnSync = require('child_process').spawnSync;

var ret = spawnSync('make', ['MANUFACTURER_DATA="' + cat + '"'], {
	cwd: __dirname + '/../',
	stdio: 'inherit',
});

if (ret.status) {
	process.exit(ret.status);
}

var ret = spawnSync('make', ['flash'], {
	cwd: __dirname + '/../',
	stdio: 'inherit',
});

if (ret.status) {
	process.exit(ret.status);
}

var noble = require('noble');

console.log('Start.');

noble.startScanning(); // any service UUID, no duplicates

noble.on('discover', function (item) {
	console.log('Found', item.advertisement);
	if (item.advertisement.localName == 'Reach') {
		var manufacturer = item.advertisement.manufacturerData.slice(0, new Buffer(cat, 'utf8').length).toString('utf8');
		console.log('\n\n');
		console.log(manufacturer, '=?', cat);
		if (manufacturer == cat) {
			console.log('SUCCESS: Cat found!')
			process.exit(0);
		} else {
			console.log('FAILURE: Wrong cat found!')
			process.exit(1);
		}
	}
});

setTimeout(function () {
	console.log('FAILURE: Timeout.');
	process.exit(1);
}, 10*1000)
