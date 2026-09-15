# Mini Calculator

A tiny GUI calculator for Windows, written in C++ (Win32 API) with **no external dependencies**. Builds into a single, portable `.exe` — download and run, no install needed.

## Read this first

One brain, three faces — everything uses the same `Calculator` class:

- **`Calculator.h`** — the brain (OOP class: private state + public methods)
- **`calculator.cpp`** — the GUI: a window and buttons. Each click calls one `Calculator` method, then prints `getDisplay()`.
- **`console.cpp`** — a terminal version that does the *exact same* thing in a tiny `main()`. The best way to learn the brain.
- **`logic_test.cpp`** — automated tests that prove the brain works.

### The flow in 4 lines

```
button/key pressed
   -> Calculator method call  (inputDigit / setOperator / equals / ...)
   -> state changes inside the Calculator object
   -> getDisplay() returns what the screen should show
```

## Code (easy to read)

- `Calculator.h` — the "brain": a simple OOP class that stores the state and does all the math
- `calculator.cpp` — only the window and buttons; every button press just calls a `Calculator` method

## Features

- Add, subtract, multiply, divide
- Shows the whole expression as you type (e.g. `12-2`), then the result (`10`) on `=`
- Percent (`%`), negate (`+/-`), decimal point
- `C` (clear all), `CE` (clear entry), backspace
- Error handling:
  - **Math error** — overflow / value too big, divide by zero, invalid result
  - **Syntax error** — operator pressed before any number, or two operators in a row
- `5 =` keeps showing `5`; a lone `=` does nothing

## Download

Grab the latest build from the [Releases](https://github.com/Shabab47/Mini-Calculator/releases/latest) page. The `.exe` is self-contained — copy it anywhere and run it.

## Build locally

### MinGW-w64 (recommended)

The GUI app (the downloadable `.exe`):

```
g++ -std=c++17 -O2 -mwindows -municode -static calculator.cpp -o calculator.exe
```

The terminal learning version (no window, great for learning):

```
g++ -std=c++17 -static -municode console.cpp -o console.exe
console.exe
```

### MSVC (Visual Studio)

```
cl /std:c++17 /utf-8 /O2 /EHsc calculator.cpp /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib
```

(`/utf-8` is required because the source uses Unicode symbols.)

## Tests

The `Calculator` class is unit-tested with `logic_test.cpp`:

```
g++ -std=c++17 -static -municode logic_test.cpp -o logic_test.exe
logic_test.exe
```

## How releases work

Pushing a tag starting with `v` triggers GitHub Actions, which compiles the `.exe` and attaches it to a Release automatically:

```
git tag v1.0
git push origin v1.0
```

Bump the version for every new release (the same tag cannot be overwritten). You can also trigger a build without a tag from the **Actions** tab — the `.exe` will be a downloadable run artifact.

## Roadmap

- Keyboard input
- Scientific notation input
- App icon and themes
- More digits of precision