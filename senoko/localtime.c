/*
** This file is in the public domain, so clarified as of
** June 5, 1996 by Arthur David Olson (arthur_david_olson@nih.gov).
**
** $FreeBSD: src/lib/libc/stdtime/localtime.c,v 1.25.2.1 2001/03/05 11:37:21 obrien Exp $
*/

/*
** Leap second handling from Bradley White (bww@k.gp.cs.cmu.edu).
** POSIX-style TZ environment variable handling from Guy Harris
** (guy@auspex.com).
*/

/*LINTLIBRARY*/

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h> /* For CHAR_BIT */

#ifndef TYPE_SIGNED
#define TYPE_SIGNED(type) (((type) -1) < 0)
#endif /* !defined TYPE_SIGNED */

#ifndef TYPE_BIT
#define TYPE_BIT(type)  (sizeof (type) * CHAR_BIT)
#endif /* !defined TYPE_BIT */

#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define DAYSPERNYEAR    365
#define DAYSPERLYEAR    366
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((long) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR     12

#define TM_SUNDAY       0
#define TM_MONDAY       1
#define TM_TUESDAY      2
#define TM_WEDNESDAY    3
#define TM_THURSDAY     4
#define TM_FRIDAY       5
#define TM_SATURDAY     6

#define TM_JANUARY      0
#define TM_FEBRUARY     1
#define TM_MARCH        2
#define TM_APRIL        3
#define TM_MAY          4
#define TM_JUNE         5
#define TM_JULY         6
#define TM_AUGUST       7
#define TM_SEPTEMBER    8
#define TM_OCTOBER      9
#define TM_NOVEMBER     10
#define TM_DECEMBER     11

#define TM_YEAR_BASE    1900

#define EPOCH_YEAR      1970
#define EPOCH_WDAY      TM_THURSDAY

/*
 ** Accurate only for the past couple of centuries;
 ** that will probably do.
 */

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/*
** Prototypes for static functions.
*/

static void		gmtsub (const time_t * timep, long offset,
				struct tm * tmp);
static void		localsub (const time_t * timep, long offset,
				struct tm * tmp);
static int		increment_overflow (int * number, int delta);
static int		normalize_overflow (int * tensptr, int * unitsptr,
				int base);
static time_t		time1 (struct tm * tmp,
				void(*funcp) (const time_t *,
				long, struct tm *),
				long offset);
static time_t		time2 (struct tm *tmp,
				void(*funcp) (const time_t *,
				long, struct tm*),
				long offset, int * okayp);
static void		timesub (const time_t * timep, long offset,
				struct tm * tmp);
static int		tmcomp (const struct tm * atmp,
				const struct tm * btmp);

#ifndef TZ_STRLEN_MAX
#define TZ_STRLEN_MAX 255
#endif /* !defined TZ_STRLEN_MAX */

/*
** Section 4.12.3 of X3.159-1989 requires that
**	Except for the strftime function, these functions [asctime,
**	ctime, gmtime, localtime] return values in one of two static
**	objects: a broken-down time structure and an array of char.
** Thanks to Paul Eggert (eggert@twinsun.com) for noting this.
*/

static struct tm	tm;

static const int	mon_lengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const int	year_lengths[2] = {
	DAYSPERNYEAR, DAYSPERLYEAR
};

/*
** The easy way to behave "as if no library function calls" localtime
** is to not call it--so we drop its guts into "localsub", which can be
** freely called.  (And no, the PANS doesn't require the above behavior--
** but it *is* desirable.)
**
** The unused offset argument is for the benefit of mktime variants.
*/

/*ARGSUSED*/
static void
localsub(timep, offset, tmp)
const time_t * const	timep;
const long		offset;
struct tm * const	tmp;
{
	const time_t			t = *timep;
	(void)offset;

	/*
	** To get (wrong) behavior that's compatible with System V Release 2.0
	** you'd replace the statement below with
	**	t += ttisp->tt_gmtoff;
	**	timesub(&t, 0L, tmp);
	*/
	timesub(&t, 0, tmp);
	tmp->tm_isdst = 0;
}

struct tm *
localtime_r(timep, p_tm)
const time_t * const	timep;
struct tm *p_tm;
{
	localsub(timep, 0L, p_tm);
	return(p_tm);
}

struct tm *
localtime(timep)
const time_t * const	timep;
{
	localsub(timep, 0L, &tm);
	return &tm;
}

/*
** gmtsub is to gmtime as localsub is to localtime.
*/

static void
gmtsub(timep, offset, tmp)
const time_t * const	timep;
const long		offset;
struct tm * const	tmp;
{
	timesub(timep, offset, tmp);
}

struct tm *
gmtime(timep)
const time_t * const	timep;
{
	gmtsub(timep, 0L, &tm);
	return &tm;
}

struct tm *
gmtime_r(const time_t * timep, struct tm * tm)
{
	gmtsub(timep, 0L, tm);
	return(tm);
}

static void
timesub(timep, offset, tmp)
const time_t * const			timep;
const long				offset;
register struct tm * const		tmp;
{
	register long			days;
	register long			rem;
	register int			y;
	register int			yleap;
	register const int *		ip;
	register long			corr;
	register int			hit;

	corr = 0;
	hit = 0;
	days = *timep / SECSPERDAY;
	rem = *timep % SECSPERDAY;
	rem += (offset - corr);
	while (rem < 0) {
		rem += SECSPERDAY;
		--days;
	}
	while (rem >= SECSPERDAY) {
		rem -= SECSPERDAY;
		++days;
	}
	tmp->tm_hour = (int) (rem / SECSPERHOUR);
	rem = rem % SECSPERHOUR;
	tmp->tm_min = (int) (rem / SECSPERMIN);
	/*
	** A positive leap second requires a special
	** representation.  This uses "... ??:59:60" et seq.
	*/
	tmp->tm_sec = (int) (rem % SECSPERMIN) + hit;
	tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYSPERWEEK);
	if (tmp->tm_wday < 0)
		tmp->tm_wday += DAYSPERWEEK;
	y = EPOCH_YEAR;
#define LEAPS_THRU_END_OF(y)	((y) / 4 - (y) / 100 + (y) / 400)
	while (days < 0 || days >= (long) year_lengths[yleap = isleap(y)]) {
		register int	newy;

		newy = y + days / DAYSPERNYEAR;
		if (days < 0)
			--newy;
		days -= (newy - y) * DAYSPERNYEAR +
			LEAPS_THRU_END_OF(newy - 1) -
			LEAPS_THRU_END_OF(y - 1);
		y = newy;
	}
	tmp->tm_year = y - TM_YEAR_BASE;
	tmp->tm_yday = (int) days;
	ip = mon_lengths[yleap];
	for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
		days = days - (long) ip[tmp->tm_mon];
	tmp->tm_mday = (int) (days + 1);
	tmp->tm_isdst = 0;
#ifdef TM_GMTOFF
	tmp->TM_GMTOFF = offset;
#endif /* defined TM_GMTOFF */
}

