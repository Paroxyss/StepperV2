import http from 'http';

function getLocalIp() {
	const { networkInterfaces } = require('os');

	const nets = networkInterfaces();
	const results: any = {}; // Or just '{}', an empty object

	for (const name of Object.keys(nets)) {
		for (const net of nets[name]) {
			// Skip over non-IPv4 and internal (i.e. 127.0.0.1) addresses
			// 'IPv4' is in Node <= 17, from 18 it's a number 4 or 6
			const familyV4Value = typeof net.family === 'string' ? 'IPv4' : 4
			if (net.family === familyV4Value && !net.internal) {
				if (!results[name]) {
					results[name] = [];
				}
				results[name].push(net.address);
			}
		}
	}
	return results.en0[0] as string;
}

async function checkIp(ip: string) {
	// first, test if port 81 return error 400

	const options = {
		hostname: ip,
		port: 81,
		path: '/',
		method: 'GET',
		timeout: 1000
	};
	const firstTest = await new Promise((resolve, reject) => {
		const req = http.request(options, res => {
			if (res.statusCode == 400) {
				resolve(true);
			} else {
				resolve(false);
			}
		});
		req.on('error', error => {
			resolve(false);
		});
		req.on("timeout", () => {
			resolve(false);
		})
		req.end();
	})
	if (!firstTest) return false;
	// second, test if port 80 return connection refused
	const options2 = {
		...options,
		port: 80
	}
	const secondTest = await new Promise((resolve, reject) => {
		const req = http.request(options2, res => {
			resolve(false);
		});
		req.on('error', error => {
			resolve(true);
		});
		req.end();
	})
	return secondTest;
}

export async function wifiScan() {
	// scan all ip addresses in the current subnet
	//return ["192.168.1.99"]
	const ip = getLocalIp();
	const ipParts = ip.split('.');
	const subnet = ipParts.slice(0, 3).join('.');
	const ips: string[] = [];
	for (let i = 1; i < 255; i++) {
		ips.push(`${subnet}.${i}`);
	}
	let eligibleIpsPromises = [];
	for (let i = 0; i < ips.length; i++) {
		eligibleIpsPromises.push(checkIp(ips[i]));
	}
	const results = await Promise.all(eligibleIpsPromises);
	const eligibleIps = [];
	for (let i_2 = 0; i_2 < ips.length; i_2++) {
		if (results[i_2]) {
			eligibleIps.push(ips[i_2]);
		}
	}
	return eligibleIps;
}
