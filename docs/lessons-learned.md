# Lessons learned (Coder loop)

| Id | Date | Lesson | Situation |
|---|---|---|---|
| NB1 | 2026-10-13 | Needle checks only prove presence. Bound the count of an inserted call (`count == 1`) and name functions that must not contain it; the Coder added `signalReady(false)` twice and once in `stop()`. | docs/qwen-situations/NB1 |
| NB2 | 2026-10-13 | A workflow `run:` line starting with `&` must be a block scalar; the Coder repeated the YAML anchor error in 3 attempts. Say so in the prompt, and make the check reject a bare `bash ` call when MSYS2 is not on PATH. | docs/qwen-situations/NB2 |
| NB3 | 2026-10-13 | When a patch adds braces to an `if` that has an `else`, ask for `} else {` on one line. | docs/qwen-situations/NB3 |
| NB4 | 2026-10-13 | The Windows PC supplied the tested PowerShell launch check and runner settings as literal text, so Claude applied them directly (literal code is not a Coder job). Lesson for U3 prompts: state `PSExecutionPolicyPreference: Bypass`, `shell: powershell` (5.1), single-quoted bash commands, and that a missing DLL shows a modal box (use SetErrorMode and a window check, not HasExited after a sleep). | docs/qwen-situations/NB2 |
