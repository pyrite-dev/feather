#include <fhttpd.h>

#if defined(PPR_IS_PSP)
#include <pspdebug.h>
#elif defined(PPR_IS_PS2)
#include <debug.h>
#endif

PPR_FILE* log_file = NULL;

void log_init(void) {
	log_file = ppr_fopen(config_logfile, "a");
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
	char	      buf[LINE_SIZE * 2 + 1];
	char	      out[LINE_SIZE * 2 + 64 + 1];
	ppr_time_t    t = ppr_time();
	struct ppr_tm tm;
	const char*   day[] = {
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

	ppr_gmtime(&tm, t);

	vsprintf(buf, fmt, va);

	sprintf(out, "[%s %s %.2d %02d:%02d:%02d UTC] %s", day[tm.tm_wday], mon[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, buf);

	if(log_file != NULL) {
		nl = ppr_newline;

		ppr_fwrite(out, 1, strlen(out), log_file);
		ppr_fwrite(nl, 1, strlen(nl), log_file);

		if(!both) return;
	}

#if defined(PPR_IS_PSP)
	pspDebugScreenPrintf("%s" ppr_newline, out);
#elif defined(PPR_IS_PS2)
	scr_printf("%s" ppr_newline, out);
#else
	fprintf(stderr, "%s" ppr_newline, out);
#endif
}

void log_nofile(void) {
	if(log_file != NULL) ppr_fclose(log_file);
	log_file = NULL;
}

void log_close(void) {
	if(log_file != NULL) ppr_fclose(log_file);
	log_file = NULL;
}
