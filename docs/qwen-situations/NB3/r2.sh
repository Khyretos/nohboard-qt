#!/bin/bash
python3 /tmp/kk-nbwin-chk/needles.py /home/khyretos/Docker/Personal-projects/kk-wt-nbwin/nohboard-qt/src/mainwindow.cpp '+statusBar()->show();' '+statusBar()->showMessage(' '+Failed to start input hook' || exit 1
python3 - /home/khyretos/Docker/Personal-projects/kk-wt-nbwin/nohboard-qt/src/mainwindow.cpp <<'P'
import sys
t = open(sys.argv[1]).read()
i = t.index("Failed to start input hook")
if t.count("statusBar()->showMessage(") != 1 or not (i < t.index("statusBar()->showMessage(") < i + 400):
    print("error: mainwindow.cpp: showMessage must appear once, right after the qWarning of 'Failed to start input hook' (inside that if)")
    sys.exit(1)
P
