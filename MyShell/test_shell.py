#!/usr/bin/env python3
import os
import pexpect

SHELL_PATH = "./build/shell"
WORK_DIR = os.getcwd()
OUT_FILE = os.path.join(WORK_DIR, "out.txt")
ERR_FILE = os.path.join(WORK_DIR, "err.txt")
HIST_FILE = os.path.join(WORK_DIR, "hist.txt")
TEST_FILE = os.path.join(WORK_DIR, "tempfile_test_autocomplete.txt")

def run_and_check(child, cmd, expected, label, expect_prompt=True):
    child.sendline(cmd)
    if expect_prompt:
        child.expect(r"\$")
    output = child.before
    if isinstance(output, bytes):
        output = output.decode("utf-8", "ignore")
    if expected in output:
        print(f"✅ {label}\n   Expected: {expected}\n   Output: {output.strip()}\n")
    else:
        print(f"❌ {label}\n   Expected: {expected}\n   Output: {output.strip()}\n")

def create_temp_file():
    with open(TEST_FILE, "w") as f:
        f.write("test file content\n")

def cleanup():
    for f in [OUT_FILE, ERR_FILE, HIST_FILE, TEST_FILE]:
        try:
            os.remove(f)
        except FileNotFoundError:
            pass

def run_tests():
    print(f"📂 Test working directory: {WORK_DIR}\n")
    cleanup()
    create_temp_file()

    child = pexpect.spawn(SHELL_PATH, encoding="utf-8", timeout=3)
    child.expect(r"\$")

    run_and_check(child, "invalidcommand", "not found", "Handles invalid command")
    run_and_check(child, "type echo", "shell builtin", "type builtin: builtins")
    run_and_check(child, "type ls", "is /", "type builtin: executable files")
    run_and_check(child, "pwd", WORK_DIR, "pwd builtin")

    run_and_check(child, f'echo "redirected" > "{OUT_FILE}"', "", "Redirect stdout (command issued)")
    if os.path.exists(OUT_FILE):
        run_and_check(child, f"cat {OUT_FILE}", "redirected", "Redirect stdout")
    else:
        print("❌ Redirect stdout\n   Expected: redirected\n   Output: <file not created>\n")

    run_and_check(child, f'ls notfoundfile 2> "{ERR_FILE}"', "", "Redirect stderr (command issued)")
    if os.path.exists(ERR_FILE):
        with open(ERR_FILE) as f:
            content = f.read()
        if "No such file" in content:
            print(f"✅ Redirect stderr\n   Expected: No such file\n   Output: {content.strip()}\n")
        else:
            print(f"❌ Redirect stderr\n   Expected: No such file\n   Output: {content.strip()}\n")
    else:
        print("❌ Redirect stderr\n   Expected: file\n   Output: <file not created>\n")

    run_and_check(child, 'echo one | grep on | grep e', "one", "Multi-stage pipeline")
    run_and_check(child, f'cat < {OUT_FILE}', "redirected", "Input redirection")

    run_and_check(child, 'history', "history", "History builtin")
    run_and_check(child, f'history -w "{HIST_FILE}"', "", "History write file")
    run_and_check(child, f'history -r "{HIST_FILE}"', "", "History read file")
    run_and_check(child, 'history', "history", "History file roundtrip")

    # Autocompletion for builtin
    child.sendline("")  # clear line
    child.expect(r"\$")
    child.send("ec")
    child.send("\t")
    child.send("\t")
    child.sendline("")
    child.expect(r"\$")
    output = child.before
    if "echo" in output:
        print(f"✅ Autocompletion for builtin\n   Expected: echo\n   Output: {output.strip()}\n")
    else:
        print(f"❌ Autocompletion for builtin\n   Expected: echo\n   Output: {output.strip()}\n")

    # Autocompletion for executable
    child.sendline("")  # clear line
    child.expect(r"\$")
    child.send("ls")
    child.send("\t")
    child.send("\t")
    child.sendline("")
    child.expect(r"\$")
    output = child.before
    if "ls" in output:
        print(f"✅ Autocompletion for executable\n   Expected: ls\n   Output: {output.strip()}\n")
    else:
        print(f"❌ Autocompletion for executable\n   Expected: ls\n   Output: {output.strip()}\n")

    # Autocompletion for file
    base = os.path.basename(TEST_FILE)
    prefix = base[:-4]
    child.sendline("")  # clear line
    child.expect(r"\$")
    child.send(f"cat {prefix}")
    child.send("\t")
    child.send("\t")
    child.sendline("")
    child.expect(r"\$")
    output = child.before
    if base in output:
        print(f"✅ Autocompletion for file\n   Expected: {base}\n   Output: {output.strip()}\n")
    else:
        print(f"❌ Autocompletion for file\n   Expected: {base}\n   Output: {output.strip()}\n")

    # Clean exit
    child.sendline("exit")
    child.expect(pexpect.EOF)
    print("✅ Shell exited\n")

if __name__ == "__main__":
    try:
        run_tests()
    finally:
        cleanup()

