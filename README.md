# StickyNotes

A small desktop application for creating sticky notes on Linux. Notes can be moved, resized, edited, and are saved locally.

<p align="center">
  <img src="media/sticky_notes.png" alt="StickyNotes on the desktop" width="480">
</p>

## Screenshots

<p align="center">
  <img src="media/creating-stickers.webp" alt="Creating and editing a sticky note" width="300">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="media/markdown.webp" alt="Using markdown" width="300"><br>
  <sub>Creating and editing &nbsp;&nbsp;|&nbsp;&nbsp; Markdown</sub>
</p>

<p align="center">
  <img src="media/stacking.webp" alt="Changing stacking in the context menu" width="300">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="media/colors.webp" alt="Changing colors" width="300"><br>
  <sub>Stacking &nbsp;&nbsp;|&nbsp;&nbsp; Colors</sub>
</p>

## Installation

### Requirements

- Linux with X11/XCB. On Wayland the application runs through XWayland.
- CMake 3.25 or newer
- C++17 compiler
- Qt 6 with Core, Gui, Widgets, and Quick
- X11 and XCB development libraries
- pkg-config

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config qt6-base-dev qt6-declarative-dev libx11-dev libxcb1-dev
```

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake pkgconf qt6-base qt6-declarative libx11 libxcb
```

### Fedora

```bash
sudo dnf install cmake gcc-c++ pkgconf-pkg-config qt6-qtbase-devel qt6-qtdeclarative-devel libX11-devel libxcb-devel
```

## Build

```bash
git clone https://github.com/DartCat25/StickyNotes.git
cd StickyNotes
cmake -S . -B build
cmake --build build -j"$(nproc)"
```

### Run

```bash
./build/StickyNotes
```

Sticker data is stored in `~/.local/share/StickyNotes/stickers`.

## Windows

Windows has not been tested yet. Build and installation instructions will be added after the Windows build is verified.
