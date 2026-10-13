#!/bin/bash
python3 /tmp/kk-nbwin-chk/needles.py /home/khyretos/Docker/Personal-projects/kk-wt-nbwin/nohboard-qt/src/input/inputhook_win.cpp '+impl->signalReady(false)' '+qWarning() << "Failed to set Windows hooks"' '+void signalReady(bool ok)' '+hookReady = true' '+readyCond.wakeAll()'
