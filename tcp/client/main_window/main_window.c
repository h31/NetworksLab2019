
#include "main_window.h"
#include <pthread.h>
#include <ncurses.h>
#include "../constants.h"

pthread_mutex_t main_window_mutex;

// window with user input
WINDOW* input_window;
// window with all messages
WINDOW* output_window;


// cancel ncurses mode
void main_window_exit() {
    endwin();
}


// reading character from user
int main_window_get_char(void) {
    return wgetch(input_window);
}


void main_window_init_mutex(void) {
    pthread_mutex_init(&main_window_mutex, NULL);
}


void main_window_init(void) {

    initscr();

    output_window = newwin(LINES-INPUT_WINDOW_HEIGHT, COLS, 0, 0);
    input_window = newwin(INPUT_WINDOW_HEIGHT, COLS, LINES-INPUT_WINDOW_HEIGHT, 0);
    box(input_window,'|', '-');

    curs_set(0); //cursor visibility = 0
    cbreak(); //non-blocking input
    noecho(); //invisible to the user input
    keypad(input_window, TRUE);  // enable system buttons

    wrefresh(output_window);
    wrefresh(input_window);

    main_window_init_mutex();
}


void main_window_write_to_input_window(char* string) {
    pthread_mutex_lock(&main_window_mutex);

    main_window_clear_input_window();

    mvwaddstr(input_window, 1, 1, string);
    wrefresh(input_window);

    pthread_mutex_unlock(&main_window_mutex);
}


void main_window_clear_input_window(void) {
    wclear(input_window);
    box(input_window,'|', '-');
    wrefresh(input_window);
}


void main_window_clear_output_window(void) {
    wclear(output_window);
    wrefresh(output_window);
}


// write all messages to output window
void main_window_write_to_output_window(char* messages[], int messages_size) {
    pthread_mutex_lock(&main_window_mutex);

    main_window_clear_output_window();

    for (int i = 0; i < messages_size; ++i) {
        mvwaddstr(output_window, i+1, 1, messages[i]);
    }
    wrefresh(output_window);

    pthread_mutex_unlock(&main_window_mutex);
}