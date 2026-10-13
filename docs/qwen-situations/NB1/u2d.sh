#!/bin/bash
f=/home/khyretos/Docker/Personal-projects/kk-wt-nbwin/nohboard-qt/src/input/inputhook_win.cpp
python3 /tmp/kk-nbwin-chk/needles.py $f '+hookReady = false' '+readyCond.wait(' '+return impl->hookOk' '+impl->thread->wait()' '+impl->thread = nullptr' '-    impl->thread->start();
    return true;' || exit 1
python3 - $f <<'P'
import sys
t = open(sys.argv[1]).read()
a = t.index("bool InputHookWin::start()")
b = t.index("void InputHookWin::stop()")
s = t[a:b]
if "return true;" in s.split("impl->thread->start();", 1)[1].split("readyCond.wait")[0]:
    print("error: inputhook_win.cpp: start() must not return true before the wait on readyCond")
    sys.exit(1)
P
