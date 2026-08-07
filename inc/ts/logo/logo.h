// Illtyd Wynn, 7-21-2026, TimeSplitters

/*
  As the ceremonial first bit of code (and to also set up my build system), I
  like to add a routine that prints the title of the project to the command
  line.
*/

#pragma once

//
// Prints the logo + extra messages. This is intended to be used at startup of
// the entire program.
//

void logo_startup();

//
// This prints the TimeSplitters logo which I got from using figlet to make a
// fancy command-line string.
//

void logo_print_logo();
