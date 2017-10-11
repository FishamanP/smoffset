/* Copyright (C) 2017 Paul Fisher ("Fishaman P")
 * 
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY,
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * The source code and binaries for this software may be freely modified and redistributed, with
 * only the following restrictions, lifted only by the express written consent of the author:
 *     - Both the copyright notice and this license must be reproduced, unmodified and in full,
 *         in all redistributed copies and derivative works.
 *     - You may not profit in any way from the redistribution of this software or any
 *         derivative works, including, but not limited to, direct sale of this software,
 *         charging for redistribution of this software, or advertising that profits
 *         specifically from this software.
 */

#include <stdbool.h>
#include <stdio.h> // stdout/file operations
#include <stdlib.h> // strtod(), exit()
#include <string.h> // memcmp(), strcpy()

#define BUFSIZE 4096 // maximum length of a line in simfile

static void usage()
{
    puts("Adjusts the offset of a Stepmania simfile.\n");
    puts("smoffset simfile delta\n");
    puts("  simfile\tThe path to the .sm or .ssc file to be adjusted.");
    puts("  delta\t\tThe amount, in seconds, to add to the offset. Can be negative.\n");
}

int main(int argc, char *argv[])
{
    FILE *simfile, *tmp;
    double delta, offset;
    char buffer[BUFSIZE];
    char tempName[L_tmpnam + 4];
    const char match[] = "#OFFSET:";
    bool offsetNotYetFound = true;
    
    // handle arguments
    
    if (argc != 3)
    {
        usage();
        exit(EXIT_SUCCESS);
    }
    
    simfile = fopen(argv[1], "r");
    if (simfile == NULL)
    {
        puts(argv[1]);
        puts("ERROR: simfile could not be opened!\n");
        exit(EXIT_FAILURE);
    }
    
    delta = strtod(argv[2], NULL);
    if (delta == 0.0)
    { // errors cause offset = 0.0
        puts("WARNING: 0 offset. Offset will not be adjusted.\n");
    }
    
    // start of actual functionality
    
    tmp = fopen(tmpnam(tempName), "w+");
    if (tmp == NULL)
    {
        puts("ERROR: Could not allocate temporary file!\n");
        exit(EXIT_FAILURE);
    }
    
    // read the simfile line by line to find the offset
    // if multiple offset lines are found, change them all
    // TODO: for simfiles with multiple offset lines, test how Stepmania and oITG behave
    // TODO: check expected behavior of OFFSET for split timing SSC's
    while(fgets(buffer, BUFSIZE, simfile) != NULL)
    {
        if (memcmp(buffer, match, 8) != 0)
        { // most lines in the file
            fputs(buffer, tmp);
        }
        else
        { // offset line
            offsetNotYetFound = false;
            offset = strtod(&buffer[8], NULL); // errors cause offset = 0.0, convenient
            fprintf(tmp, "%s%#.6f;\n", match, offset + delta);
        }
    }
    fflush(tmp);
    fclose(simfile);
    
    if (offsetNotYetFound)
    { // add it manually
        rewind(tmp);
        char tempName2[L_tmpnam + 4];
        FILE *tmp2 = fopen(tmpnam(tempName2), "w");
        if (tmp2 == NULL)
        {
            puts("ERROR: Could not allocate temporary file 2!\n");
            exit(EXIT_FAILURE);
        }
        
        // add an offset line, with program argument delta as the offset
        fprintf(tmp2, "%s%#.6f;\n", match, delta);
        
        // copy the rest of the file over
        while(fgets(buffer, BUFSIZE, tmp) != NULL)
        {
            fputs(buffer, tmp2);
        }
        
        remove(tempName);
        fflush(tmp2);
        fclose(tmp2);
        strcpy(tempName, tempName2);
    }
    fclose(tmp);
    
    if (remove(argv[1]))
    {
        puts("ERROR: Could not overwrite simfile!\n");
        exit(EXIT_FAILURE);
    }
    if (rename(tempName, argv[1]))
    {
        puts("ERROR: Could not rename working file to original name! Please fix manually.\n");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
