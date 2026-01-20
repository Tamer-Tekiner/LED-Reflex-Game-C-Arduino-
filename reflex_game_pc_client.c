// ==================================
//   PC CLIENT (C) – CHAMPION EDITION
// ==================================
//
// Builds a 2-player reflex game controlled via Arduino serial.
// Arduino handles: LEDs, button, buzzer, colour sensor.
// PC handles: rounds, timing, scoring, winner logic.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef _WIN32
  #include <windows.h>
  #define sleep(x)  Sleep((x) * 1000)
  #define usleep(x) Sleep((x) / 1000)
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <termios.h>
  #ifndef CRTSCTS
    #define CRTSCTS 020000000000
  #endif
#endif

#define RED   0
#define GREEN 1
#define BLUE  2

#ifdef _WIN32
  HANDLE serial_port;
#else
  int serial_port;
#endif

typedef struct {
  char name[20];
  int score;
  unsigned long reactionTime;
} Player;

typedef struct {
  int currentRound;
  Player player1;
  Player player2;
} Game;

#ifdef _WIN32
  HANDLE openSerialPort(const char *portname);
#else
  int openSerialPort(const char *portname);
#endif
void closeSerialPort(void);
int sendCommand(const char *command);
char* readResponse(int timeout_ms);

void initGame(Game *game);
void playRound(Game *game);
void showColorWithEffect(int color);
unsigned long measureReactionTime(int targetColor);
void updateScore(Game *game, int winner);
void endGame(Game *game);

