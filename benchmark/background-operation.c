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

#include <jbackground-operation.h>

#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
gint benchmark_background_operation_counter;

static gpointer
on_background_operation_completed(gpointer data)
{
	(void)data;

	g_atomic_int_inc(&benchmark_background_operation_counter);

	return NULL;
}
static int compare (const void * a, const void * b)
{
    if (*(const double*)a > *(const double*)b) return 1;
    else if (*(const double*)a < *(const double*)b) return -1;
    else return 0;
}
static void
benchmark_background_operation_new_ref_unref(BenchmarkRun* run)
{
	guint const n = 100000;
	
    gdouble latency;
	JBackgroundOperation* background_operation;
	double latencies[n];
	j_benchmark_timer_start(run);

	while (j_benchmark_iterate(run))
	{
		g_atomic_int_set(&benchmark_background_operation_counter, 0);

		for (guint i = 0; i < n; i++)
		{
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			background_operation = j_background_operation_new(on_background_operation_completed, NULL);
			j_background_operation_unref(background_operation);
			g_timer_stop(func_timer);
//			g_print("%s (%.0f/s)\n", run->name ,1000000* g_timer_elapsed(func_timer, NULL));
			latency =1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i]=latency;
                        if(run->min_latency < 0){
                            run->min_latency=latency;
                            run->max_latency=latency;

                       }else{
                            if(latency>run->max_latency)run->max_latency=latency;
                            if(latency<run->min_latency)run->min_latency=latency;
                        }
		}
		qsort(latencies, n, sizeof(double), compare);
		for(int a=0;a<n;a++)
		printf("%.3f\n",latencies[a]);
		 
		/* FIXME overhead? */
		while ((guint64)g_atomic_int_get(&benchmark_background_operation_counter) != n)
		{
		}
	}

	j_benchmark_timer_stop(run);

	run->operations = n;
}

void
benchmark_background_operation(void)
{
	j_benchmark_add("/background-operation/new", benchmark_background_operation_new_ref_unref);
}
