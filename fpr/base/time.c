#include <fpr.h>
#include <fpr_int.h>

void* fpr_gmtime_mutex;
void* fpr_localtime_mutex;

fpr_time_t fpr_time(void) {
	time_t t = time(NULL);

	return t;
}

static void tm_to_fpr(struct fpr_tm* to, struct tm* from) {
	to->tm_sec   = from->tm_sec;
	to->tm_min   = from->tm_min;
	to->tm_hour  = from->tm_hour;
	to->tm_mday  = from->tm_mday;
	to->tm_mon   = from->tm_mon;
	to->tm_year  = from->tm_year;
	to->tm_wday  = from->tm_wday;
	to->tm_yday  = from->tm_yday;
	to->tm_isdst = from->tm_isdst;
}

void fpr_gmtime(struct fpr_tm* tm, fpr_time_t t) {
	struct tm from_tm;
	time_t	  from = t;

	fpr_mutex_lock(fpr_gmtime_mutex);
	from_tm = *gmtime(&from);
	fpr_mutex_unlock(fpr_gmtime_mutex);

	tm_to_fpr(tm, &from_tm);
}

void fpr_localtime(struct fpr_tm* tm, fpr_time_t t) {
	struct tm from_tm;
	time_t	  from = t;

	fpr_mutex_lock(fpr_localtime_mutex);
	from_tm = *localtime(&from);
	fpr_mutex_unlock(fpr_localtime_mutex);

	tm_to_fpr(tm, &from_tm);
}
