#!/bin/bash
BEE=${1:-./bee}

assert() {
  { set +x; } 2>/dev/null;
  set -o pipefail;
  PROGRAM="$1";
  EXPECTED="$2";

  RESULT=$($BEE "$PROGRAM" 2>&1);
  if [ "$RESULT" != "$EXPECTED" ]; then
      echo "FAIL: result \"$RESULT\" != expected \"$EXPECTED\" on program:
        $PROGRAM";
      exit 1;
  fi
  set -x
}

echo "Bee 0.0.2-alpha incremental"
echo "==========================="
set -x

assert "1" "1"
assert "12" "12"
assert "-123" "-123"

assert "1+1" "2"
assert "2-1" "1"
assert "1+1+1-4" "-1"

assert "2*2" "4"
assert "6/2" "3"
assert "2+1*3" "5"
assert "4-6/2" "1"
assert "4%2" "0"
assert "3%2" "1"

assert "-2" "-2"
assert "+2" "2"
assert "--2" "2"
assert "-+2" "-2"
assert "+-2" "-2"
assert "+++2" "2"
assert "(1+1)*3" "6"
assert "(-2)+3" "1"

assert "1&1" "1"
assert "1&0" "0"
assert "0&1" "0"
assert "0&0" "0"

assert "1&&1" "1"
assert "1&&0" "0"
assert "0&&1" "0"
assert "0&&0" "0"

assert "1|1" "1"
assert "1|0" "1"
assert "0|1" "1"
assert "0|0" "0"

assert "1||1" "1"
assert "1||0" "1"
assert "0||1" "1"
assert "0||0" "0"

assert "1+1&1-1" "0"
assert "1*(2-1)&(6/3-2)" "0"

assert "1|1&0" "1"
assert "(1|1&0)&0" "0"

assert "1==1" "1"
assert "1!=1" "0"
assert "1>=1" "1"
assert "1>1" "0"
assert "1<=1" "1"
assert "1<1" "0"

assert "2+1>2-1" "1"
assert "2+1<2-1" "0"

{ set +x; } 2>/dev/null;
echo "ALL PASSED"
