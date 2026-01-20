#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

#define RED_LED 9
#define GREEN_LED 10
#define BLUE_LED 11

#define BUZZER_PIN 3
#define START_BUTTON_PIN 2

#define PLAYER1_LED1 A0
#define PLAYER1_LED2 A1
#define PLAYER1_LED3 A2
#define PLAYER2_LED1 A3
#define PLAYER2_LED2 A4
#define PLAYER2_LED3 A5

// Notes (generic tones)
#define NOTE_G3  196
#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_AS5 932
#define NOTE_C6  1047

int redFreq, greenFreq, blueFreq;
String command = "";

// ---- Helpers ----
void clearGameLEDs();
void clearScoreLEDs();
void clearAllLEDs();

void excitementEffect();
void showColor(int color);
int  readColorSensor();

void setScoreLED(int player, int round);
void celebrationAnimation(int player);

// ---- Sounds (Champion edition) ----
void playGameStartSound();
void playChampionFanfare();

void setup() {
  Serial.begin(9600);

  pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // 20% scaling for stable sensor readings
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  pinMode(PLAYER1_LED1, OUTPUT);
  pinMode(PLAYER1_LED2, OUTPUT);
  pinMode(PLAYER1_LED3, OUTPUT);

  pinMode(PLAYER2_LED1, OUTPUT);
  pinMode(PLAYER2_LED2, OUTPUT);
  pinMode(PLAYER2_LED3, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);

  clearAllLEDs();
  Serial.println("READY");
}

void loop() {
  // Start button (debounced)
  if (digitalRead(START_BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(START_BUTTON_PIN) == LOW) {
      Serial.println("BUTTON_START");
      while (digitalRead(START_BUTTON_PIN) == LOW) { }
    }
  }

  // Serial commands (PC -> Arduino)
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "EFFECT") {
      excitementEffect();
      Serial.println("EFFECT_DONE");
    }
    else if (command.startsWith("SHOW_COLOR:")) {
      int color = command.substring(11).toInt(); // 0 red, 1 green, 2 blue
      showColor(color);
      Serial.println("COLOR_SHOWN");
    }
    else if (command == "READ_COLOR") {
      int detectedColor = readColorSensor();
      Serial.print("DETECTED:");
      Serial.println(detectedColor); // -1 = none/unknown
    }
    else if (command == "CLEAR_GAME") {
      clearGameLEDs();
      Serial.println("GAME_CLEARED");
    }
    else if (command.startsWith("SCORE_LED:")) {
      // SCORE_LED:<player>:<round>
      int player = command.substring(10, 11).toInt();
      int round  = command.substring(12).toInt();
      setScoreLED(player, round);
      Serial.println("SCORE_LED_SET");
    }
    else if (command.startsWith("CELEBRATE:")) {
      int player = command.substring(10).toInt();
      celebrationAnimation(player);
      Serial.println("CELEBRATION_DONE");
    }
    else if (command == "CLEAR_ALL") {
      clearAllLEDs();
      Serial.println("ALL_CLEARED");
    }
    else if (command == "PLAY_START") {
      playGameStartSound();
      Serial.println("START_SOUND_DONE");
    }
    else if (command == "PLAY_CHAMPION") {
      playChampionFanfare();
      Serial.println("CHAMPION_SOUND_DONE");
    }
  }
}

// -------- COLOUR SENSOR (BALANCED) --------
int readColorSensor() {
  // Read red
  digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  redFreq = pulseIn(sensorOut, LOW); delay(10);

  // Read green
  digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  greenFreq = pulseIn(sensorOut, LOW); delay(10);

  // Read blue
  digitalWrite(S2, LOW); digitalWrite(S3, HIGH);
  blueFreq = pulseIn(sensorOut, LOW); delay(10);

  // No-card filter
  if (redFreq > 550 && greenFreq > 550 && blueFreq > 550) return -1;

  int detectedColor = -1;

  // Offsets (tuning)
  int offset_RB = 35;
  int offset_G  = 10;

  if (redFreq < (greenFreq - offset_RB) && redFreq < (blueFreq - offset_RB) && redFreq < 400) {
    detectedColor = 0;
  } 
  else if (greenFreq < (redFreq - offset_G) && greenFreq < (blueFreq - offset_G) && greenFreq < 450) {
    detectedColor = 1;
  } 
  else if (blueFreq < (redFreq - offset_RB) && blueFreq < (greenFreq - offset_RB) && blueFreq < 400) {
    detectedColor = 2;
  }

  return detectedColor;
}

// -------- SOUNDS (CHAMPION EDITION) --------
void playGameStartSound() {
  // Short “ready” jingle (original)
  tone(BUZZER_PIN, NOTE_C5, 120); delay(160);
  tone(BUZZER_PIN, NOTE_E5, 120); delay(160);
  tone(BUZZER_PIN, NOTE_G5, 140); delay(180);
  noTone(BUZZER_PIN);
}

void playChampionFanfare() {
  // Winner fanfare (original)
  int notes[] = { NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, NOTE_G5, NOTE_C6 };
  int dur[]   = { 120,     120,     140,     260,     140,     350     };

  for (int i = 0; i < 6; i++) {
    tone(BUZZER_PIN, notes[i], dur[i]);
    delay(dur[i] + 60);
  }
  noTone(BUZZER_PIN);
}

// -------- EFFECTS + LEDs --------
void excitementEffect() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH);   delay(70); digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH); delay(70); digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);  delay(70); digitalWrite(BLUE_LED, LOW);
  }
}

void showColor(int color) {
  clearGameLEDs();
  if (color == 0) digitalWrite(RED_LED, HIGH);
  else if (color == 1) digitalWrite(GREEN_LED, HIGH);
  else if (color == 2) digitalWrite(BLUE_LED, HIGH);
}

void setScoreLED(int player, int round) {
  if (player == 1) {
    if (round == 1) digitalWrite(PLAYER1_LED1, HIGH);
    else if (round == 2) digitalWrite(PLAYER1_LED2, HIGH);
    else if (round == 3) digitalWrite(PLAYER1_LED3, HIGH);
  } else {
    if (round == 1) digitalWrite(PLAYER2_LED1, HIGH);
    else if (round == 2) digitalWrite(PLAYER2_LED2, HIGH);
    else if (round == 3) digitalWrite(PLAYER2_LED3, HIGH);
  }
}

void celebrationAnimation(int player) {
  for (int i = 0; i < 6; i++) {
    int on = (i % 2);
    if (player == 1) {
      digitalWrite(PLAYER1_LED1, on);
      digitalWrite(PLAYER1_LED2, on);
      digitalWrite(PLAYER1_LED3, on);
    } else {
      digitalWrite(PLAYER2_LED1, on);
      digitalWrite(PLAYER2_LED2, on);
      digitalWrite(PLAYER2_LED3, on);
    }
    delay(200);
  }
}

void clearGameLEDs() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}

void clearScoreLEDs() {
  digitalWrite(PLAYER1_LED1, LOW);
  digitalWrite(PLAYER1_LED2, LOW);
  digitalWrite(PLAYER1_LED3, LOW);
  digitalWrite(PLAYER2_LED1, LOW);
  digitalWrite(PLAYER2_LED2, LOW);
  digitalWrite(PLAYER2_LED3, LOW);
}

void clearAllLEDs() {
  clearGameLEDs();
  clearScoreLEDs();
}
