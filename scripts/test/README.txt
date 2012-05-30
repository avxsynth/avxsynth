AvxSynth Automatic Regression Testing (ver1)

COMPAT: Python2.7, Python3
USAGE: python AvxTestSuite.py [Infile] [Outfile]

This script will automatically run through a list of
AvxSynth scripts (.avs) and record the MD5 digest of
the primary output. The primary output is defined as
the video information except for audio-only clips
where it is the audio information.

The accepted file format is a comma-separated-value
file with either UNIX or DOS line endings in the system
character encoding. Non-ASCII characters should be
avoided for this reason. A line may either be a SCRIPT
element in this format:

SCRIPT,filename.avs,hash_sum

or an EXTRAFILE associated with the most recent script

extra_filename,hash_sum

If the first non-whitespace character on a line is "#",
then that line is skipped. Commas within quotations (")
will not be parsed, so all filenames should be quoted.


CAVEAT

Passing automated regression testing only indicates that
the output is identical to the recorded output. It makes
no guarantee of correct behavior. Likewise, a test
failure only indicates that output has changed. If the
recorded output is that of an incorrect result, a test
failure may well indicate that the new behavior is now
corrected.

