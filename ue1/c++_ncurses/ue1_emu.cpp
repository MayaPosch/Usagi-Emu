/*
	ue1_emu.cpp - Main source file of the UE1 emulator with NCurses UI.
	
*/


#include <ncurses/ncurses.h>
#include <ncurses/panel.h>
#include <ncurses/menu.h>

#include <iostream>
#include <cstring>
#include <thread>

#include "sarge.h"

#include "processor.h"


/* void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string) {
	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	mvwprintw(win, y, x, "%s", string);
	refresh();
} */


// Global variables.
WINDOW* ue1;
WINDOW* desc;


// --- UPDATE DISPLAY ---
// Update the status display.
void update_display() {
	// Update with the current opcode, address, flags & register values.
	mvwprintw(ue1, 2, 18, opco.c_str());
	mvwprintw(ue1, 3, 18, mema.c_str());
	
	mvwprintw(ue1, 7, 15, std::to_string(carry).c_str());
	mvwprintw(ue1, 8, 15, std::to_string(rr).c_str());
	mvwprintw(ue1, 9, 15, std::to_string(ien).c_str());
	mvwprintw(ue1, 10, 15, std::to_string(oen).c_str());
	mvwprintw(ue1, 11, 15, std::to_string(sctrg).c_str());
	mvwprintw(ue1, 12, 15, std::to_string(outrg).c_str());
	
	// TODO: Switches.
	
	mvwprintw(ue1, 19, 12, std::to_string(flag0).c_str());
	mvwprintw(ue1, 20, 12, std::to_string(wrt).c_str());
	mvwprintw(ue1, 21, 12, std::to_string(ioc).c_str());
	mvwprintw(ue1, 22, 12, std::to_string(rtn).c_str());
	mvwprintw(ue1, 23, 12, std::to_string(skz).c_str());
	
	if (halt) {
		mvwprintw(ue1, 25, 16, "HALTED ");
	}
	else {
		mvwprintw(ue1, 25, 16, "RUNNING");
	}
	
	wrefresh(desc);	// Show the box.
	wrefresh(ue1);	// Show the box.
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
	
	// Setup the ncurses library.
	initscr();				// Init the library.
	noecho();				// Do not echo user input.
	cbreak();				// Do not buffer user input, retain Ctrl-Z & Ctrl-C functions.
	//raw();					// Do nut buffer any user input. Present all input to program.
	keypad(stdscr, TRUE);	// Enable extended character (e.g. F-keys, numpad) input.
	curs_set(1);			// Change cursor appearance. 0 invisible, 1 normal, 2 strong.
	nodelay(stdscr, TRUE);
	
	// Check for colour support in terminal.
	// FIXME: Exit if not supported.
	if (has_colors() == FALSE) {
		endwin();
		std::cout << "Your terminal doesn't support colours." << std::endl;
		return 1;
	}
	
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE); // fore & background colours.
	
	attron(COLOR_PAIR(1));
	
	// Create a window that we can put the emulator status on.
	// arguments: rows, columns, start_y, start_x
	desc = newwin(12, 70, 2, 2);
	ue1 = newwin(30, 60, 15, 2);
	box(desc, 0, 0);	// use default window outline characters.
	box(ue1, 0, 0);		// use default window outline characters.
	wrefresh(desc);	// Show the box.
	wrefresh(ue1);	// Show the box.
	
	// Use a panel to ease management. 
	//PANEL* desc_panel = new_panel(desc);
	//PANEL* ue1_panel = new_panel(ue1);
	
	// Print the basic text.
	mvwprintw(desc, 0, 2, " Description ");
	mvwprintw(desc, 2, 2, "Welcome to the UE1 emulator.");
	mvwprintw(desc, 3, 2, "Ported from Usagi Electric's QuickBasic version by Maya Posch.");
	mvwprintw(desc, 5, 2, "The program will run in a loop until 'q' is pressed to quit.");
	mvwprintw(desc, 6, 2, "Press 'h' to halt the CPU. A Flag F instruction will also halt.");
	mvwprintw(desc, 7, 2, "If the CPU is halted, press the 'g' key to resume.");
	mvwprintw(desc, 8, 2, "Press 1 - 7 to toggle the input switch bit.");
	
	mvwprintw(ue1, 0, 2, " UE1 ");
	mvwprintw(ue1, 2, 2, "Instruction   : ");
	mvwprintw(ue1, 3, 2, "Memory Address: ");
	mvwprintw(ue1, 4, 2, "--------------------");
	
	mvwprintw(ue1, 6, 2, ">> Registers <<");
	mvwprintw(ue1, 7, 2, "Carry      = ");
	mvwprintw(ue1, 8, 2, "Results    = ");
	mvwprintw(ue1, 9, 2, "Input EN   = ");
	mvwprintw(ue1, 10, 2, "Output EN = ");
	mvwprintw(ue1, 11, 2, "Scratch   = ");
	mvwprintw(ue1, 12, 2, "Output    = ");
	
	mvwprintw(ue1, 14, 2, ">> Input Switches <<");
	mvwprintw(ue1, 15, 2, " 1   2   3   4   5   6   7");
	mvwprintw(ue1, 16, 2, "[ ] [ ] [ ] [ ] [ ] [ ] [ ]");
	
	mvwprintw(ue1, 18, 2, ">> Flags <<");
	mvwprintw(ue1, 19, 2, "Flag 0  = ");
	mvwprintw(ue1, 20, 2, "Write   = ");
	mvwprintw(ue1, 21, 2, "I/O con = ");
	mvwprintw(ue1, 22, 2, "Return  = ");
	mvwprintw(ue1, 23, 2, "Skip Z  = ");
	
	mvwprintw(ue1, 25, 2, "Processor is: RUNNING");
	
	
	// Create processor thread, pass it the file handle.
	std::thread cpu(run_program, std::ref(asmfile));
	
	// Update panel.
	//update_panels();
	//doupdate();
	
	// Do things
	//wgetch(ue1); // Wait for key before exit.
	int key;
	while (1) {
		//update_panels();
		//doupdate();
		wrefresh(desc);	// Show the box.
		wrefresh(ue1);	// Show the box.
		
		key = wgetch(desc);
		//key = getch();
		if (key == 'h') {
			// Send Halt to CPU.
			halt = true;
		}
		else if (key == 'g') {
			// Resume the CPU.
			halt = false;
		}
		else if (key == 'a') {
			// ??
			
		}
		else if (key == 'd') {
			// ??
			
		}
		else if (key == 'w') {
			// Toggle input switch bit.
			
		}
		else if (key == 'q') {
			// Exit.
			stop = true;
			break;
		}
	}
	//while (key != 'q');
	
	//print_in_middle(stdscr, LINES / 2, 0, 0, "Hello World! In color ...");
	attroff(COLOR_PAIR(1));
	
	// Clean up ncurses library.
	delwin(desc);
	delwin(ue1);
	endwin();		// Main ncurses clean-up.
	//echo();			// Echo user input again.
	//curs_set(1);	// Set if changed.
	
	// Wait for the processor thread to finish.
	cpu.join();
	
	return 0;
}
