# NB1: U2 stray signalReady calls

Job: nohboard U2 (hook start result), four patch jobs on `src/input/inputhook_win.cpp` (`u2.json`, checks `u2b.sh`, `u2d.sh`).
Input: `input-inputhook_win.cpp`. Wrong output: `wrong-inputhook_win.cpp`. Corrected: `corrected-inputhook_win.cpp`.

Mistakes (OVMS Coder, 1 pipeline run, check loop 1-2 attempts per job):
1. nb-u2b needed a second check attempt and ended with `impl->signalReady(false);` twice in the failure branch of `threadMain`.
2. Some job added `impl->signalReady(false);` to the end of `InputHookWin::stop()`, which the spec never asked for. The needle checks only look for presence, so it passed.

Fix: Claude deleted both extra lines. Lesson: needle checks must also bound the count of the inserted call (`count == 1`) and name the functions that must not contain it.
