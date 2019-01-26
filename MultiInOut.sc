MultiInOut : MultiOutUGen {

	*ar { arg inputArray, modFreq = 0.2, modDepth = 1;

		// this.multiNewList used for expanding the input array.
		// Multichannel inputs are appended to the end of the
		// argument list to accomodate variable input array size.
		^this.multiNewList(
			['audio', modFreq, modDepth] ++ inputArray.asArray
		)
    }

	// Override MultiOutUGen:init to assign inputs variable
	// and set the number of outputs.
	init { arg ... allInputs;
		var numOutputs;

		// Store the UGen's inputs to the 'inputs' instance variable.
		inputs = allInputs;

		// Because the UGen's output is just a modulated modulated version
		// of the inputs, the number of outputs will equal the size of the
		// inputArray (excludes modFreq and modDepth inputs).
		numOutputs = allInputs.size - 2;

		^this.initOutputs(numOutputs, rate)
	}
}