int main(void) {
  Game game;
  char portname[80];

  printf("=================================\n");
  printf("   COLOUR SENSOR REFLEX GAME\n");
  printf("        (CHAMPION EDITION)\n");
  printf("=================================\n\n");

  printf("Enter Arduino serial port name\n");
#ifdef _WIN32
  printf("(Windows: COM3 or \\\\.\\COM3)\n");
#else
  printf("(Linux: /dev/ttyACM0 or /dev/ttyUSB0)\n");
  printf("(macOS: /dev/tty.usbmodemXXXX)\n");
#endif
  printf("Port: ");
  scanf("%79s", portname);

  serial_port = openSerialPort(portname);

#ifdef _WIN32
  if (serial_port == INVALID_HANDLE_VALUE) {
#else
  if (serial_port < 0) {
#endif
    printf("ERROR: Could not open serial port.\n");
    return 1;
  }

  printf("\nConnecting to Arduino...\n");
  sleep(2);

  {
    char *response = readResponse(3000);
    if (response && strstr(response, "READY")) printf("Arduino is ready.\n\n");
    else printf("Warning: READY not received (continuing anyway).\n\n");
  }

  srand((unsigned int)time(NULL));

  while (1) {
    initGame(&game);

    printf("\n###########################################\n");
    printf("#   PRESS THE START BUTTON ON ARDUINO     #\n");
    printf("###########################################\n");

    sendCommand("CLEAR_ALL");
    readResponse(500);

    int gameStart = 0;
    while (!gameStart) {
      char *resp = readResponse(200);
      if (resp && strstr(resp, "BUTTON_START")) {
        gameStart = 1;
        printf("\n>> BUTTON_START received. Starting... <<\n");
      }
    }

    if (gameStart) {
      printf("\n>> Start sound...\n");
      sendCommand("PLAY_START");
      readResponse(2000);

      sleep(1);

      sendCommand("CLEAR_ALL");
      readResponse(1000);

      for (game.currentRound = 1; game.currentRound <= 3; game.currentRound++) {
        playRound(&game);
        sleep(2);
      }

      endGame(&game);

      printf("\nRestarting...\n");
      sleep(3);
    }
  }

  closeSerialPort();
  return 0;
}

#ifdef _WIN32
HANDLE openSerialPort(const char *portname) {
  HANDLE hSerial = CreateFileA(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hSerial == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;

  DCB dcb = {0}; dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(hSerial, &dcb)) { CloseHandle(hSerial); return INVALID_HANDLE_VALUE; }

  dcb.BaudRate = CBR_9600;
  dcb.ByteSize = 8;
  dcb.StopBits = ONESTOPBIT;
  dcb.Parity   = NOPARITY;

  if (!SetCommState(hSerial, &dcb)) { CloseHandle(hSerial); return INVALID_HANDLE_VALUE; }

  COMMTIMEOUTS timeouts = {0};
  timeouts.ReadIntervalTimeout         = 50;
  timeouts.ReadTotalTimeoutConstant    = 50;
  timeouts.ReadTotalTimeoutMultiplier  = 10;
  timeouts.WriteTotalTimeoutConstant   = 50;
  timeouts.WriteTotalTimeoutMultiplier = 10;

  if (!SetCommTimeouts(hSerial, &timeouts)) { CloseHandle(hSerial); return INVALID_HANDLE_VALUE; }

  return hSerial;
}
void closeSerialPort(void) { if (serial_port != INVALID_HANDLE_VALUE) CloseHandle(serial_port); }

int sendCommand(const char *command) {
  char cmd[256];
  DWORD bytes_written = 0;
  snprintf(cmd, sizeof(cmd), "%s\n", command);
  if (!WriteFile(serial_port, cmd, (DWORD)strlen(cmd), &bytes_written, NULL)) return -1;
  return (int)bytes_written;
}

char* readResponse(int timeout_ms) {
  static char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  DWORD bytes_read = 0;
  int total = 0;
  int loops = timeout_ms / 100;

  for (int i = 0; i < loops; i++) {
    if (ReadFile(serial_port, buffer + total, (DWORD)(sizeof(buffer) - total - 1), &bytes_read, NULL)) {
      if (bytes_read > 0) {
        total += (int)bytes_read;
        if (strchr(buffer, '\n')) break;
      }
    }
    Sleep(100);
  }
  return (total > 0) ? buffer : NULL;
}
#else
int openSerialPort(const char *portname) {
  int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) return -1;

  struct termios tty;
  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0) { close(fd); return -1; }

  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
  tty.c_iflag &= ~IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;

  tty.c_cc[VMIN]  = 0;
  tty.c_cc[VTIME] = 5;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~(PARENB | PARODD);
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) { close(fd); return -1; }
  return fd;
}
void closeSerialPort(void) { if (serial_port >= 0) close(serial_port); }

int sendCommand(const char *command) {
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "%s\n", command);
  return (int)write(serial_port, cmd, strlen(cmd));
}

char* readResponse(int timeout_ms) {
  static char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  int total = 0;
  int loops = timeout_ms / 100;

  for (int i = 0; i < loops; i++) {
    int n = (int)read(serial_port, buffer + total, (int)(sizeof(buffer) - total - 1));
    if (n > 0) {
      total += n;
      if (strchr(buffer, '\n')) break;
    }
    usleep(100000);
  }
  return (total > 0) ? buffer : NULL;
}
#endif

void initGame(Game *game) {
  game->currentRound = 0;

  snprintf(game->player1.name, sizeof(game->player1.name), "PLAYER 1");
  game->player1.score = 0;
  game->player1.reactionTime = 0;

  snprintf(game->player2.name, sizeof(game->player2.name), "PLAYER 2");
  game->player2.score = 0;
  game->player2.reactionTime = 0;
}

