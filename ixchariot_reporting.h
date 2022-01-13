/* *** IxChariot Throughput Test Functions Header *** 
 *
 * Header File for IxChariotReporting.cpp
 * Intended for Python integration
 *
 * Author: Nathan Hu
 */
#ifndef OTA_H
#define OTA_H


#include "C:\Program Files (x86)\Ixia\IxChariot\SDK\include\chrapi.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdlib.h>
#include <chrono>
#include <string.h>
//#include <cstring>
#include <windows.h>


/*
 *	CLIENT INFORMATION STRUCT
 */
struct Endpoint {
	char label[10];		/* Client abbreviation */
	CHR_STRING ip;
};
typedef struct Endpoint Endpoint;

/* 
 *	THROUGHPUT TEST CLASS 
 */
class Throughput {
public:
	/* *** IxChariot API Variables *** */
	/* Test Timing */
	CHR_COUNT test_run_duration = 60;		/* Default 60 sec */
	CHR_COUNT maxWait = 600;				/* In seconds */
	CHR_COUNT timeout = 5;					/* Periodic 5 second check */
	CHR_COUNT fix_run_time = 1;	    		/* 1 - Run test at fixed duration, 0 - full run */
	CHR_COUNT between_pair_delay = 10;
	CHR_COUNT iteration_delay = 10; 		/* Delay between iterations (sec) */
	CHR_COUNT real_time_interval = 1;
	CHR_STRING test_name = (char*)"test";
	CHR_STRING script = (char*)"C:/Program Files (x86)/Ixia/IxChariot/Scripts/High_Performance_Throughput.scr";
	
	/* Test Variables */
	CHR_TEST_HANDLE	test;
	CHR_PAIR_HANDLE	pair;
	CHR_REPORT_HANDLE report;
	CHR_TRACERT_PAIR_HANDLE tracert;
	CHR_HOPREC_HANDLE hoprec;
	CHR_TRACERT_RUNSTATUS_TYPE status;
	//CHR_FLOAT joinLatency, leaveLatency;
	CHR_COUNT delay;
	/* Timing Variables */
	CHR_TIMINGREC_HANDLE timingRec;
	//CHR_COUNT timingRecCount;

	CHR_RUNOPTS_HANDLE	runopts;
	char		errorInfo[CHR_MAX_ERROR_INFO];
	CHR_LENGTH	errorLen;
	CHR_COUNT index;
	CHR_BOOLEAN isStopped;
	CHR_COUNT   timer = 0;
	CHR_API_RC rc;

	/* *** Reporting Variables *** */
	int decimal = 3;		/* Set the number of decimal places for the results. Default 3. */
	double	run_tSum = 0.0,
			run_tAvg = 0.0, 
			run_tMin = 0.0, 
			run_tMax = 0.0;	/* Temporary variables to store run results */
	
	void initialize();
	void start_test(CHR_STRING e1Addr, CHR_STRING e2Addr, int streams);
	void get_real_time();
	void get_throughput();
	void multiple_client(CHR_STRING* clientList, int numClients);
	//void get_latency();
	void get_tracert(CHR_STRING e1Addr, CHR_STRING e2Addr);
	void end_test();
	void show_error(CHR_HANDLE handle,
		CHR_API_RC code,
		CHR_STRING where);
	static void show_results_error(
		CHR_API_RC rc,
		CHR_STRING what);
	void show_timing_rec();

private:

};

#endif /* OTA_H */