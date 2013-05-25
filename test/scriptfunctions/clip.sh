#!/bin/bash
cat > avxtemp-clip.avs << EOF
clp = BlankClip(length=777, width=122, height=456, pixel_type="YUY2", fps=90, fps_denominator=23, \
	audio_rate=322440000, channels=3, sample_type="8bit")
clp_floataudio = BlankClip(clp, sample_type="float")
clp_yv12 = BlankClip(clp, pixel_type="YV12")
clp_rgb24 = BlankClip(clp, pixel_type="RGB24")
clp_rgb32 = BlankClip(clp, pixel_type="RGB32")
clp_fieldbased = AssumeFieldBased(clp)
clp_framebased = AssumeFrameBased(clp)
clp_tff = AssumeTFF(clp)
clp_bff = AssumeBFF(clp)
clp_novideo = KillVideo(clp)
clp_noaudio = KillAudio(clp)

width_result = Width(clp) # Expected 122
height_result = Height(clp) # Expected 456

framecount_result = FrameCount(clp) # Expected 777
framerate_result = FrameRate(clp) # Expected 3.913043
frameratenumerator_result = FrameRateNumerator(clp) # Expected 90
frameratedenominator_result = FrameRateDenominator(clp) # Expected 23

audiorate_result = AudioRate(clp) # Expected 322440000
audiolength_result = AudioLength(clp) # Expected -398673440 # Integer overflow
audiolengthf_result = AudioLengthF(clp) # Expected 64025837568.000000
audiochannels_result = AudioChannels(clp) # Expected 3

audiobits_result = AudioBits(clp) # Expected 8
audiobits_result2 = AudioBits(clp_floataudio) # Expected 32

isaudiofloat_result = IsAudioFloat(clp) # Expected false
isaudiofloat_result2 = IsAudioFloat(clp_floataudio) # Expected true

isaudioint_result = IsAudioInt(clp) # Expected true
isaudioint_result2 = IsAudioInt(clp_floataudio) # Expected false

isplanar_result = IsPlanar(clp) # Expected false
isplanar_result2 = IsPlanar(clp_yv12) # Expected true

isinterleaved_result = IsInterleaved(clp) # Expected true
isinterleaved_result2 = IsInterleaved(clp_yv12) # Expected false

isrgb_result = IsRGB(clp) # Expected false
isrgb_result2 = IsRGB(clp_rgb24) # Expected true
isrgb_result3 = IsRGB(clp_rgb32) # Expected true

isyuv_result = IsYUV(clp) # Expected true
isyuv_result2 = IsYUV(clp_rgb24) # Expected false

isrgb24_result = IsRGB24(clp_rgb24) # Expected true
isrgb24_result2 = IsRGB24(clp_rgb32) # Expected false

isrgb32_result = IsRGB32(clp_rgb24) # Expected false
isrgb32_result2 = IsRGB32(clp_rgb32) # Expected true

isyuy2_result = IsYUY2(clp) # Expected true
isyuy2_result2 = IsYUY2(clp_yv12) # Expected false

isyv12_result = IsYV12(clp) # Expected false
isyv12_result2 = IsYV12(clp_yv12) # Expected true

isfieldbased_result = IsFieldBased(clp_fieldbased) # Expected true
isfieldbased_result2 = IsFieldBased(clp_framebased) # Expected false

isframebased_result = IsFrameBased(clp_fieldbased) # Expected false
isframebased_result2 = IsFrameBased(clp_framebased) # Expected true

getparity_result = GetParity(clp_tff) # Expected true
getparity_result2 = GetParity(clp_bff) # Expected false

hasaudio_result = HasAudio(clp_novideo) # Expected true
hasaudio_result2 = HasAudio(clp_noaudio) # Expected false

hasvideo_result = HasVideo(clp_novideo) # Expected false
hasvideo_result2 = HasVideo(clp_noaudio) # Expected true
EOF

CHECK=(\
"width_result" "122" \
"height_result" "456" \
"framecount_result" "777" \
"framerate_result" "3.913043" \
"frameratenumerator_result" "90" \
"frameratedenominator_result" "23" \
"audiorate_result" "322440000" \
"audiolength_result" "-398673440" \
"audiolengthf_result" "64025837568.000000" \
"audiochannels_result" "3" \
"audiobits_result" "8" \
"audiobits_result2" "32" \
"isaudiofloat_result" "false" \
"isaudiofloat_result2" "true" \
"isaudioint_result" "true" \
"isaudioint_result2" "false" \
"isplanar_result" "false" \
"isplanar_result2" "true" \
"isinterleaved_result" "true" \
"isinterleaved_result2" "false" \
"isrgb_result" "false" \
"isrgb_result2" "true" \
"isrgb_result3" "true" \
"isyuv_result" "true" \
"isyuv_result2" "false" \
"isrgb24_result" "true" \
"isrgb24_result2" "false" \
"isrgb32_result" "false" \
"isrgb32_result2" "true" \
"isyuy2_result" "true" \
"isyuy2_result2" "false" \
"isyv12_result" "false" \
"isyv12_result2" "true" \
"isfieldbased_result" "true" \
"isfieldbased_result2" "false" \
"isframebased_result" "false" \
"isframebased_result2" "true" \
"getparity_result" "true" \
"getparity_result2" "false" \
"hasaudio_result" "true" \
"hasaudio_result2" "false" \
"hasvideo_result" "false" \
"hasvideo_result2" "true" \
)

CHECK_LEN=${#CHECK[@]}
FAILED_TEST="no"

for ((i=0; i<${CHECK_LEN}; i+=2)); do
    VAR="${CHECK[$i]}"
    EXPECTED="${CHECK[$i+1]}"
    echo "Checking whether $VAR is $EXPECTED"
    VAL="$(./tools/avsgrabber avxtemp-clip.avs "$VAR")"
    test ! "x$VAL" = "x$EXPECTED" && FAILED_TEST="yes" && echo "FAILED"
    echo "Got $VAL"
done

rm -f avxtemp-clip.avs

if test "x$FAILED_TEST" = "xyes"; then
    exit 2
else
    exit 0
fi