void playRound(Game *game) {
  printf("\n=============================\n");
  printf("           ROUND %d\n", game->currentRound);
  printf("=============================\n\n");

  int p1Color = rand() % 3;
  int p2Color = rand() % 3;

  printf(">> %s's turn!\n", game->player1.name);
  sleep(2);
  game->player1.reactionTime = measureReactionTime(p1Color);
  printf("%s reaction time: %lu ms\n\n", game->player1.name, game->player1.reactionTime);

  sendCommand("CLEAR_GAME");
  readResponse(1000);
  sleep(2);

  printf(">> %s's turn!\n", game->player2.name);
  sleep(2);
  game->player2.reactionTime = measureReactionTime(p2Color);
  printf("%s reaction time: %lu ms\n\n", game->player2.name, game->player2.reactionTime);

  sendCommand("CLEAR_GAME");
  readResponse(1000);

  int winner = (game->player1.reactionTime < game->player2.reactionTime) ? 1 : 2;
  updateScore(game, winner);

  printf("\n--- ROUND RESULT ---\n");
  printf("*** %s WINS! ***\n", (winner == 1) ? game->player1.name : game->player2.name);

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "SCORE_LED:%d:%d", winner, game->currentRound);
  sendCommand(cmd);
  readResponse(1000);
}

void showColorWithEffect(int color) {
  const char *names[] = {"RED", "GREEN", "BLUE"};

  printf("\nGet ready...");
  fflush(stdout);

  int waitTime = 1 + (rand() % 2);
  sleep(waitTime);

  printf("\n[STARTING LED EFFECT]\n");
  sendCommand("EFFECT");
  readResponse(2000);

  printf("\n>>> LED ON: %s <<<\n", names[color]);
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "SHOW_COLOR:%d", color);
  sendCommand(cmd);
  readResponse(1000);
}

unsigned long measureReactionTime(int targetColor) {
  const char *names[] = {"RED", "GREEN", "BLUE"};

  showColorWithEffect(targetColor);

#ifdef _WIN32
  LARGE_INTEGER freq, start, end;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);
#else
  struct timeval start, end;
  gettimeofday(&start, NULL);
#endif

  printf("\nShow the correct colour card to the sensor...\n");

  int detected = -1;
  int warned = 0;

  while (detected != targetColor) {
    sendCommand("READ_COLOR");
    char *resp = readResponse(1000);

    if (resp && strstr(resp, "DETECTED:")) {
      sscanf(resp, "DETECTED:%d", &detected);

      if (detected >= 0 && detected <= 2) {
        if (detected != targetColor && !warned) {
          printf("Wrong colour (%s). Try again...\n", names[detected]);
          warned = 1;
        }
      }
    }

    usleep(100000);
  }

#ifdef _WIN32
  QueryPerformanceCounter(&end);
  unsigned long elapsed = (unsigned long)(((end.QuadPart - start.QuadPart) * 1000) / freq.QuadPart);
#else
  gettimeofday(&end, NULL);
  unsigned long elapsed =
    (unsigned long)((end.tv_sec - start.tv_sec) * 1000UL +
                    (end.tv_usec - start.tv_usec) / 1000UL);
#endif

  printf("Correct! Detected: %s\n", names[detected]);
  return elapsed;
}

void updateScore(Game *game, int winner) {
  if (winner == 1) game->player1.score++;
  else game->player2.score++;
}

void endGame(Game *game) {
  printf("\n\n=================================\n");
  printf("            GAME OVER\n");
  printf("=================================\n\n");

  printf("FINAL SCORES:\n");
  printf("%s: %d\n", game->player1.name, game->player1.score);
  printf("%s: %d\n\n", game->player2.name, game->player2.score);

  int winner = 0;
  if (game->player1.score > game->player2.score) {
    winner = 1;
    printf("*** %s IS THE CHAMPION! ***\n", game->player1.name);
  } else if (game->player2.score > game->player1.score) {
    winner = 2;
    printf("*** %s IS THE CHAMPION! ***\n", game->player2.name);
  } else {
    printf("*** DRAW! ***\n");
  }

  if (winner > 0) {
    printf("\n[CHAMPION FANFARE + CELEBRATION]\n");

    sendCommand("PLAY_CHAMPION");
    readResponse(4000);

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "CELEBRATE:%d", winner);
    sendCommand(cmd);
    readResponse(8000);

    printf("[CELEBRATION COMPLETE]\n");
  }
}
