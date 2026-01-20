README.md

# Reflex Game – Champion Edition (Arduino + C)

Two-player reflex game using a colour sensor, RGB LEDs, score LEDs, a start button, and a champion fanfare sound.

## Hardware
- Arduino (Uno/Nano)
- Colour sensor (TCS3200/TCS230 type)
- 1x RGB LED (or 3 separate LEDs: red/green/blue)
- 6x score LEDs (3 per player)
- 1x buzzer
- 1x push button (start)
- Resistors + breadboard + jumper wires

## Pin mapping (Arduino)
Colour sensor:
- S0 = D4
- S1 = D5
- S2 = D6
- S3 = D7
- OUT = D8

RGB LED:
- Red = D9
- Green = D10
- Blue = D11

Buzzer + Button:
- Buzzer = D3
- Start button = D2 (INPUT_PULLUP)

Score LEDs:
- Player 1: A0, A1, A2
- Player 2: A3, A4, A5

## Arduino run steps
1. Open `arduino/reflex_game_arduino/reflex_game_arduino.ino`
2. Select board + port
3. Upload
4. Optional: Serial Monitor at 9600 baud (you’ll see READY)

## PC client build
Windows (MinGW):
gcc pc_client/reflex_game_pc_client.c -o reflex_game.exe

Linux/macOS:
gcc pc_client/reflex_game_pc_client.c -o reflex_game

Run it and enter your port when asked:
- Windows: COM3 (or \\.\COM3)
- Linux: /dev/ttyACM0 or /dev/ttyUSB0
- macOS: /dev/tty.usbmodemXXXX

## Serial commands
- PLAY_START
- PLAY_CHAMPION
- EFFECT
- SHOW_COLOR:<0|1|2>
- READ_COLOR
- CLEAR_GAME / CLEAR_ALL
- SCORE_LED:<player>:<round>
- CELEBRATE:<player>
