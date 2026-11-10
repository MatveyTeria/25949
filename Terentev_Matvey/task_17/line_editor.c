#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAX_COLS 40

char buffer[4096];
int pos = 0;
int len = 0;

static int last_lines = 0;
static int last_cursor_row = 0;

void redraw(char *buf, int pos, int len) {
	printf("\r\033[K");
	int cur_col = 0;
	int cur_row = 0;
	int row = 0;
	int col = 0;
	for (int i = 0; i < len; i++) {
		int word_start = i;
		while (i < len && buf[i] != ' ') i++;
		int word_lenght = i - word_start;
		if (col > 0 && col + word_len > MAX_COLS) {
            printf("\n");
            row++;
			lines++;
            col = 0;
            if (i == pos) {
				cur_row = row;
				cur_col = col;
			}
        }

		for (int j = word_start; j < i; j++) {
			if (j == pos) {
				cur_col = col;
				cur_row = row;
			}
			putchar(buf[j]);
            col++;
		}

		while (i < len && buf[i] == ' ') {
            if (i == pos) {
				cur_row = row;
				cur_col = col;
			}
            putchar(' ');
            i++;
            col++;
        }
	}
	if (pos == len) {
		cur_row = row;
		cur_col = col;
	}
	fflush(stdout);
}

void enable_raw_mode(void) {
		if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    atexit(disable_raw_mode);
}

int main() {
	enable_raw_mode();

	printf("> ");
	fflush(stdout);

	while (true) {
		char c;
		ssize_t n = read(STDIN_FILENO, &c, 1);
		if (n <= 0) break;

		// Ctrl+D (4)
		if (c == 4) {
            if (pos == 0) {
                printf("\nExiting...\n");
                break;
            }
            continue;
        }

		// Backspace
		if ((unsigned char)c == orig_termios.c_cc[VERASE] || c == 127) {
			if (pos <= 0) continue;
			memmove(&buffer[pos - 1], &buffer[pos], len - pos);
			pos--;
			len--;
			redraw(buffer, pos, len);
		}

		if ((unsigned char)c == orig_termios.c_cc[VKILL] || c == 21) {
            len = 0;
            pos = 0;
            redraw(buffer, pos, len);
            continue;
        }

		if (c == '\n' || c == '\r') {
            printf("\n> ");
            fflush(stdout);
            len = 0;
            pos = 0;
            last_lines = 0;
            continue;
        }

		if (isprint((unsigned char)c)) {
            if (len < (int)sizeof(buffer) - 1) {
                memmove(&buffer[pos + 1], &buffer[pos], len - pos);
                buffer[pos] = c;
                len++;
                pos++;
                redraw(buffer, len, pos);
            }
            continue;
        }

		putchar('\a'); 
        fflush(stdout);
	}

	return 0;
}
