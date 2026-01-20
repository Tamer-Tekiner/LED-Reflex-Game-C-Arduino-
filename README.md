# Reflex Game – Mario Edition (Arduino + C)

A two-player reflex game using a colour sensor (TCS3200/TCS230 style), RGB LEDs, score LEDs, a start button, and Mario sound effects.  
The Arduino handles hardware + sensor reading. The PC app (C) controls the game flow over serial.

## How it works
- Press the **start button** on Arduino to begin a new game.
- Each round, the PC app tells Arduino which colour to show.
- Players present the correct colour card to the sensor.
- PC measures reaction time and awards the round to the faster player.
- Score LEDs show who won each round (best of 3).

## Hardware
- Arduino (Uno/Nano works)
- Colour sensor module (TCS3200/TCS230 type)
- RGB LED (or 3 single LEDs)
- 6 score LEDs (3 per player)
- Buzzer
- Start button
- Resistors + jumper wires

## Pin mapping (Arduino)
### Colour sensor
- S0 = D4  
- S1 = D5  
- S2 = D6  
- S3 = D7  
- OUT = D8  

### RGB LED
- Red = D9  
- Green = D10  
- Blue = D11  

### Buzzer + Start button
- Buzzer = D3  
- Start button = D2 (INPUT_PULLUP)

### Score LEDs
- Player 1: A0, A1, A2  
- Player 2: A3, A4, A5  

## Run the Arduino code
1. Open the `.ino` file in Arduino IDE.
2. Select your board + port.
3. Upload.
4. Open Serial Monitor at **9600 baud** if you want to see status messages.

## Run the PC client (C)
### Windows (MinGW example)
```bash
gcc pc_client/reflex_game_pc_client.c -o reflex_game.exe
