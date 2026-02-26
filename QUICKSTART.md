# Quick Start Guide for CachyOS + Hyprland

## Quick Install (Recommended)

For Hyprland users, I recommend building **Wayland-only** to avoid X11 dependency issues.

### Step 1: Install Dependencies

```bash
sudo pacman -S cmake qt6-base libinput libudev ninja
```

### Step 2: Use the Build Script (Easiest)

```bash
cd /media/development/repos/nohboard-qt
chmod +x build_wayland.sh
./build_wayland.sh
```

This will automatically:

- Check dependencies
- Configure for Wayland only
- Build the project
- Tell you if anything is missing

### Step 3: Set Up Permissions

```bash
# Add yourself to the input group (required for Wayland input capture)
sudo usermod -a -G input $USER

# Log out and log back in (or reboot)
```

### Step 4: Run

```bash
cd build
./NohBoard
```

---

## Manual Build (Alternative)

If you prefer to build manually:

### Option A: Wayland Only (Recommended for Hyprland)

```bash
cd /media/development/repos/nohboard-qt
rm -rf build
mkdir build && cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_X11=OFF \
  -DBUILD_WAYLAND=ON \
  -G Ninja

cmake --build .
```

### Option B: Both X11 and Wayland

If you want both (requires libxtst):

```bash
# Install X11 dependencies
sudo pacman -S libx11 libxtst libxext

# Build
cd /media/development/repos/nohboard-qt
rm -rf build
mkdir build && cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_X11=ON \
  -DBUILD_WAYLAND=ON \
  -G Ninja

cmake --build .
```

---

## Troubleshooting

### "X11 Record extension not found"

This happens when you try to build with X11 support but don't have libxtst:

**Solution 1: Install libxtst** (if you want X11 support)

```bash
sudo pacman -S libxtst
```

**Solution 2: Build Wayland-only** (recommended for Hyprland)

```bash
cmake .. -DBUILD_X11=OFF -DBUILD_WAYLAND=ON
```

### "libinput not found"

```bash
sudo pacman -S libinput libudev
```

### "Failed to start input hook" (Runtime Error)

This means you don't have permission to access input devices. Two solutions:

**Solution 1: Add yourself to input group** (recommended)

```bash
sudo usermod -a -G input $USER
# Then log out and back in
```

**Solution 2: Test with sudo** (temporary, for testing only)

```bash
sudo ./NohBoard
```

### Verify Your Setup

Check if you're in the input group:

```bash
groups | grep input
```

If you see "input" in the output, you're good to go!

---

## What You Should See

When running successfully:

```
Starting NohBoard Qt
Platform: wayland
Creating Wayland input hook
```

If you see this, you're running native Wayland! 🎉

Press keys and click mouse buttons - they should be highlighted on the keyboard widget.

---

## File Structure for Manual Setup

If copying files manually, use this structure:

```
nohboard-qt/
├── CMakeLists.txt
├── build_wayland.sh
├── src/
│   ├── main.cpp
│   ├── mainwindow.h
│   ├── mainwindow.cpp
│   ├── keyboardwidget.h
│   ├── keyboardwidget.cpp
│   └── input/
│       ├── inputhook.h
│       ├── inputhook_x11.cpp
│       ├── inputhook_wayland.cpp
│       └── inputhook_factory.cpp
```

---

## Performance Tips

For best performance on Hyprland:

1. Build Wayland-only (no X11)
2. Make sure you're running native Wayland (not XWayland)
3. Check with: `echo $XDG_SESSION_TYPE` - should say "wayland"

---

## Next Steps

Once running:

- Customize keyboard layout in `keyboardwidget.cpp`
- Adjust colors and themes
- Configure window position
- Add configuration file support

Enjoy! 🚀
