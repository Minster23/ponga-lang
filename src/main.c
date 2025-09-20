#include <stdio.h>
#include "process.h"

int main(int argc, char *argv[]) {

    if (argc > 1) {
	start(argv[1]);
    } else {
        printf("Invalid command or wrong file path\n");
    }

    return 0;
}

