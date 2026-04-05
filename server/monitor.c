#include <fhttpd.h>
#include <curses.h>
#include <unistd.h>
#include <stb_ds.h>

char* argv0;

static int     my, mx;
static WINDOW* title;
static WINDOW* info;
static WINDOW* keys;
static time_t  start;
static void*   thread;

#define SHIFT 4
#define TITLE_HEIGHT (3)
#define INFO_HEIGHT (1 + 1 + 1 + 5 + 1)

static void thread_main(void* arg) {
	fhttpd_loop();
}

static void erasearea(WINDOW* win, int lines, int cols, int y, int x) {
	int iy, ix;

	for(iy = y; iy < y + lines; iy++) {
		for(ix = x; ix < x + cols; ix++) {
			mvwaddch(win, iy, ix, ' ');
		}
	}
}

static void label(WINDOW* win, int y, int x, const char* txt, const char* str) {
	mvwaddstr(win, y, x, txt);
	wmove(win, y, x + 24);
	waddstr(win, ": ");
	waddstr(win, str);
}

static void draw_info(void) {
	char   buf[128];
	time_t diff = fpr_time() - start;
	int    y = 3, x = 1 + SHIFT;
	int    i;
	int    conn = 0;
	int    days, hours, minutes, seconds;

	erasearea(info, INFO_HEIGHT - 1 - 1 - 1 - 1, mx - 2, 3, 1);

	days	= diff / 60 / 60 / 24;
	hours	= (diff / 60 / 60) % 24;
	minutes = (diff / 60) % 60;
	seconds = (diff) % 60;
	sprintf(buf, "%d Day%s  %d Hour%s  %d Minute%s  %d Second%s", days, days == 1 ? "" : "s", hours, hours == 1 ? "" : "s", minutes, minutes == 1 ? "" : "s", seconds, seconds == 1 ? "" : "s");
	label(info, y++, x, "Uptime", buf);

#if defined(MULTITHREAD)
	for(i = 0; i < arrlen(server_workers); i++) {
		fpr_mutex_lock(server_workers[i].mutex);
		conn += arrlen(server_workers[i].clients);
		fpr_mutex_unlock(server_workers[i].mutex);
	}
#else
	conn = hmlen(server_clients);
#endif
	sprintf(buf, "%d", conn);
	label(info, y++, x, "Connection In Use", buf);

	sprintf(buf, "%d", arrlen(module_modules));
	label(info, y++, x, "Active Modules", buf);
}

static void init_screen(void) {
	int y, x;

	wbkgd(stdscr, COLOR_PAIR(1));
	werase(stdscr);

	for(y = 0; y < my; y++) {
		for(x = 0; x < mx; x++) {
			mvwaddch(stdscr, y, x, ACS_CKBOARD);
		}
	}
}

static void init_windows(void) {
	int  i;
	char infotext[512];

	/* title */
	wbkgd(title, COLOR_PAIR(2));
	wresize(title, TITLE_HEIGHT, mx);
	werase(title);
	box(title, 0, 0);
	mvwaddstr(title, 1, 2, "Feather HTTPd v" FR_VERSION);
	wrefresh(title);

	/* info */
	wbkgd(info, A_BOLD | COLOR_PAIR(1));
	mvwin(info, TITLE_HEIGHT, 0);
	wresize(info, INFO_HEIGHT, mx);
	werase(info);
	box(info, 0, 0);

	mvwaddch(info, 2, 0, ACS_LTEE);
	for(i = 0; i < mx - 2; i++) mvwaddch(info, 2, 1 + i, ACS_HLINE);
	mvwaddch(info, 2, mx - 1, ACS_RTEE);

	sprintf(infotext, "Information For Server");
	mvwaddstr(info, 1, 1 + ((mx - 2) - strlen(infotext)) / 2, infotext);

	draw_info();

	/* keys */
	wbkgd(keys, COLOR_PAIR(3));
	mvwin(keys, my - 1, 0);
	wresize(keys, 1, mx);
	werase(keys);
	mvwaddstr(keys, 0, 1, "Ctrl+C=");
	wattron(keys, COLOR_PAIR(2));
	waddstr(keys, "Exit");
	wattron(keys, A_NORMAL);
}

static void resize(void) {
	getmaxyx(stdscr, my, mx);

	init_screen();
	init_windows();

	refresh();
}

int main(int argc, char** argv) {
	double n = 0;
	int    st;

	if((st = fhttpd_init(NULL, fpr_false)) != 0) {
		return st;
	}
	log_file = fpr_fopen("/dev/null", "w");

	start = fpr_time();

	argv0 = argv[0];

	initscr();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	raw();
	;
	cbreak();
	noecho();
	curs_set(0);
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_WHITE);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);

	getmaxyx(stdscr, my, mx);

	title = subwin(stdscr, 1, 1, 0, 0);
	info  = subwin(stdscr, 1, 1, TITLE_HEIGHT, 0);
	keys  = subwin(stdscr, 1, 1, my - 1, 0);

	resize();

	thread = fpr_thread_create(thread_main, NULL);

	while(running) {
		int ch;

		while((ch = getch()) != ERR) {
			if(ch == 3) {
				running = fpr_false;
			} else if(ch == KEY_RESIZE) {
				resize();
			}
		}

		if(n >= 0.5) {
			draw_info();
			n = 0;
		}

		usleep(50000);

		n += 0.05;
	}

	fhttpd_uninit();

	endwin();
}
