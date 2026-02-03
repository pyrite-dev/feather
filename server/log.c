#include <fhttpd.h>

static FPR_FILE* log_file = NULL;

void log_init(void) {
	log_file = fpr_fopen(config_logfile, "a");
}

void log_srv(const char* fmt, ...) {
	char	    buf[LINE_SIZE + 1];
	char	    out[LINE_SIZE + 64 + 1];
	va_list	    va;
	time_t	    t	  = time(NULL);
	struct tm*  tm	  = gmtime(&t);
	const char* day[] = {
	    "Sun",
	    "Mon",
	    "Tue",
	    "Wed",
	    "Thu",
	    "Fri",
	    "Sat"};
	const char* mon[] = {
	    "Jan",
	    "Feb",
	    "Mar",
	    "Apr",
	    "May",
	    "Jun",
	    "Jul",
	    "Aug",
	    "Sep",
	    "Oct",
	    "Nov",
	    "Dec"};

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	sprintf(out, "[%s %s %.2d %02d:%02d:%02d UTC] %s", day[tm->tm_wday], mon[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, buf);

	if(log_file == NULL) {
		fprintf(stderr, "%s" fpr_newline, out);
	} else {
		const char* nl = fpr_newline;

		fpr_fwrite(out, 1, strlen(out), log_file);
		fpr_fwrite(nl, 1, strlen(nl), log_file);
	}
}

void log_nofile(void) {
	if(log_file != NULL) fpr_fclose(log_file);
	log_file = NULL;
}