/*
** Adapted from code provided by Robert Elz, who writes:
**	The "best" way to do mktime I think is based on an idea of Bob
**	Kridle's (so its said...) from a long time ago.
**	[kridle@xinet.com as of 1996-01-16.]
**	It does a binary search of the time_t space.  Since time_t's are
**	just 32 bits, its a max of 32 iterations (even at 64 bits it
**	would still be very reasonable).
*/

#ifndef WRONG
#define WRONG	(-1)
#endif /* !defined WRONG */

/*
** Simplified normalize logic courtesy Paul Eggert (eggert@twinsun.com).
*/

static int
increment_overflow(number, delta)
int *	number;
int	delta;
{
	int	number0;

	number0 = *number;
	*number += delta;
	return (*number < number0) != (delta < 0);
}

static int
normalize_overflow(tensptr, unitsptr, base)
int * const	tensptr;
int * const	unitsptr;
const int	base;
{
	register int	tensdelta;

	tensdelta = (*unitsptr >= 0) ?
		(*unitsptr / base) :
		(-1 - (-1 - *unitsptr) / base);
	*unitsptr -= tensdelta * base;
	return increment_overflow(tensptr, tensdelta);
}

static int
tmcomp(atmp, btmp)
register const struct tm * const atmp;
register const struct tm * const btmp;
{
	register int	result;

	if ((result = (atmp->tm_year - btmp->tm_year)) == 0 &&
		(result = (atmp->tm_mon - btmp->tm_mon)) == 0 &&
		(result = (atmp->tm_mday - btmp->tm_mday)) == 0 &&
		(result = (atmp->tm_hour - btmp->tm_hour)) == 0 &&
		(result = (atmp->tm_min - btmp->tm_min)) == 0)
			result = atmp->tm_sec - btmp->tm_sec;
	return result;
}

