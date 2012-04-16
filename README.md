## What Is AvxSynth?

AvxSynth is a Linux port of the [AviSynth](http://avisynth.org/mediawiki/Main_Page) toolkit. The objective of the porting effort was to bring the power of AviSynth into the Linux world. In particular, we are interested in AviSynth as a frame server front-end to the encode step of our media pipeline. An overview of the AvxSynth architecture can be found [here](https://github.com/avxsynth/avxsynth/wiki/Overview).

The AvxSynth port was based on the AviSynth 2.5.8 code base, and we are calling this version 4.0, so as to differentiate from the dead AviSynth 3.0 project.

## Getting Started

To build and run AvxSynth, follow [these](https://github.com/avxsynth/avxsynth/wiki/System-Setup) instructions.

## Porting Discussion

You can read about the porting effort [here](https://github.com/avxsynth/avxsynth/wiki/Porting-Discussion).

You can see the status of the built-in function porting [here](https://github.com/avxsynth/avxsynth/wiki/Built-in-Functions). 

## The AvxSynth Frame Server

The AvxSynth toolkit includes a frame server application that delivers decoded frames through stdout. This can be piped to x264. You can see how to use AvxSynth with x264 [here](https://github.com/avxsynth/avxsynth/wiki/AvxSynth-Frame-Server).

## Watch

Click [here](http://www.youtube.com/watch?v=DdaQeMcE0UM&context=C49774bdADvjVQa1PpcFPjEU87afkCgg4WN_KrxYQ2lYo_e_FWKPI=) to see a short video of AvxSynth in action. This video demonstrates the ffmpeg source, ffmpeg scaling, ShowSMPTE, and AvxSubtitle, a new filter that does caption burn-in.

## Error Handling

The AvxSynth error handling model is discussed [here](https://github.com/avxsynth/avxsynth/wiki/Error-Handling).

## Discuss

Doom9 discussion is [here](http://forum.doom9.org/showthread.php?t=164386).

## Contribute

If you would like to contribute to AvxSynth, we have need for developers, testers, and build-install experts. More information can be found [here](https://github.com/avxsynth/avxsynth/wiki/Want-to-Help%3F). 


