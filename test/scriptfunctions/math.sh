#!/bin/bash
cat > avxtemp-math.avs << EOF
muldiv_result = MulDiv(2, 3, 2) # Expected 3

floor_result = Floor(1.6) # Expected 1
floor_result2 = Floor(-1.2) # Expected -2

ceil_result = Ceil(1.2) # Expected 2
ceil_result2 = Ceil(-1.6) # Expected -1

round_result = Round(0.5) # Expected 1
round_result2 = Round(-1.2) # Expected -1

pi_result = Pi() #3.141593

sin_result = Sin(pi_result / 4) # Expected 0.707107
sin_result2 = Sin(pi_result / 2) # Expected 1.000000

cos_result = Cos(pi_result / 4) # Expected 0.707107
cos_result2 = Cos(pi_result / 2) # Expected -0.000000

exp_result = Exp(1) # Expected 2.718282
exp_result2 = Exp(2) # Expected 7.389056

log_result = Log(1) # Expected 0.000000
log_result2 = Log(exp_result) # Expected 1.000000

pow_result = Pow(2, 3) # Expected 8.000000
pow_result2 = Pow(8, 1./3) # Expected 2.000000

sqrt_result = Sqrt(2) # Expected 1.414214
sqrt_result2 = Sqrt(3) # Expected 1.732051

abs_result = Abs(-1.5) # Expected 1.500000
abs_result2 = Abs(-4) # Expected 4

sign_result = Sign(4) # Expected 1
sign_result2 = Sign(-4) # Expected -1
sign_result3 = Sign(0) # Expected 0

int_result = int(1.6) # Expected 1
int_result2 = int(-1.2) # Expected -1

frac_result = Frac(3.5) # Expected 0.500000
frac_result2 = Frac(-1.8) # Expected -0.800000

float_result = float(4) / 3 # Expected 1.333333

# rand_result = Rand() # No option for deterministic testing

spline_result = Spline(5, 0, 0, 10, 10, 20, 0, false) # Expected value 5.000000
spline_result2 = Spline(5, 0, 0, 10, 10, 20, 0, true) # Expected value: 6.875000
EOF

CHECK=(\
"muldiv_result" "3" \
"floor_result" "1" \
"floor_result2" "-2" \
"ceil_result" "2" \
"ceil_result2" "-1" \
"round_result" "1" \
"round_result2" "-1" \
"pi_result" "3.141593" \
"sin_result" "0.707107" \
"sin_result2" "1.000000" \
"cos_result" "0.707107" \
"cos_result2" "-0.000000" \
"exp_result" "2.718282" \
"exp_result2" "7.389056" \
"log_result" "0.000000" \
"log_result2" "1.000000" \
"pow_result" "8.000000" \
"pow_result2" "2.000000" \
"sqrt_result" "1.414214" \
"sqrt_result2" "1.732051" \
"abs_result" "1.500000" \
"abs_result2" "4" \
"sign_result" "1" \
"sign_result2" "-1" \
"sign_result3" "0" \
"int_result" "1" \
"int_result2" "-1" \
"frac_result" "0.500000" \
"frac_result2" "-0.800000" \
"float_result" "1.333333" \
"spline_result" "5.000000" \
"spline_result2" "6.875000" \
)

CHECK_LEN=${#CHECK[@]}
FAILED_TEST="no"

for ((i=0; i<${CHECK_LEN}; i+=2)); do
    VAR="${CHECK[$i]}"
    EXPECTED="${CHECK[$i+1]}"
    echo "Checking whether $VAR is $EXPECTED"
    VAL="$(./tools/avsgrabber avxtemp-math.avs "$VAR")"
    test ! "x$VAL" = "x$EXPECTED" && FAILED_TEST="yes" && echo "FAILED"
    echo "Got $VAL"
done

rm -f avxtemp-math.avs

if test "x$FAILED_TEST" = "xyes"; then
    exit 2
else
    exit 0
fi