static time_t
time2(tmp, funcp, offset, okayp)
struct tm * const	tmp;
void (* const		funcp) (const time_t*, long, struct tm*);
const long		offset;
int * const		okayp;
{
	register int			dir;
	register int			bits;
	register int			i;
	register int			saved_seconds;
	time_t				newt;
	time_t				t;
	struct tm			yourtm, mytm;

	*okayp = false;
	yourtm = *tmp;
	if (normalize_overflow(&yourtm.tm_hour, &yourtm.tm_min, MINSPERHOUR))
		return WRONG;
	if (normalize_overflow(&yourtm.tm_mday, &yourtm.tm_hour, HOURSPERDAY))
		return WRONG;
	if (normalize_overflow(&yourtm.tm_year, &yourtm.tm_mon, MONSPERYEAR))
		return WRONG;
	/*
	** Turn yourtm.tm_year into an actual year number for now.
	** It is converted back to an offset from TM_YEAR_BASE later.
	*/
	if (increment_overflow(&yourtm.tm_year, TM_YEAR_BASE))
		return WRONG;
	while (yourtm.tm_mday <= 0) {
		if (increment_overflow(&yourtm.tm_year, -1))
			return WRONG;
		i = yourtm.tm_year + (1 < yourtm.tm_mon);
		yourtm.tm_mday += year_lengths[isleap(i)];
	}
	while (yourtm.tm_mday > DAYSPERLYEAR) {
		i = yourtm.tm_year + (1 < yourtm.tm_mon);
		yourtm.tm_mday -= year_lengths[isleap(i)];
		if (increment_overflow(&yourtm.tm_year, 1))
			return WRONG;
	}
	for ( ; ; ) {
		i = mon_lengths[isleap(yourtm.tm_year)][yourtm.tm_mon];
		if (yourtm.tm_mday <= i)
			break;
		yourtm.tm_mday -= i;
		if (++yourtm.tm_mon >= MONSPERYEAR) {
			yourtm.tm_mon = 0;
			if (increment_overflow(&yourtm.tm_year, 1))
				return WRONG;
		}
	}
	if (increment_overflow(&yourtm.tm_year, -TM_YEAR_BASE))
		return WRONG;
	if (yourtm.tm_year + TM_YEAR_BASE < EPOCH_YEAR) {
		/*
		** We can't set tm_sec to 0, because that might push the
		** time below the minimum representable time.
		** Set tm_sec to 59 instead.
		** This assumes that the minimum representable time is
		** not in the same minute that a leap second was deleted from,
		** which is a safer assumption than using 58 would be.
		*/
		if (increment_overflow(&yourtm.tm_sec, 1 - SECSPERMIN))
			return WRONG;
		saved_seconds = yourtm.tm_sec;
		yourtm.tm_sec = SECSPERMIN - 1;
	} else {
		saved_seconds = yourtm.tm_sec;
		yourtm.tm_sec = 0;
	}
	/*
	** Divide the search space in half
	** (this works whether time_t is signed or unsigned).
	*/
	bits = TYPE_BIT(time_t) - 1;
	/*
	** If time_t is signed, then 0 is just above the median,
	** assuming two's complement arithmetic.
	** If time_t is unsigned, then (1 << bits) is just above the median.
	*/
	t = TYPE_SIGNED(time_t) ? 0 : (((time_t) 1) << bits);
	for ( ; ; ) {
		(*funcp)(&t, offset, &mytm);
		dir = tmcomp(&mytm, &yourtm);
		if (dir != 0) {
			if (bits-- < 0)
				return WRONG;
			if (bits < 0)
				--t; /* may be needed if new t is minimal */
			else if (dir > 0)
				t -= ((time_t) 1) << bits;
			else	t += ((time_t) 1) << bits;
			continue;
		}
		if (yourtm.tm_isdst < 0 || mytm.tm_isdst == yourtm.tm_isdst)
			break;
		return WRONG;
	}

	newt = t + saved_seconds;
	if ((newt < t) != (saved_seconds < 0))
		return WRONG;
	t = newt;
	(*funcp)(&t, offset, tmp);
	*okayp = true;
	return t;
}

static time_t
time1(tmp, funcp, offset)
struct tm * const	tmp;
void (* const		funcp) (const time_t *, long, struct tm *);
const long		offset;
{
	register time_t			t;
	int				okay;

	if (tmp->tm_isdst > 1)
		tmp->tm_isdst = 1;
	t = time2(tmp, funcp, offset, &okay);
#ifdef PCTS
	/*
	** PCTS code courtesy Grant Sullivan (grant@osf.org).
	*/
	if (okay)
		return t;
	if (tmp->tm_isdst < 0)
		tmp->tm_isdst = 0;	/* reset to std and try again */
#endif /* defined PCTS */
#ifndef PCTS
	if (okay || tmp->tm_isdst < 0)
		return t;
#endif /* !defined PCTS */
	return WRONG;
}

time_t
mktime(tmp)
struct tm * const	tmp;
{
	time_t mktime_return_value;
	mktime_return_value = time1(tmp, localsub, 0L);
	return(mktime_return_value);
}
