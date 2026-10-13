# NB3: R2 braces

Job: `r.json` job nb-r2 (patch `src/mainwindow.cpp`), check `r2.sh`. The Coder added `{` to the `if` but left the following `else {` on its own line (`}` newline `else {`), which the repo's .clang-format style would reflow. The check passed, so only a style review caught it. Claude joined them to `} else {`. Lesson: when a patch adds braces to an `if` that already has an `else`, say "also put `} else {` on one line" in the prompt and needle for it.
