#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // For usleep
#include <stdbool.h> // For bool type

// Game board dimensions
#define HEIGHT 20
#define WIDTH 40
#define MAX_TAIL_LEN 100 // Maximum number of tail segments

// Snake coordinates (tail segments)
int snakeTailX[MAX_TAIL_LEN];
int snakeTailY[MAX_TAIL_LEN];
int snakeTailLen; // Current number of tail segments (0 initially)

// Game state variables
bool gameover;
int score;
int x, y; // Snake head coordinates
int fruitx, fruity; // Fruit coordinates
int direction; // Current direction: 1=left, 2=down, 3=right, 4=up, 0=stopped

// Functions
void setup();
void generate_fruit();
void draw();
void input();
void logic();

// Function to generate the fruit within the boundary and not on the snake
void generate_fruit() {
    bool on_snake;
    do {
        on_snake = false;
        // Generate coordinates within the playable area (inside walls)
        fruitx = (rand() % (WIDTH - 2)) + 1;
        fruity = (rand() % (HEIGHT - 2)) + 1;

        // Check if fruit spawned on the head
        if (fruitx == x && fruity == y) {
            on_snake = true;
            continue; // Try again
        }

        // Check if fruit spawned on any tail segment
        for (int k = 0; k < snakeTailLen; k++) {
            if (fruitx == snakeTailX[k] && fruity == snakeTailY[k]) {
                on_snake = true;
                break; // No need to check further segments
            }
        }
    } while (on_snake); // Keep trying until a free spot is found
}

// Function to initialize game state
void setup() {
    gameover = false;
    direction = 3; // Start moving right initially
    x = WIDTH / 2;
    y = HEIGHT / 2;
    snakeTailLen = 0; // Start with just the head
    score = 0;
    generate_fruit(); // Place initial fruit
}

// Function to draw the game board, snake, fruit, and score
void draw() {
    clear(); // Clear the ncurses screen

    // Draw top and bottom walls
    for (int i = 0; i < WIDTH; i++) {
        mvaddch(0, i, '#');
        mvaddch(HEIGHT - 1, i, '#');
    }
    // Draw side walls
    for (int i = 1; i < HEIGHT - 1; i++) {
        mvaddch(i, 0, '#');
        mvaddch(i, WIDTH - 1, '#');
    }

    // Draw snake head
    mvaddch(y, x, 'O');

    // Draw snake tail segments
    for (int k = 0; k < snakeTailLen; k++) {
        mvaddch(snakeTailY[k], snakeTailX[k], 'o');
    }

    // Draw fruit
    mvaddch(fruity, fruitx, '*');

    // Print score and instructions below the game area
    mvprintw(HEIGHT, 1, "Score: %d", score);
    mvprintw(HEIGHT + 1, 1, "Use WASD to move, X to quit.");

    refresh(); // Update the physical screen with the changes
}

// Function to get user input
void input() {
    int ch = getch(); // Get character (non-blocking due to timeout)
    int new_direction = direction; // Store potential new direction

    switch (ch) {
        case 'a':
        case 'A':
            new_direction = 1; // Left
            break;
        case 's':
        case 'S':
            new_direction = 2; // Down
            break;
        case 'd':
        case 'D':
            new_direction = 3; // Right
            break;
        case 'w':
        case 'W':
            new_direction = 4; // Up
            break;
        case 'x':
        case 'X':
            gameover = true;
            break;
        default:
            // Ignore other keys
            break;
    }

    // Prevent the snake from reversing direction
    // Allow change only if not moving opposite or if stopped
    if ((new_direction == 1 && direction != 3) ||
        (new_direction == 2 && direction != 4) ||
        (new_direction == 3 && direction != 1) ||
        (new_direction == 4 && direction != 2) ||
         direction == 0) // Allow first move
    {
        direction = new_direction;
    }
}

// Function to update game state (movement, collisions, eating)
void logic() {
    if (gameover) return; // Don't do logic if game is already over

    int prev_hx = x; // Store head position before moving
    int prev_hy = y;

    // Move head based on the current direction
    switch (direction) {
        case 1: x--; break; // Left
        case 2: y++; break; // Down
        case 3: x++; break; // Right
        case 4: y--; break; // Up
        default: break; // No movement if direction is 0
    }

    // Check Wall Collision
    if (x <= 0 || x >= WIDTH - 1 || y <= 0 || y >= HEIGHT - 1) {
        gameover = true;
        return;
    }

    // Check Self Collision (check new head pos against all tail segments)
    for (int i = 0; i < snakeTailLen; i++) {
        if (snakeTailX[i] == x && snakeTailY[i] == y) {
            gameover = true;
            return;
        }
    }

    // Check Fruit Collision
    if (x == fruitx && y == fruity) {
        score += 10;
        if (snakeTailLen < MAX_TAIL_LEN) {
            snakeTailLen++; // Increase length
        } else {
             // Optional: Handle max length reached (e.g., display message)
             mvprintw(HEIGHT / 2, (WIDTH / 2) - 8, "MAX LENGTH!"); // Center roughly
        }
        generate_fruit(); // Generate new fruit
    }

    // Update Tail Position (Shift segments back)
    // Only if there is a tail to move
    if (snakeTailLen > 0) {
        // Iterate downwards to prevent overwriting needed values
        for (int i = snakeTailLen - 1; i > 0; i--) {
            snakeTailX[i] = snakeTailX[i - 1];
            snakeTailY[i] = snakeTailY[i - 1];
        }
        // The first tail segment takes the position where the head *was*
        snakeTailX[0] = prev_hx;
        snakeTailY[0] = prev_hy;
    }
}


int main() {
    // Seed random number generator ONCE
    srand(time(NULL));

    // Initialize ncurses
    initscr();             // Start curses mode
    noecho();              // Don't echo user input
    cbreak();              // Disable line buffering (get input char-by-char)
    nodelay(stdscr, TRUE); // Make getch() non-blocking
    timeout(150);          // Set getch timeout (controls game speed, lower is faster)
    curs_set(0);           // Hide the terminal cursor

    // Initial game setup
    setup();

    // Main game loop
    while (!gameover) {
        draw();  // Draw the current game state
        input(); // Process user input
        logic(); // Update game state
    }

    // Game Over sequence
    nodelay(stdscr, FALSE); // Make getch() blocking again to wait for input
    mvprintw(HEIGHT / 2, (WIDTH / 2) - 5, "GAME OVER!");
    mvprintw(HEIGHT / 2 + 1, (WIDTH / 2) - 10, "Final Score: %d", score);
    mvprintw(HEIGHT / 2 + 2, (WIDTH / 2) - 12, "Press any key to exit...");
    refresh();
    getch(); // Wait for user to press a key

    // Clean up ncurses
    endwin();

    return 0;
}
