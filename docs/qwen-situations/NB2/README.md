# NB2: U3 windows.yml, 3 attempts failed

Job: `u3.json` (new file `.forgejo/workflows/windows.yml`), check `u3.sh`. Wrong output: `wrong-windows.yml`. Corrected: `corrected-windows.yml`.

Mistakes (OVMS Coder, pipeline run 1, 3 check attempts):
1. The Install step is an unquoted one-line scalar starting with `& C:\msys64\...`: YAML reads `&` as an anchor, so the file does not parse (the failure all 3 attempts kept).
2. Configure and Build call a bare `bash -lc` although MSYS2 is not on PATH (the prompt said to call it by full path).
3. No `-DCMAKE_PREFIX_PATH=/ucrt64/qt6-static` (known only from the Windows PC afterwards), and no `$LASTEXITCODE` checks.

Fix: Claude wrote the file: MSYS2_BASH in env, single-quoted bash commands, exit-code checks. Lesson: in a prompt for a workflow with PowerShell, say "every run: starting with & must be a block scalar (`run: |`)" and make the check reject a bare `bash ` call.
