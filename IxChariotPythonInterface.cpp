// IxChariotPythonInterface.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "C:\Program Files (x86)\Ixia\IxChariot\SDK\include\chrapi.h"
#include "ixchariot_reporting.h"


int main(int argc, char* argv[])
{
	static CHR_STRING e1Addrs = (CHR_STRING)"192.168.1.50";

	static
		CHR_STRING e2Addrs[] = {
			(CHR_STRING)"192.168.1.40",
			(CHR_STRING)"192.168.1.65",
			(CHR_STRING)"192.168.1.40",
			(CHR_STRING)"192.168.1.52",
			(CHR_STRING)"192.168.1.53"
	};

	/*
	Endpoint IxChariot_Console	= { "Console", "192.168.1.60" };
	Endpoint QTN_RDK			= { "RDK", "192.168.1.42" };
	Endpoint MacBook_Pro		= { "MBP", "192.168.1.79" };
	Endpoint Galaxy_S10			= { "GS10", "192.168.1.74" };
	Endpoint Tab_S4				= { "Tab", "192.168.1.75" };
	Endpoint iPad_Air			= { "iPad", "192.168.1.65" };
	*/

	Throughput* test = new Throughput;
	test->initialize();
	
	/*for (int client = 0; client < sizeof(e2Addrs) - 1; client++)
	{
		std::cout << "\nStarting test " << client << " -\n";
		test->start_test(e1Addrs, e2Addrs[client], 5);
		test->get_throughput();
		test->end_test();
		// TEST
		std::cout << "Test Run Stored Values: " << test->run_tSum << " " << test->run_tAvg << " " << test->run_tMin << " " << test->run_tMax;
	}*/

	if (argc == 0 || argc > 3) {	// Concurrent client test
		//test->start_test(e1Addrs, e2Addrs[0], 1);
		CHR_STRING* client_list = argv;
		/*
		for (int i = 1; i <= argc; i++)
		{
			client_list[i] = argv[i];
		}
		*/
		/*
		for (int i = 1; i < argc; i++)
		{
			std::cout << "Client " << i << ": " << client_list[i] << " ";
		}
		*/
		//std::cout << sizeof() << std::endl;

		test->multiple_client(client_list, argc);
	}
	/*if (argc == 0 || argc > 3) {
		//test
		std::cout << "Initializing..." << "\n";
		CHR_STRING client_list[4] = { (CHR_STRING)"192.168.1.50", (CHR_STRING)"192.168.1.30", (CHR_STRING)"192.168.1.31", (CHR_STRING)"192.168.1.32" };
		std::cout << "Running multiple client test." << "\n";

		test->multiple_client(client_list);
	}*/
	else
	{
		test->start_test(argv[1], argv[2], std::stoi(argv[3]));
	}

	//test->start_test(e1Addrs, e2Addrs[0], 1);
	test->get_throughput();
	test->end_test();
	// TEST
	std::cout << test->delay << " " << test->run_tSum << " " << test->run_tAvg << " " << test->run_tMin << " " << test->run_tMax <<"\n";
	std::cout << "--------------------------------------------------\n\n";
	//return test->run_tAvg;
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

