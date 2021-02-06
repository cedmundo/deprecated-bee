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

{ set +x; } 2>/dev/null;
echo "ALL PASSED"
