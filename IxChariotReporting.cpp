/* *** IxChariot Throughput Test Functions *** 
 *
 * Test setup and reporting as call-able functions
 * Intended for python integration
 *
 * Author: Nathan Hu
 */

#include "ixchariot_reporting.h"		/* Header File */

#define NUM_PINGS 5


/* 
 *	Initialize the IxChariot API
 */
void Throughput::initialize() {
	/* Setting console output to show up to [decimal] decimal places */
	std::cout << std::fixed;
	std::cout << std::setprecision(3);

	/* 1. Initialize the Chariot API */
	std::cout << "Initializing Chariot API...";
    rc = CHR_api_initialize(CHR_DETAIL_LEVEL_ALL, errorInfo,
                            CHR_MAX_ERROR_INFO, &errorLen);
    if (rc != CHR_OK) {
        /*
         * Because initialization failed, we can't
         * ask the API for the message for this
         * return code so we can't call our
         * show_error() function. Instead, we'll
         * print what we do know before exiting.
         */
        printf("Initialization failed: rc = %d\n", rc);
        printf("Extended error info:\n%s\n", errorInfo);
        exit(EXIT_FAILURE);
    }
	std::cout << "DONE.\n\n";
}


/*
 *	Function to end the test and force a delay before the next run
 */
void Throughput::end_test() {
	std::cout << "TEST COMPLETED.\n";
	std::cout << "Closing test instance...\n\n";
	rc = CHR_test_force_delete(test);
	// Sleep(10 * 1000);
}


void Throughput::start_test( CHR_STRING e1Addr, CHR_STRING e2Addr, int streams ) {
	/* 2. Create/open new test for every test permutation */
	std::cout << "--------------------------------------------------\n";
	std::cout << "Starting the test instance:\n";
	//printf("\n");
	rc = CHR_test_new(&test);
	if (rc != CHR_OK) {
	   show_error((CHR_HANDLE)NULL, rc, (CHR_STRING)"test_new");
	}

	/* Initialize the runopts handle for this test instance and set the run time */
	rc = CHR_runopts_set_test_duration(runopts, test_run_duration);
	//rc = CHR_runopts_set_reporting_type(runopts, CHR_TEST_REPORTING_REALTIME);	 // ***  CHR_TEST_REPORTING_BATCH or CHR_TEST_REPORTING_REALTIME ***
	rc = CHR_test_get_runopts(test, &runopts);		
	
	/* Set test to end after a fixed duration */
	rc = CHR_runopts_set_test_end(runopts, CHR_TEST_END_AFTER_FIXED_DURATION);
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"set_test_duration");
	} /* End of test creation */

	/* Set the test filename test */
	rc = CHR_test_set_filename(test, test_name, strlen(test_name));
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"test_set_filename");
	}
	
	// TESTING DELAY
	get_tracert(e1Addr, e2Addr);


	/* 3. Set the pair attributes from config file */
	std::cout << "Creating " << streams << " pairs and setting pair attributes...\n\n";
	for ( int stPair = 0; stPair < streams; stPair++ ) {
		char comment[CHR_MAX_PAIR_COMMENT] = "\0";

		/* Create a pair */
		std::cout << "Pair " << stPair+1 << ": " << e1Addr << " to " << e2Addr << "\n";
		rc = CHR_pair_new(&pair);
		if (rc != CHR_OK) {
			show_error((CHR_HANDLE)NULL, rc, (CHR_STRING)"pair_new");
		}

		/* Set the pair attributes from our lists */
		memcpy(comment, "Pair %d", stPair + 1);
		rc = CHR_pair_set_comment(pair,
			comment,
			strlen(comment));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_comment");
		}
		/* --- Set Endpoint 1 --- */
		rc = CHR_pair_set_e1_addr(pair,
			e1Addr,
			strlen(e1Addr));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_e1_addr");
		}
		/* --- Set Endpoint 2 --- */
		rc = CHR_pair_set_e2_addr(pair,
			e2Addr,
			strlen(e2Addr));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_e2_addr");
		}
		rc = CHR_pair_set_protocol(pair, CHR_PROTOCOL_TCP);
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_protocol");
		}
		/* --- Use specified throughput script --- */
		rc = CHR_pair_use_script_filename(pair,
			script,
			strlen(script));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_use_script_filename");
		}

		/* Add the pair to the test */
		rc = CHR_test_add_pair(test, pair);
		if (rc != CHR_OK) {
			show_error(test, rc, (CHR_STRING)"test_add_pair");
		}
	} 
	/* End of stream pairing */	
	std::cout << "DONE.\n\n";

	/* 4. RUN THE TEST */
	std::cout << "Running test with a duration of [" << test_run_duration << "] seconds.\n";
	std::cout << "Please wait for the test to finish...\n\n";
	rc = CHR_test_start(test);
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"start_test");
	}

	/* Wait for the test to stop */
	isStopped = CHR_FALSE;
	timer = 0;
	while (!isStopped && timer < maxWait) {
		rc = CHR_test_query_stop(test, timeout);
		//this->get_real_time();
		//Sleep(this->real_time_interval * 1000);

		if (rc == CHR_OK) {
			isStopped = CHR_TRUE;
		}
		else if (rc == CHR_TIMED_OUT) {
			timer += timeout;
			//printf("Waiting for test to stop... (%d)\n", timeout);
		} 
		else {
			show_error(test, rc, (CHR_STRING)"test_query_stop");
		}
	}
	if (!isStopped) {
		show_error(test, CHR_TIMED_OUT, (CHR_STRING)"test_query_stop");
	} /* End of a single test permutation */
	/* Close test instance */
	//std::cout << "TEST COMPLETED!\n\n";
	//std::cout << "Closing test instance...\n\n";
	//rc = CHR_test_force_delete(test);
	//Sleep(10 * 1000);
}


