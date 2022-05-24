 /*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"


void pageit(Pentry q[MAXPROCESSES])
{
    /* Static vars */
    static int initialized = 0;
    static int tick = 1;         
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++)
    {
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++)
        {
		    timestamps[proctmp][pagetmp] = 0; 
	    }
	}
	initialized = 1;
}

    /* Local vars */
    int proc;
    int pc;
    int page;
    int currentPage;
    int SelectedPage;

    /* Loop through processes */
    for (proc = 0; proc < MAXPROCESSES; proc++)
    {
        /* Check if active */
        if (q[proc].active)
        {
            pc = q[proc].pc;
            page = (pc / PAGESIZE);
            /* Check if not swapped in */
            if (!q[proc].pages[page])
            {
                /* Swap in */
                if (!pagein(proc, page))
                {
                    /* If can't swap in, choose page to evict */
                    int lru_time = tick;
                    for (currentPage = 0; currentPage < q[proc].npages; currentPage++)
                    {
                        /* If swapped in, older than curent page? */
                        if (q[proc].pages[currentPage] && timestamps[proc][currentPage] < lru_time)
                        {
                            /* Set the new lru time and the new page */
                            lru_time = timestamps[proc][currentPage];
                            SelectedPage = currentPage;
                        }
                    }
                    pageout(proc, SelectedPage);
                }
            }
            timestamps[proc][page] = tick;
        }
    }
    /* advance time for next pageit iteration */
    tick++;
} 
