import os
import subprocess
import sys

compiler = r"C:\Program Files\LLVM\bin\clang-cl.exe"
ignored = {
    "-mthreads",
    "-g",
    "-frandom-seed",
    "-fno-caret-diagnostics",
    "-Qunused-arguments",
    "-fmessage-length=0",
    "-fno-stack-protector",
    "-Werror",
    "-Wall",
    "-Wdeclaration-after-statement",
    "-Wno-error=unknown-argument",
    "-Wno-unknown-argument",
    "-Wno-unused-macros",
    "-Wno-unused-function",
    "-Wno-implicit-function-declaration",
    "-Wno-int-conversion",
    "-Wno-unused-command-line-argument",
    "-Wno-unused-macros",
    "-Wno-unused-function",
    "-Wno-implicit-function-declaration",
    "-Wno-int-conversion",
    "-dM",
}

args = []
i = 0
while i < len(sys.argv[1:]):
    arg = sys.argv[1:][i]
    if arg in ignored:
        pass
    elif arg == "-m64":
        pass
    elif arg == "-E":
        args.append("/E")
    elif arg == "-c":
        args.append("/c")
    elif arg == "-x":
        if i + 1 < len(sys.argv[1:]):
            nxt = sys.argv[1:][i + 1]
            if nxt.lower() == "c++":
                args.append("/TP")
            else:
                args.append("/TC")
            i += 1
    elif arg == "-o":
        if i + 1 < len(sys.argv[1:]):
            args.append("-o")
            args.append(sys.argv[1:][i + 1])
            i += 1
    elif arg == "-I":
        if i + 1 < len(sys.argv[1:]):
            args.append("/I")
            args.append(sys.argv[1:][i + 1])
            i += 1
    elif arg == "-D":
        if i + 1 < len(sys.argv[1:]):
            args.append("/D")
            args.append(sys.argv[1:][i + 1])
            i += 1
    elif arg == "-include":
        if i + 1 < len(sys.argv[1:]):
            args.append("/FI")
            args.append(sys.argv[1:][i + 1])
            i += 1
    elif arg == "-isystem":
        if i + 1 < len(sys.argv[1:]):
            args.append("/I")
            args.append(sys.argv[1:][i + 1])
            i += 1
    else:
        args.append(arg)
    i += 1

# Preserve the input file(s) and fall back to the compiler for everything else.
cmd = [compiler] + args
print("WRAPPER CMD: " + " ".join(cmd), file=sys.stderr)
sys.exit(subprocess.call(cmd))