void Throughput::multiple_client(CHR_STRING* clientList, int numClients) {
	/* 2. Create/open new test for every test permutation */
	std::cout << "--------------------------------------------------\n";
	std::cout << "Starting the concurrent clients test instance:\n";
	//printf("\n");
	rc = CHR_test_new(&test);
	if (rc != CHR_OK) {
		show_error((CHR_HANDLE)NULL, rc, (CHR_STRING)"test_new");
	}

	/* Initialize the runopts handle for this test instance and set the run time */
	rc = CHR_runopts_set_test_duration(runopts, test_run_duration);
	//rc = CHR_runopts_set_reporting_type(runopts, CHR_TEST_REPORTING_REALTIME);	 // ***  CHR_TEST_REPORTING_BATCH or CHR_TEST_REPORTING_REALTIME ***
	rc = CHR_test_get_runopts(test, &runopts);

	/* Set test to end after a fixed duration */
	rc = CHR_runopts_set_test_end(runopts, CHR_TEST_END_AFTER_FIXED_DURATION);
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"set_test_duration");
	} /* End of test creation */

	/* Set the test filename test */
	rc = CHR_test_set_filename(test, test_name, strlen(test_name));
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"test_set_filename");
	}


	/* 3. Set the pair attributes from config file */
	std::cout << "Creating pairs and setting pair attributes...\n\n";
	for (int stPair = 1; stPair < numClients - 1; stPair++) {
		char comment[CHR_MAX_PAIR_COMMENT] = "\0";

		/* Create a pair */
		std::cout << "Pair " << stPair << ": " << clientList[1] << " to " << clientList[stPair + 1] << "\n";
		rc = CHR_pair_new(&pair);
		if (rc != CHR_OK) {
			show_error((CHR_HANDLE)NULL, rc, (CHR_STRING)"pair_new");
		}

		/* Set the pair attributes from our lists */
		memcpy(comment, "Pair %d", stPair + 1);
		rc = CHR_pair_set_comment(pair,
			comment,
			strlen(comment));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_comment");
		}
		/* --- Set Endpoint 1 --- */
		rc = CHR_pair_set_e1_addr(pair,
			clientList[1],
			strlen(clientList[1]));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_e1_addr");
		}
		/* --- Set Endpoint 2 --- */
		rc = CHR_pair_set_e2_addr(pair,
			clientList[stPair+1],
			strlen(clientList[stPair+1]));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_e2_addr");
		}
		rc = CHR_pair_set_protocol(pair, CHR_PROTOCOL_TCP);
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_set_protocol");
		}
		/* --- Use specified throughput script --- */
		rc = CHR_pair_use_script_filename(pair,
			script,
			strlen(script));
		if (rc != CHR_OK) {
			show_error(pair, rc, (CHR_STRING)"pair_use_script_filename");
		}

		/* Add the pair to the test */
		rc = CHR_test_add_pair(test, pair);
		if (rc != CHR_OK) {
			show_error(test, rc, (CHR_STRING)"test_add_pair");
		}
	}
	/* End of stream pairing */
	std::cout << "DONE.\n\n";

	/* 4. RUN THE TEST */
	std::cout << "Running test with a duration of [" << test_run_duration << "] seconds.\n";
	std::cout << "Please wait for the test to finish...\n\n";
	rc = CHR_test_start(test);
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"start_test");
	}

	/* Wait for the test to stop */
	isStopped = CHR_FALSE;
	timer = 0;
	while (!isStopped && timer < maxWait) {
		rc = CHR_test_query_stop(test, timeout);
		//this->get_real_time();
		//Sleep(this->real_time_interval * 1000);

		if (rc == CHR_OK) {
			isStopped = CHR_TRUE;
		}
		else if (rc == CHR_TIMED_OUT) {
			timer += timeout;
			//printf("Waiting for test to stop... (%d)\n", timeout);
		}
		else {
			show_error(test, rc, (CHR_STRING)"test_query_stop");
		}
	}
	if (!isStopped) {
		show_error(test, CHR_TIMED_OUT, (CHR_STRING)"test_query_stop");
	} /* End of a single test permutation */
	/* Close test instance */
	//std::cout << "TEST COMPLETED!\n\n";
	//std::cout << "Closing test instance...\n\n";
	//rc = CHR_test_force_delete(test);
	//Sleep(10 * 1000);
}


