# MultiInOut-UGen
An example UGen which reads in multichannel input and generate multichannel output.

In order to focus on this topic, input and output rates are limited to audio rate, and 
parameters (modFreq and modDepth) are not modulatable, i.e. "scalar rate" or 
"initialization-rate", for brevity and simplicity. A fully-featured UGen would support 
audio- and control-rate output, as well as modulatable input parameters (at various 
rates).

## Building
This directory structure follows that of SuperCollider's 
[example-plugins](https://github.com/supercollider/example-plugins)
repo, and build instructions can be found there.
