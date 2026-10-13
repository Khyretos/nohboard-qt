#!/bin/bash
f=/home/khyretos/Docker/Personal-projects/kk-wt-nbwin/nohboard-qt/.forgejo/workflows/windows.yml
test -e $f || { echo "error: .forgejo/workflows/windows.yml: not written"; exit 1; }
python3 - $f <<'P'
import sys, yaml
f = sys.argv[1]
try:
    d = yaml.safe_load(open(f))
except Exception as e:
    print(f"error: windows.yml: yaml: {e}"); sys.exit(1)
on = d.get(True, d.get("on"))
need = []
if not isinstance(on, dict) or not {"push", "pull_request", "workflow_dispatch"} <= set(on):
    need.append("on: needs push, pull_request, workflow_dispatch")
jobs = d.get("jobs") or {}
if len(jobs) != 1:
    need.append("exactly one job")
else:
    j = list(jobs.values())[0]
    if j.get("runs-on") != "windows":
        need.append("runs-on: windows")
    txt = open(f).read()
    for n in ["C:\\msys64", "ucrt64", "cmake --install", "dist", "System32", "Start-Sleep", "upload-artifact", "NohBoard.exe", "Stop-Process"]:
        if n not in txt:
            need.append("missing " + n)
    if "defaults" not in j and "shell" not in txt:
        need.append("a shell for the msys2 steps")
if need:
    print("error: windows.yml: " + "; ".join(need)); sys.exit(1)
P