/*
 *	Retrieve the throughput real-time every specified interval
 */
void Throughput::get_real_time() {
	int timingRecIter = 0;
	CHR_COUNT timingRecCount;
	CHR_COUNT lastPrinted, lastIndex;

	/* Show timing records if selected */
	rc = CHR_pair_get_timing_record_count(pair,
		&timingRecCount);
	if (rc != CHR_OK) {
		show_error(pair, rc,
			CHR_STRING("pair_get_timing_record_count"));
	}
	printf("  Number of timing records = %d\n",
		timingRecCount);
	if (timingRecIter != 0 && timingRecCount != 0) {

		printf("\n");
		for (index = 0;
			index < timingRecCount;
			index += timingRecIter) {

			printf("Timing record %d:\n", index + 1);
			rc = CHR_pair_get_timing_record(pair,
				index, &timingRec);
			if (rc != CHR_OK) {
				show_error(pair, rc,
					CHR_STRING("pair_get_timing_record"));
			}
			show_timing_rec();
		}
		lastPrinted = index - timingRecIter;
		lastIndex = timingRecCount - 1;
		if (lastPrinted != lastIndex) {

			/* Always show last timing record */
			index = timingRecCount - 1;
			printf("Timing record %d:\n", index + 1);
			rc = CHR_pair_get_timing_record(pair,
				index, &timingRec);
			if (rc != CHR_OK) {
				show_error(pair, rc,
					CHR_STRING("pair_get_timing_record"));
			}
			show_timing_rec();
		}
	}
}


/*
 *	Retrieve the pair throughput data from the IxChariot API
 */
