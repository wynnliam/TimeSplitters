// Illtyd Wynn, 7-21-2026, TimeSplitters

#include <ts/logo/logo.h>
#include <stdio.h>

void logo_startup() {
  printf("Welcome to...\n");
  logo_print_logo();
  printf("It's time to split!\n");
}

void logo_print_logo() {
  printf(" _____ _                __       _ _ _   _                \n");
  printf("/__   (_)_ __ ___   ___/ _\\_ __ | (_| |_| |_ ___ _ __ ___ \n");
  printf("  / /\\| | '_ ` _ \\ / _ \\ \\| '_ \\| | | __| __/ _ | '__/ __|\n");
  printf(" / /  | | | | | | |  ___\\ | |_) | | | |_| ||  __| |  \\__ \\\n");
  printf(" \\/   |_|_| |_| |_|\\___\\__| .__/|_|_|\\__|\\__\\___|_|  |___/\n");
  printf("                          |_|                             \n");
}

