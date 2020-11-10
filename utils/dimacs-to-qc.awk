/^p/ {print $3; print $4*2}
/^e/ {v=$2-1; w=$3-1; print v "," w; print w "," v}
