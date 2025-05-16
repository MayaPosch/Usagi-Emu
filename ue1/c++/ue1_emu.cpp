/*
	ue1_emu.cpp - Implementation of the Usagi Electric UE1 valve-based computer.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2025/05/15 - Maya Posch - Original version.
*/


#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <thread>
#include <cstdarg> // For printxy().

#include "sarge.h"

#include "processor.h"


void printxy(int x, int y, const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("\033[%d;%dH", x, y);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}


int main(int argc, char* argv[]) {
	// Check for input arguments, std::cout << out usage if not found.
	Sarge sarge;
	
	sarge.setArgument("h", "help", "std::cout << this message.", false);
	sarge.setArgument("f", "file", "Input file with UE1 ASM.", true);
	sarge.setArgument("c", "clock", "Clock speed of the emulated UE1 in Hz.", true);
	sarge.setDescription("Usagi Electric 1 (UE1) emulator in C++. Written by Maya Posch.");
	sarge.setUsage("ue1_emu <options>");
	
	if (!sarge.parseArguments(argc, argv)) {
		std::cerr << "Couldn't parse arguments. Run with '-h' to see options." << std::endl;
		return 1;
	}
	
	if (sarge.exists("help")) {
		sarge.printHelp();
		return 0;
	}
	
	std::string filepath;
	if (!sarge.getFlag("file", filepath)) {
		// Request filename from the user.
		std::cout << "Please enter the full file path:\n> ";
		std::cin >> filepath;
	}
	
	std::string t;
	if (!sarge.getFlag("clock", t)) {
		std::cout << "Using default 10 Hz clock speed." << std::endl;
	}
	else {
		speed = std::stof(t); // Get provided clock as FP32 value.
		
		// Validate clock speed.
		if (speed < 1) { 
			// Invalid clock speed provided. Setting default 10 Hz.
			speed = 10; 
		}
	}
	
	// Divide 1 by the clock Hz to get the delay between cycles.
	speed = 1 / speed;
	delay = speed * 1000; // Gets milliseconds.
	
	// Try to open the file with UE1 ASM.
	std::ifstream asmfile(filepath);
	if (asmfile.fail()) {
		std::cerr << "Failed to open the file. Please check the path and try again." << std::endl;
		return 1;
	}
	
	// std::cout << out welcome message.
	std::cout << "Welcome to the UE1 emulator.\n";
	std::cout << "Ported from Usagi Electric's QuickBasic version by Maya Posch.\n\n";
	std::cout << "The program will run on a loop until 'Q' is pressed to quit.\n";
	std::cout << "Press 'H' to halt the CPU. A Flag F instruction will also halt.\n";
	std::cout << "If the CPU is halted, press the 'G' key to resume.\n";
	std::cout << "Press 'A' or 'D' to select the desired input switch bit.\n";
	std::cout << "Press 'W' to toggle that bit from 0 to 1 or 1 to 0.\n\n";
	
	std::cout << "Press Enter to start..." << std::endl;
	std::cin.ignore();
	
	// Draw the initial screen.
	std::cout << "INSTRUCTION   : \n";
	std::cout << "MEMORY ADDRESS: \n";
	std::cout << "--------------------\n\n";
	
	std::cout << "REGISTERS\n";
	std::cout << "CARRY     = \n";
	std::cout << "RESULTS   = \n";
	std::cout << "INPUT EN  = \n";
	std::cout << "OUTPUT EN = \n";
	std::cout << "SCRATCH   = \n";
	std::cout << "OUTPUT    = \n";
	std::cout << "INPUT SW. = \n\n";
	
	std::cout << "FLAGS\n";
	std::cout << "FLAG 0    = \n";
	std::cout << "WRITE     = \n";
	std::cout << "I/O CON   = \n";
	std::cout << "RETURN    = \n";
	std::cout << "SKIP Z    = \n\n";

	std::cout << "PROCESSOR IS RUNNING\n\n";
	
	//std::cout << "^\n";
	
	// Create processor thread, pass it the file handle.
	std::thread cpu(run_program, std::ref(asmfile));
	
	// Wait for user input.
	char c;
	while (1) {
		std::cin >> c;
		if (c == 'h') {
			// Send Halt to CPU.
			halt = true;
		}
		else if (c == 'g') {
			// Resume the CPU.
			halt = false;
		}
		else if (c == 'a') {
			// ??
			
		}
		else if (c == 'd') {
			// ??
			
		}
		else if (c == 'w') {
			// Toggle input switch bit.
			
		}
		else if (c == 'q') {
			// Exit.
			stop = true;
			break;
		}
	}
	
	// Wait for the processor thread to finish.
	cpu.join();
	
	return 0;
}
