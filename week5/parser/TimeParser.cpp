// Tavoitteena on 2 pistettä
// Tein kaikki 1 pisteeseen tarvittavat tehtävät ja lisäsin uusia testejä

#include <stdlib.h>
#include <string.h>
#include <bits/stdc++.h> // for isdigit()
#include "TimeParser.h"

// time format: HHMMSS (6 characters)
int time_parse(char *time) {

	// how many seconds, default returns error
	int seconds = TIME_LEN_ERROR;

	// TODO: Check that string is not null
    if (time == nullptr) {
        return TIME_NULL_ERROR;
    }

    // additional test, check that time is 6 characters long
    if (strlen(time) != 6) {
        return TIME_LEN_ERROR;
    }

	// additional test, check that all characters are digits
    for (int i = 0; i < 6; ++i) {
        if (!isdigit((unsigned char)time[i])) {
            return TIME_DIGIT_ERROR;
        }
    }

	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec
    int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours
	// Now you have:
	// values[0] hour
	// values[1] minute
	// values[2] second

	// TODO: Add boundary check time values: below zero or above limit not allowed
	// limits are 59 for minutes, 23 for hours, etc
    if (values[0] < 0 || values[0] > 23 ||
        values[1] < 0 || values[1] > 59 ||
        values[2] < 0 || values[2] > 59) {
        return TIME_VALUE_ERROR;
    } 

	// TODO: Calculate return value from the parsed minutes and seconds
	// Otherwise error will be returned!
	// seconds = ...
    seconds = values[0] * 3600 + values[1] * 60 + values[2];

    // additional test, check that seconds is not zero or below
    if (seconds <= 0) {
        return TIME_VALUE_ERROR;
    }

	return seconds;
}