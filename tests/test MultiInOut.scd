(
s.waitForBoot({
	p = {
		// [SinOsc.ar(3), LFTri.ar(6), LFSaw.ar(1.5)],
		// [SinOsc.ar(3.reciprocal), LFTri.ar(6.reciprocal), LFSaw.ar(1.5.reciprocal)],
		// PinkNoise.ar(1.dup(5)), // 5 channels of noise

		var depth = 0.5;
		var freq = 5;

		// var in = DC.ar(1).dup(5); // BREAKS when only output is used (works when input is also plotted)
		var in = DC.ar(1.dup(5));    // OK
		// var in = DC.ar(1);
		// var in = [SinOsc.ar(3), LFTri.ar(6), LFSaw.ar(1.5)];

		// var mod = SinOsc.ar(
		// 	freq,
		// 	in.size.collect({ |i| i * 2pi / in.size }),
		// 	depth / 2,
		// 	1-(depth/2)
		// );

		var out = MultiInOut.ar(in, freq, depth);
		out
		// [in, mod, out].flop.flat
		// [in, mod, out].flat;
		// [in, mod, out].flat.drop(6);
		// [in, out].flop.flat.postln;
		// [in, out].flop.flat
		// out.dup.flat
		// out.size.postln;
		// }.play(outbus: 2);
		// s.scope(3, 2);

	// }.plot(duration: 1);
		}.plot(duration: 1, minval: 0, maxval: 1);
// }.play(outbus: 2)
	// }.plot(duration: 15 / s.sampleRate).plotMode_(\plines);

})
)

MultiOutUGen.subclasses.postlns

/* Observe the graph! */
(
d = SynthDef(\testMulti, {
	var depth = 0.5;
	var freq = 5;
	var mod = SinOsc.ar(freq, in.size.collect({|i| i*2pi/in.size }), depth/2, 1-(depth/2));

	/* BREAKS when only output is used (works when input is also plotted)
	   One UGen is created, with 5 pointers to it's location, so the UGen using this
	   iterates over inputs while also setting output, output is fed into input on
	   successive iterations
	*/
	var in = DC.ar(1).dup(5);

	/* OK in all cases: 5 distinct UGens are created
	*/
	// var in = DC.ar(1.dup(5));

	var out = MultiInOut.ar(
		in, freq, depth
	);

	out = [in, mod, out].flop.flat; // input is "included" in output
	// out = out;

	Out.ar(2, out);
})
)
d.dumpUGens

(
d = SynthDef(\testMulti, {
	var freq = DC.ar(30).dup(2);

	var sin = SinOsc.ar(freq[0]);
	var tri = LFTri.ar(freq[1]);//, sin);

	Out.ar(2, [sin, tri])
})
)
d.dumpUGens
(
{
	var freq = DC.ar(4).dup(2);
	var sin = SinOsc.ar(freq[0]);
	var tri = LFTri.ar(freq[1], add: sin * 0.0001);

	[sin, tri]
}.plot(duration: 1)
)