void Throughput::get_throughput() {
	/* Calculation Variables */
	int convert = pow(10, this->decimal);
	int tCount = 0;
	double tSum = 0, tAvg = 0;
	double tMin = 0, tMax = 0;
	CHR_FLOAT pAvg;
	CHR_FLOAT pMin, pMax;

	/* (Re)set the global test result variables to 0 */
	this->run_tSum = 0.0, this->run_tAvg = 0.0, this->run_tMin = 0.0, this->run_tMax = 0.0;

	rc = CHR_test_get_pair_count(test, &index);
	if (rc != CHR_OK) {
		show_error(test, rc, (CHR_STRING)"test_get_pair_count");
	}

	std::cout << "--- TEST RUN RESULTS ---\n";
	for (unsigned long prIndex = 0; prIndex < index; prIndex++) {
		std::cout << "Pair " << prIndex + 1 << ")\n";
		rc = CHR_test_get_pair(test, (CHR_COUNT)prIndex, &pair);
		if (rc != CHR_OK) {
			show_error(test, rc, (CHR_STRING)"test_get_pair");
		}

		rc = CHR_pair_results_get_average(pair,
			CHR_RESULTS_THROUGHPUT, &pAvg);
		if (rc != CHR_OK) {

			if (rc == CHR_NO_SUCH_VALUE) {
				printf("  Throughput: n/a ");
			}
			else {
				show_results_error(rc, (CHR_STRING)"throughput avg");
				//return;
			}
		}
		else {
			tCount++;
			tSum += pAvg;	/* Add the pair average to the total sum variable */

			rc = CHR_pair_results_get_minimum(pair,
				CHR_RESULTS_THROUGHPUT, &pMin);
			if (rc != CHR_OK) {
				show_results_error(rc, (CHR_STRING)"throughput min");
				return;
			}
			rc = CHR_pair_results_get_maximum(pair,
				CHR_RESULTS_THROUGHPUT, &pMax);
			if (rc != CHR_OK) {
				show_results_error(rc, (CHR_STRING)"throughput max");
				return;
			}

			/* Determine the final max and min */
			if ((tMin == 0) || (pMin < tMin)) {
				tMin = pMin;
			}

			if (tMax < pMax) {
				tMax = pMax;
			}

			printf("  Throughput: avg %.3f   min %.3f   max %.3f\n",
				pAvg, pMin, pMax);
		}		
	}

	/* Final Calculations */
	if (tCount == 0) { tCount = 1; }
	tAvg = tSum / tCount;

	/* Console Result Reporting */
	std::cout << "\n";
	std::cout << "THROUGHPUT TOTAL)    " << tSum << " Mbps\n";
	std::cout << "     PAIR(S) AVG)    " << tAvg << "\n";
	std::cout << "     PAIR(S) MIN)    " << tMin << "\n";
	std::cout << "     PAIR(S) MAX)    " << tMax << "\n";
	std::cout << "--------------------------------------------------\n\n";

	/* Timing record */
	/*if (CHR_OK == rc) { 
		printf("\nTest results:\n------------\n"); 
		rc = CHR_pair_get_timing_record_count(pair, &timingRecCount); 
		if (rc != CHR_OK) { show_error(pair, rc, "pair_get_timing_record_count");
		} 
		printf("Number of timing records = %d\n", timingRecCount); 
	}*/
	
	/* Round Results to [decimal] decimal places */
	tSum = (int)(tSum * convert + .5);
	tAvg = (int)(tAvg * convert + .5);
	tMin = (int)(tMin * convert + .5);
	tMax = (int)(tMax * convert + .5);
	/* Store run results in the global class variables */
	this->run_tSum = (double)tSum / convert;
	this->run_tAvg = (double)tAvg / convert;
	this->run_tMin = (double)tMin / convert;
	this->run_tMax = (double)tMax / convert;
}


void Throughput::get_tracert(CHR_STRING e1Addr, CHR_STRING e2Addr)
{
	CHR_COUNT hops;
	int sum_lat = 0;

	rc = CHR_tracert_pair_new(&tracert);

	rc = CHR_tracert_pair_set_e1_addr(tracert, e1Addr, strlen(e1Addr));
	rc = CHR_tracert_pair_set_e2_addr(tracert, e2Addr, strlen(e2Addr));
	rc = CHR_tracert_pair_run(tracert);
	rc = CHR_tracert_pair_query_stop(tracert, 1);
	rc = CHR_tracert_pair_get_runStatus(tracert, &status);
	rc = CHR_tracert_pair_results_get_hop_count(tracert, &hops);
	std::cout << "\nHops: " << hops << "\n";
	rc = CHR_tracert_pair_get_hop_record(tracert, 0, &hoprec);
	rc = CHR_hoprec_get_hop_latency(hoprec, &delay);
	std::cout << "Latency: " << delay << " ms\n\n";

}


/*	Ixia Functions	*/
/***************************************************************
 *
 * Print information about an error and exit. If there is
 * extended error information, log that before exiting.
 *
 * Parameters: handle - what object had the error
 *             code   - the Chariot API return code
 *             where  - what function call failed
 ***************************************************************/
