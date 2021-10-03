/*
 * JULEA - Flexible storage framework
 * Copyright (C) 2010-2021 Michael Kuhn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <julea-config.h>

#include <glib.h>

#include <julea.h>

#include <jmemory-chunk.h>

#include "benchmark.h"
/**********************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
static int compare (const void * a, const void * b)
{
    if (*(const double*)a > *(const double*)b) return 1;
    else if (*(const double*)a < *(const double*)b) return -1;
    else return 0;
}
/**********************************/

static void
benchmark_memory_chunk_get(BenchmarkRun* run)
{
	guint const n = 10000000;
/**********************************/
	guint perc;
	double latencies[n];
    gdouble latency;
	
/**********************************/

	JMemoryChunk* memory_chunk;

	memory_chunk = j_memory_chunk_new(n);

	j_benchmark_timer_start(run);

	while (j_benchmark_iterate(run))
	{
		for (guint i = 0; i < n; i++)
		{
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/
			
			j_memory_chunk_get(memory_chunk, 1);
			/**********************************/
			
			latency =1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i]=latency;
                        if(run->min_latency < 0){
                            run->min_latency=latency;
                            run->max_latency=latency;

                       }else{
                            if(latency>run->max_latency)run->max_latency=latency;
                            if(latency<run->min_latency)run->min_latency=latency;
                        }
			/**********************************/
			
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc=(int)((gdouble)0.95*(gdouble)n);
		if(perc>=n)perc=n-1;
		run->percLatnecy95=latencies[perc];
		perc=(int)((gdouble)0.90*(gdouble)n);
		if(perc>=n)perc=n-1;
		run->percLatnecy90=latencies[perc];
		/**********************************/


		j_memory_chunk_reset(memory_chunk);
	}

	j_benchmark_timer_stop(run);

	j_memory_chunk_free(memory_chunk);

	run->operations = n;
}

void
benchmark_memory_chunk(void)
{
	j_benchmark_add("/memory-chunk/get-reset", benchmark_memory_chunk_get);
}
