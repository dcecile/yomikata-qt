#ifndef MAIN_H
#define MAIN_H

#ifndef UNIT_TESTING
int main(int argc, char *argv[]);
#else
int application_main(int argc, char *argv[]);
#endif

#endif