void Throughput::show_error(
	CHR_HANDLE handle,
	CHR_API_RC code,
	CHR_STRING where)
{
	char       msg[CHR_MAX_RETURN_MSG];
	CHR_LENGTH msgLen;

	char       errorInfo[CHR_MAX_ERROR_INFO];
	CHR_LENGTH errorLen;

	CHR_API_RC rc;


	/*
	 * Get the API message for this return code.
	 */
	rc = CHR_api_get_return_msg(code, msg,
		CHR_MAX_RETURN_MSG, &msgLen);
	if (rc != CHR_OK) {

		/* Could not get the message: show why */
		printf("%s failed\n", where);
		printf(
			"Unable to get message for return code %d, rc = %d\n",
			code, rc);
	}
	else {

		/* Tell the user about the error */
		printf("%s failed: rc = %d (%s)\n", where, code, msg);
	}

	/*
	 * See if there is extended error information available.
	 * It's meaningful only after some error returns. After
	 * failed "new" function calls, we don't have a handle so
	 * we cannot check for extended error information.
	 */
	if ((code == CHR_OPERATION_FAILED ||
		code == CHR_OBJECT_INVALID ||
		code == CHR_APP_GROUP_INVALID) &&
		handle != (CHR_HANDLE)NULL) {

		/* Get the extended error info */
		rc = CHR_common_error_get_info(handle,
			CHR_DETAIL_LEVEL_ALL,
			errorInfo,
			CHR_MAX_ERROR_INFO,
			&errorLen);
		if (rc == CHR_OK) {

			/*
			 * We can ignore all non-success return codes here
			 * because most should not occur (the api's been
			 * initialized, the handle is good, the buffer
			 * pointer is valid, and the detail level is okay),
			 * and the NO_SUCH_VALUE return code here means
			 * there is no info available.
			 */
			printf("%s\n", errorInfo);
		}
	}

	/*
	 * We've told everything we know, so now just exit.
	 */
	exit(EXIT_FAILURE);
}


/***************************************************************
 *
 * Print information about an error that occurred while
 * getting results information. There is no extended error
 * information in these cases and we do not want to exit
 * this program as these are not fatal errors.
 *
 * Parameters: code   - the Chariot API return code
 *             what  - what function call failed
 ***************************************************************/
void Throughput::show_results_error(
	CHR_API_RC rc,
	CHR_STRING what)
{
	char msg[CHR_MAX_RETURN_MSG];

	CHR_LENGTH len;

	CHR_API_RC msgRc;


	printf("Get %s failed: ", what);
	msgRc = CHR_api_get_return_msg(rc, msg,
		CHR_MAX_RETURN_MSG,
		&len);
	if (msgRc == CHR_OK) {
		printf("%s\n", msg);
	}
	else {
		printf("rc = %d\n", rc);
	}
}


/**************************************************************
 *
 * Local function to print results common to timing records.
 * Show "n/a" if NO_SUCH_VALUE is returned.
 *
 * Parameters: handle to object for which to get results
 *
 * Note: If the attempt to get any results fails, a message
 *       is printed showing the reason and we return to rather
 *       than force exit of the program, since an error when
 *       getting results is not necessarily a fatal condition.
 **************************************************************/
void Throughput::show_timing_rec()
{
	CHR_FLOAT  result;
	//CHR_API_RC rc;
	CHR_FLOAT  float_result;
	//CHR_COUNT  count_result;

	/* These should be available in all timing records */
	rc = CHR_timingrec_get_elapsed(timingRec, &result);
	if (rc != CHR_OK) {
		show_results_error(rc, CHR_STRING("elapsed"));
		return;
	}
	printf("  Elapsed time      : %.3f\n", result);

	rc = CHR_timingrec_get_inactive(timingRec, &result);
	if (rc != CHR_OK) {
		show_results_error(rc, CHR_STRING("inactive"));
		return;
	}
	printf("  Inactive time     : %.3f\n", result);

	/*
	 * These do not have NO_SUCH_VALUE as a possible return,
	 * so anything other than success is an error
	 */
	rc = CHR_common_results_get_meas_time(timingRec, &float_result);
	if (rc != CHR_OK) {
		show_results_error(rc, CHR_STRING("meas_time"));
		return;
	}
	printf("  Measured time           : %.3f\n", float_result);

	rc = CHR_common_results_get_trans_count(timingRec, &float_result);
	if (rc != CHR_OK) {
		show_results_error(rc, CHR_STRING("trans_count"));
		return;
	}
	printf("  Transaction count       : %.0f\n", float_result);

	/* Show common results */
	//show_common(this->timingRec);
}