for (let counter = 1; counter <= 10000; counter++) {
	let result = "";

	if (counter % 3 == 0) {
		result += "fizz";
	}

	if (counter % 5 == 0) {
		result += "buzz";
	}

	if (result != "") {
		console.log(result);
	}
	else {
		console.log(counter);
	}
}
