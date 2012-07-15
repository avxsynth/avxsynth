#!/bin/bash
cat > avxtemp-string.avs << EOF
lcase_result = LCase("AvxSynth") # Expected avxsynth
ucase_result = UCase("AvxSynth") # Expected AVXSYNTH

strlen_result = StrLen("AvxSynth") # Expected 8

revstr_result = RevStr("AvxSynth") # Expected htnySvxA

leftstr_result = LeftStr("AvxSynth", 3) # Expected Avx
rightstr_result = RightStr("AvxSynth", 5) # Expected Synth
midstr_result = MidStr("AvxSynth", 4, 3) # Expected Syn

findstr_result = FindStr("AvxSynthAvxSynthAvxSynth", "Synth") # Expected 4

chr_result = Chr(65) + Chr(118) + Chr(120) # Expected Avx

# time_result = Time() # non-determinstic
EOF

CHECK=(\
"lcase_result" "avxsynth" \
"ucase_result" "AVXSYNTH" \
"strlen_result" "8" \
"revstr_result" "htnySxvA" \
"leftstr_result" "Avx" \
"rightstr_result" "Synth" \
"midstr_result" "Syn" \
"findstr_result" "4" \
"chr_result" "Avx" \
)

CHECK_LEN=${#CHECK[@]}
FAILED_TEST="no"

for ((i=0; i<${CHECK_LEN}; i+=2)); do
    VAR="${CHECK[$i]}"
    EXPECTED="${CHECK[$i+1]}"
    echo "Checking whether $VAR is $EXPECTED"
    VAL="$(./tools/avsgrabber avxtemp-string.avs "$VAR")"
    test ! "x$VAL" = "x$EXPECTED" && FAILED_TEST="yes" && echo "FAILED"
    echo "Got $VAL"
done

rm -f avxtemp-string.avs

if test "x$FAILED_TEST" = "xyes"; then
    exit 2
else
    exit 0
fi
