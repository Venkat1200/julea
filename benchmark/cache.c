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

#include <jcache.h>

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
benchmark_cache_get_release(BenchmarkRun* run)
{
	guint const n = 100000;
/**********************************/
	int perc;
	double latencies[n];
    gdouble latency;
	
/**********************************/

	JCache* cache;

	cache = j_cache_new(n);

	j_benchmark_timer_start(run);

	while (j_benchmark_iterate(run))
	{
		for (guint i = 0; i < n; i++)
		{
			gpointer buf;
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/

			buf = j_cache_get(cache, 1);
			j_cache_release(cache, buf);
			
			
			/**********************************/
			g_timer_stop(func_timer);
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


	}
	j_benchmark_timer_stop(run);

	j_cache_free(cache);

	run->operations = n * 2;
	
	
}

void
benchmark_cache(void)
{
	j_benchmark_add("/cache/get-release", benchmark_cache_get_release);
}
