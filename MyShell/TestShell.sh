#!/bin/bash

# Path to your compiled shell binary
BIN="./build/shell"

# Colors for messages
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
RESET=$(tput sgr0)

fail() {
  echo "${RED}❌ Test failed:${RESET} $1"
  exit 1
}

pass() {
  echo "${GREEN}✅ Passed:${RESET} $1"
}

echo "Running integration tests..."

# --- Test 1: echo ---
echo "echo Hello World" | $BIN | grep -q "^Hello World$" || fail "echo should print 'Hello World'"
pass "echo"

# --- Test 2: pwd ---
PWD_EXPECTED=$(pwd)
echo "pwd" | $BIN | grep -q "^$PWD_EXPECTED$" || fail "pwd should print current working directory"
pass "pwd"

# --- Test 3: invalid command ---
echo "nosuchcommand" | $BIN 2>&1 | grep -qi "not found" || fail "should warn on invalid command"
pass "invalid command handling"

# --- Test 4: cd ---
echo -e "cd /\npwd" | $BIN | grep -q "^/$" || fail "cd to root should succeed"
pass "cd to root"

# --- Test 5: type ---
echo "type echo" | $BIN | grep -q "echo is a shell builtin" || fail "'type echo' should say it's a builtin"
pass "type command"

# --- Test 6: redirect output ---
TMPFILE="/tmp/myshell_test_output.txt"
echo "echo redirected > $TMPFILE" | $BIN
grep -q "redirected" "$TMPFILE" || fail "output redirection failed"
rm -f "$TMPFILE"
pass "output redirection"

# --- Test 7: History read/write ---
HISTFILE="/tmp/myshell_history_test.txt"
rm -f "$HISTFILE"

# Add a command to history and write it to the file
echo -e "echo history line 1\nhistory -w $HISTFILE" | HISTFILE=$HISTFILE $BIN

# Read back the history file and check for the line
echo -e "history -r $HISTFILE\nhistory" | HISTFILE=$HISTFILE $BIN | grep -q "history line 1" || fail "history read/write failed"
rm -f "$HISTFILE"
pass "history read/write"

echo
echo "${GREEN}🎉 All integration tests passed!${RESET}"

