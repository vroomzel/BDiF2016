//
// Created by alex on 2/28/16.
//

#include "LOG.h"
#include "Parameters.h"


void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message) {
    // No output?
    if ((t==CALLED && !params.output_called) ||
        (t==DIAGNOSTIC && !params.output_diagnostic) ||
        (t==EVOLVE && !params.output_evolve) ||
        (t==EXIT && !params.output_exit) ||
        (t==INFO && !params.output_info) ||
        // NAME is always output.
        (t==RESULT && !params.output_result)) return;

    static int line_count = 0;

    stringstream output;
    output << setfill('0') << setw(8) << ++line_count << " ";
    output << Timestamp() << " ";
    output << ElapsedTime() << " ";
    output << pLogTypeEnum[t] << " ";
    output << pModuleNameEnum[m] << " ";

    switch (t)
    {
        case CALLED:
        case DIAGNOSTIC:
        case EVOLVE:
        case EXIT:
        case INFO:
        case NAME:
        case RESULT:
            output << message;
            break;
        default:
            output << "Error in LOG() function!";
            break;
    }

    cout << output.str() << endl << flush;

    if (t==EXIT) {
        exit(1);
    }
}

void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message1, string message2) {
    stringstream msg;
    msg << message1 << message2;
    LOG(params, t, m, msg.str());
}

void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message1, string message2, string message3) {
    stringstream msg;
    msg << message1 << message2 << message3;
    LOG(params, t, m, msg.str());
}

void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message1, string message2, string message3,string message4) {
	stringstream msg;
	msg << message1 << message2 << message3 << message4;
	LOG(params, t, m, msg.str());
}
string ElapsedTime()
{
	static int once = 0;

	// Initialize the timer on the first call.
	if (!once) {
		start = clock();
		once = 1;
	}

	long hours = 0;
	long minutes = 0;
	long seconds = 0;
	long hundredths = 0; // Hundredths of a second.
	double secs;	// Difference, in seconds and factions thereof, between two clock samples.
	long   lsecs;	// Seconds as a long.
	ldiv_t result;

	clock_t end = clock();
	secs = (double)(end - start)/CLOCKS_PER_SEC;
	lsecs = floor(secs);
	if (lsecs>3600) { // Hours.
		result = ldiv(lsecs,3600L);
		hours  = result.quot;
		lsecs  = result.rem;
	}
	if (lsecs>60) { // Minutes.
		result  = ldiv(secs,60L);
		minutes = result.quot;
		lsecs   = result.rem;
	}
	if (lsecs>=0) { // Seconds.
		seconds = lsecs;
	}

	// Fraction of a second left over.
	secs -= hours * 3600L + minutes * 60L + seconds;
	if (secs>=0.0) hundredths = floor(100.0 * secs);

	// Construct an elapsed timestamp in "hh:mm:ss" format
	stringstream ss;
	ss << setfill('0') << setw(2) << hours;		// Hours.
	ss << ":";
	ss << setfill('0') << setw(2) << minutes;	// Minutes.
	ss << ":";
	ss << setfill('0') << setw(2) << seconds;	// Seconds.
	ss << ".";
	ss << setfill('0') << setw(2) << hundredths;	// Seconds.

	return ss.str();
}

string Timestamp()
{
	time_t now = time(0);
	tm *local = localtime(&now);

	// Construct a timestamp in "YYYYMMDD:hh:mm:ss" format
	stringstream ss;
	ss << setfill('0') << setw(4) << 1900 + local->tm_year;	// Year.
	ss << setfill('0') << setw(2) << 1    + local->tm_mon;	// Month.
	ss << setfill('0') << setw(2) <<        local->tm_mday;	// Day.
	ss << ":";
	ss << setfill('0') << setw(2) <<        local->tm_hour;	// Hours.
	ss << ":";
	ss << setfill('0') << setw(2) <<        local->tm_min;	// Minutes.
	ss << ":";
	ss << setfill('0') << setw(2) <<        local->tm_sec;	// Seconds.

	return ss.str();
}
