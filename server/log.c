#include <fhttpd.h>

#if defined(FPR_IS_PSP)
#include <pspdebug.h>
#elif defined(FPR_IS_PS2)
#include <debug.h>
#endif

static FPR_FILE* log_file = NULL;

void log_init(void) {
	log_file = fpr_fopen(config_logfile, "a");
}

void log_srv(const char* fmt, ...) {
	va_list va;

	va_start(va, fmt);
	log_vasrv(fmt, 0, va);
	va_end(va);
}

void log_srv2(const char* fmt, ...) {
	va_list va;

	va_start(va, fmt);
	log_vasrv(fmt, 1, va);
	va_end(va);
}

void log_vasrv(const char* fmt, int both, va_list va) {
	char	    buf[LINE_SIZE + 1];
	char	    out[LINE_SIZE + 64 + 1];
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
	const char* nl;

	vsprintf(buf, fmt, va);

	sprintf(out, "[%s %s %.2d %02d:%02d:%02d UTC] %s", day[tm->tm_wday], mon[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, buf);

	if(log_file != NULL) {
		nl = fpr_newline;

		fpr_fwrite(out, 1, strlen(out), log_file);
		fpr_fwrite(nl, 1, strlen(nl), log_file);

		if(!both) return;
	}

#if defined(FPR_IS_PSP)
	pspDebugScreenPrintf("%s" fpr_newline, out);
#elif defined(FPR_IS_PS2)
	scr_printf("%s" fpr_newline, out);
#else
	fprintf(stderr, "%s" fpr_newline, out);
#endif
}

void log_nofile(void) {
	if(log_file != NULL) fpr_fclose(log_file);
	log_file = NULL;
}

void log_close(void) {
	if(log_file != NULL) fpr_fclose(log_file);
	log_file = NULL;
}
