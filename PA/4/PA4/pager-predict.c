/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 * Solution and code below derived from work by Rudy G. Hill: 
 * https://github.com/rudyghill/os-projects/blob/master/PA4/src/pager-predict.c
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES])
{
    /* Static vars */
    static int lastpc[MAXPROCESSES] = {0};
    static int program_type[MAXPROCESSES] = {0};

    /* Local vars */
    int proc = 0;
    int pc = 0;
    int page = 0;
    int i = 0;

    // Check for active processes, and filter out inactive processes(dormant , completed)
    for (proc = 0; proc < MAXPROCESSES; proc++)
    {
        if (!q[proc].active)
        {
            continue;
        }
        
        /* program counter */
        pc = q[proc].pc;

        /* current page */
        page = pc / PAGESIZE;

        /* pc intra-process jump distance*/
        int pc_diff = lastpc[proc] - pc;

        /* Check if we are jumping backwards, and unknown program type */
        if (pc_diff > 1 && program_type[proc] == 0)
        {
            switch (pc_diff)
            {
            case 1533:                         // Program Counter
                program_type[proc] = 1;
                for (i = 12; i <= 14; i++)
                    pageout(proc, i);
                break;

            case 1129:
                program_type[proc] = 2;
                for (i = 9; i <= 14; i++)
                    pageout(proc, i);
                break;

            case 516:
            case 1683:
                program_type[proc] = 3;
                pageout(proc, 14);
                break;

            case 501:
            case 503:
                program_type[proc] = 5;
                for (i = 4; i <= 14; i++)
                    pageout(proc, i);
                break;
            }
        }

        /* Check if process is about to exit, reset its classification to program type = 0 */
        if ((program_type[proc] == 0 && pc == 1911) || (program_type[proc] == 1 && pc == 1534) || (program_type[proc] == 2 && pc == 1130) || (program_type[proc] == 3 && pc == 1684) || (program_type[proc] == 5 && pc == 504))
        {
            program_type[proc] = 0;
            lastpc[proc] = -1;
        }

        /* Page currently needed */
        pagein(proc, page);

        /* Program type = 4, is same as program type = 0 */
        if (program_type[proc] == 0)
        {
            /* Load 3 pages ahead, and remove 3 pages back */
            for (int i = 1; i < 4; i++)
            {
                pagein(proc, page + i);
                pageout(proc, page - i);
            }
        }
        else if (program_type[proc] == 1)
        {
            switch (page)
            {
            case 0:                     // Page Number
                pagein(proc, 1);
                pageout(proc, 11);
                break;

            case 3:
                pagein(proc, 4);
                pagein(proc, 10);
                pagein(proc, 11);
                pageout(proc, 2);
                break;

            case 4:
                pagein(proc, 5);
                pageout(proc, 3);
                pageout(proc, 10);
                break;

            case 10:
                pagein(proc, 11);
                pagein(proc, 0);
                pageout(proc, 9);
                pageout(proc, 3);
                pageout(proc, 4);
                break;

            case 11:
                pagein(proc, 0);
                pageout(proc, 10);

            default:
                pagein(proc, page + 1);
                pageout(proc, page - 1);
                break;
            }
        }
        else if (program_type[proc] == 2)
        {
            switch (page)
            {
            case 0:
                pagein(proc, 1);
                pageout(proc, 8);
                break;

            case 8:
                pagein(proc, 0);
                pageout(proc, 7);
                break;

            default:
                pagein(proc, page + 1);
                pageout(proc, page - 1);
                break;
            }
        }
        else if (program_type[proc] == 3)
        {
            switch (page)
            {
                case 0:
                    pagein(proc, 1);
                    pageout(proc, 9);
                    pageout(proc, 13);
                    break;
                    
                case 9:
                    pagein(proc, 10);
                    pageout(proc, 0);
                    pageout(proc, 13);
                    break;

                case 12:
                    pagein(proc, 0);
                    pagein(proc, 9);
                    pagein(proc, 13);
                    pageout(proc, 11);
                    break;

                case 13:
                    pagein(proc, 0);
                    pagein(proc, 9);
                    pageout(proc, 12);
                    break;

                default:
                    pagein(proc, page + 1);
                    pageout(proc, page - 1);
                    break;
            }
        }
        /* Program type = 4, is same as program type = 0 */
        else if (program_type[proc] == 5)
        {
            switch (page)
            {
            case 0:
                pagein(proc, 1);
                pageout(proc, 3);
                pageout(proc, 4);
                break;

            case 3:
                pagein(proc, 0);
                pageout(proc, 2);
                break;

            default:
                pagein(proc, page + 1);
                pageout(proc, page - 1);
                break;
            }
        }
        lastpc[proc] = pc;
    }
}