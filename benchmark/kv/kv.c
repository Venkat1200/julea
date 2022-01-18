/*
 * JULEA - Flexible storage framework
 * Copyright (C) 2017-2021 Michael Kuhn
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

#include <string.h>

#include <julea.h>
#include <julea-kv.h>

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
_benchmark_kv_put(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1000;
/**********************************/
	guint perc;
	double latencies[n];
   	gdouble latency;
	
/**********************************/

	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);

	while (j_benchmark_iterate(run))
	{
		j_benchmark_timer_start(run);

		for (guint i = 0; i < n; i++)
		{
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;

			name = g_strdup_printf("benchmark-%d", i);
			object = j_kv_new("benchmark", name);
			j_kv_put(object, g_strdup("empty"), 6, g_free, batch);

			j_kv_delete(object, delete_batch);

			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				g_assert_true(ret);
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                         if (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else {
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			g_assert_true(ret);
		}

		j_benchmark_timer_stop(run);

		ret = j_batch_execute(delete_batch);
		g_assert_true(ret);
	}

	run->operations = n;
}

static void
benchmark_kv_put(BenchmarkRun* run)
{
	_benchmark_kv_put(run, FALSE);
}

static void
benchmark_kv_put_batch(BenchmarkRun* run)
{
	_benchmark_kv_put(run, TRUE);
}

static void
_benchmark_kv_get_callback(gpointer value, guint32 len, gpointer data)
{
	(void)len;
	(void)data;

	g_free(value);
}

static void
_benchmark_kv_get(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1000;

/**********************************/
	guint perc;
	double latencies[n];
    	gdouble latency;
	
/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);

	for (guint i = 0; i < n; i++)
	{
		g_autoptr(JKV) object = NULL;
		g_autofree gchar* name = NULL;

		name = g_strdup_printf("benchmark-%d", i);
		object = j_kv_new("benchmark", name);
		j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

		j_kv_delete(object, delete_batch);
	}

	ret = j_batch_execute(batch);
	g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;

			name = g_strdup_printf("benchmark-%d", i);
			object = j_kv_new("benchmark", name);

			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else{
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/

			j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency90  = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n;
}

static void
benchmark_kv_get(BenchmarkRun* run)
{
	_benchmark_kv_get(run, FALSE);
}

static void
benchmark_kv_get_batch(BenchmarkRun* run)
{
	_benchmark_kv_get(run, TRUE);
}
static void
_benchmark_kv_scientificAppWorkload(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1;

/**********************************/
	guint perc;
	double latencies[n];
    	gdouble latency;
	
/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);


	ret = j_batch_execute(batch);
	//g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;
			for (guint ii = 0; ii < 500; ii++)
			{
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d%d", i,ii);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
				
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if  (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else{
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/

			
			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n *1000;
}

static void
benchmark_kv_scientificAppWorkload(BenchmarkRun* run)
{
	_benchmark_kv_scientificAppWorkload(run, FALSE);
}
/*************************************************************************************************************************/
/*************************************************************************************************************************/

static void
_benchmark_kv_WriteIntensiveWorkload(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1;

/**********************************/
	guint perc;
	double latencies[n];
    	gdouble latency;

/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);


	ret = j_batch_execute(batch);
	//g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;
			for (guint ii = 0; ii < 800; ii++)
			{
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d%d", i,ii);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}


			}
			for (guint ii = 0; ii < 200; ii++)
			{

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);

			}
			/**********************************/

			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if  (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else{
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/


			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency90 = latencies[perc];

		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n*1000;
}

static void
benchmark_kv_WriteIntensiveWorkload(BenchmarkRun* run)
{
	_benchmark_kv_WriteIntensiveWorkload(run, FALSE);
}
			
	
/*************************************************************************************************************************/
/*************************************************************************************************************************/
static void
_benchmark_kv_WriteIntensiveWorkload(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1;

/**********************************/
	guint perc;
	double latencies[n];
    	gdouble latency;

/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);


	ret = j_batch_execute(batch);
	//g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;
			for (guint ii = 0; ii < 950; ii++)
			{
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d%d", i,ii);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}


			}
			for (guint ii = 0; ii < 50; ii++)
			{

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);

			}
			/**********************************/

			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if  (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else{
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/


			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency90 = latencies[perc];

		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n*1000;
}

static void
benchmark_kv_WriteIntensiveWorkload(BenchmarkRun* run)
{
	_benchmark_kv_WriteIntensiveWorkload(run, FALSE);
}
static void
_benchmark_kv_MLWorkload(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1;

/**********************************/
	guint perc;
	double latencies[n];
        gdouble latency;
	
/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);


	ret = j_batch_execute(batch);
	//g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;
			for (guint ii = 0; ii < 10; ii++)
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d", i);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}
			for (guint ii = 0; ii < 200; ii++)
			{

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
				
			}
			for (guint ii = 0; ii < 10; ii++)
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d%d", i,ii);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}
			for (guint ii = 0; ii < 200; ii++)
			{

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
				
			}
			for (guint ii = 0; ii < 10; ii++)
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d%d", i,ii);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}
			for (guint ii = 0; ii < 200; ii++)
			{

				name = g_strdup_printf("benchmark-%d", i);
				name = g_strdup_printf("benchmark-%d%d", i,ii);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
				
			}
				for (guint ii = 0; ii < 10; ii++)
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d", i);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}
			for (guint ii = 0; ii < 200; ii++)
			{

				name = g_strdup_printf("benchmark-%d", i);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
				
			}
				for (guint ii = 0; ii < 10; ii++)
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d", i);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}
			for (guint ii = 0; ii < 150; ii++)
			{

				name = g_strdup_printf("benchmark-%d", i);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
				
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else {
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/

			
			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n*1000;
}

static void
benchmark_kv_MLWorkload(BenchmarkRun* run)
{
	_benchmark_kv_MLWorkload(run, FALSE);
}
static void
_benchmark_kv_AutoSysWorkload(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1;

/**********************************/
	guint perc;
	double latencies[n];
    gdouble latency;
	
/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);
	for (guint i = 0; i < n; i++)
	{
		g_autoptr(JKV) object = NULL;
		g_autofree gchar* name = NULL;

		name = g_strdup_printf("benchmark-%d", i);
		object = j_kv_new("benchmark", name);
		j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

		j_kv_delete(object, delete_batch);
	}


	ret = j_batch_execute(batch);
	//g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;
			for (guint ii = 0; ii < 500; ii++)
			{

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);				
				
				{
					g_autoptr(JKV) object = NULL;
					g_autofree gchar* name = NULL;

					name = g_strdup_printf("benchmark-%d%d", i,ii);
					object = j_kv_new("benchmark", name);
					j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

					j_kv_delete(object, delete_batch);
				}
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if (run->min_latency < 0) {
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else{
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/

			
			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n*1000;
}

static void
benchmark_kv_AutoSysWorkload(BenchmarkRun* run)
{
	_benchmark_kv_AutoSysWorkload(run, FALSE);
}
static void
_benchmark_kv_streamingWorkload(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1;

/**********************************/
	guint perc;
	double latencies[n];
    	gdouble latency;
	
/**********************************/
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);

	for (guint i = 0; i < n; i++)
	{
		g_autoptr(JKV) object = NULL;
		g_autofree gchar* name = NULL;

		name = g_strdup_printf("benchmark-%d", i);
		object = j_kv_new("benchmark", name);
		j_kv_put(object, g_strdup(name), strlen(name), g_free, batch);

		j_kv_delete(object, delete_batch);
	}

	ret = j_batch_execute(batch);
	//g_assert_true(ret);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;
				
			for (guint ii = 0; ii < 1000; ii++)
			{

				name = g_strdup_printf("benchmark-%d%d", i,ii);
				object = j_kv_new("benchmark", name);
				j_kv_get_callback(object, _benchmark_kv_get_callback, NULL, batch);
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else {
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/

			
			if (!use_batch)
		{
				ret = j_batch_execute(batch);
				//g_assert_true(ret);
			}
		}
		/**********************************/
		 qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency95 = latencies[perc];
		perc=(int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			//g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	//g_assert_true(ret);

	run->operations = n*1000;
}

static void
benchmark_kv_streamingWorkload(BenchmarkRun* run)
{
	_benchmark_kv_streamingWorkload(run, FALSE);
}

static void
_benchmark_kv_delete(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1000;
/**********************************/
	guint perc;
	double latencies[n];
  	gdouble latency;
	
/**********************************/

	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	batch = j_batch_new(semantics);

	while (j_benchmark_iterate(run))
	{
		for (guint i = 0; i < n; i++)
		{
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;

			name = g_strdup_printf("benchmark-%d", i);
			object = j_kv_new("benchmark", name);
			j_kv_put(object, g_strdup("empty"), 6, g_free, batch);
		}

		ret = j_batch_execute(batch);
		g_assert_true(ret);

		j_benchmark_timer_start(run);

		for (guint i = 0; i < n; i++)
		{
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;

			name = g_strdup_printf("benchmark-%d", i);
			object = j_kv_new("benchmark", name);

			j_kv_delete(object, batch);

			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				g_assert_true(ret);
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else {
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		perc = (int)((gdouble)0.95*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency95 = latencies[perc];
		perc = (int)((gdouble)0.90*(gdouble)n);
		if (perc>=n)perc = n-1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			g_assert_true(ret);
		}

		j_benchmark_timer_stop(run);
	}

	run->operations = n;
}

static void
benchmark_kv_delete(BenchmarkRun* run)
{
	_benchmark_kv_delete(run, FALSE);
}

static void
benchmark_kv_delete_batch(BenchmarkRun* run)
{
	_benchmark_kv_delete(run, TRUE);
}

static void
_benchmark_kv_unordered_put_delete(BenchmarkRun* run, gboolean use_batch)
{
	guint const n = 1000;
/**********************************/
	guint perc;
	double latencies[n];
   	gdouble latency;
	
/**********************************/

	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	gboolean ret;

	semantics = j_benchmark_get_semantics();
	batch = j_batch_new(semantics);

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
			g_autoptr(JKV) object = NULL;
			g_autofree gchar* name = NULL;

			name = g_strdup_printf("benchmark-%d", i);
			object = j_kv_new("benchmark", name);
			j_kv_put(object, g_strdup("empty"), 6, g_free, batch);
			j_kv_delete(object, batch);

			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				g_assert_true(ret);
			}
			/**********************************/
			
			latency = 1000000* g_timer_elapsed(func_timer, NULL);
			latencies[i] = latency;
                        if (run->min_latency < 0){
                            run->min_latency = latency;
                            run->max_latency = latency;

                       }else {
                            if (latency > run->max_latency)run->max_latency = latency;
                            if (latency < run->min_latency)run->min_latency = latency;
                        }
			/**********************************/
		}
		/**********************************/
		qsort(latencies, n, sizeof(double), compare);
		
		perc = (int)((gdouble)0.95*(gdouble)n);
		
		if (perc>=n)perc = n-1;
		
			run->percLatency95 = latencies[perc];
		
		perc = (int)((gdouble)0.90*(gdouble)n);
		
		if (perc>=n)perc = n-1;
		
			run->percLatency90 = latencies[perc];
		
		//-/
		run->latency = 0;
		
        	for (guint iin = 0; iin < n; iin++)
			
        		run->latency = run->latency + latencies[iin];
		
       		run->latency = run->latency / n;
		/**********************************/

		if (use_batch)
		{
			ret = j_batch_execute(batch);
			g_assert_true(ret);
		}
	}

	j_benchmark_timer_stop(run);

	run->operations = n * 2;
}

static void
benchmark_kv_unordered_put_delete(BenchmarkRun* run)
{
	_benchmark_kv_unordered_put_delete(run, FALSE);
}

static void
benchmark_kv_unordered_put_delete_batch(BenchmarkRun* run)
{
	_benchmark_kv_unordered_put_delete(run, TRUE);
}

void
benchmark_kv(void)
{
	j_benchmark_add("/kv/put", benchmark_kv_put);
	j_benchmark_add("/kv/put-batch", benchmark_kv_put_batch);
	j_benchmark_add("/kv/get", benchmark_kv_get);
	j_benchmark_add("/kv/get-batch", benchmark_kv_get_batch);
	j_benchmark_add("/kv/delete", benchmark_kv_delete);
	j_benchmark_add("/kv/delete-batch", benchmark_kv_delete_batch);
	j_benchmark_add("/kv/unordered-put-delete", benchmark_kv_unordered_put_delete);
	j_benchmark_add("/kv/unordered-put-delete-batch", benchmark_kv_unordered_put_delete_batch); 
	j_benchmark_add("/kv/benchmark_kv_streamingWorkload", benchmark_kv_streamingWorkload);
	j_benchmark_add("/kv/benchmark_kv_scientificAppWorkload", benchmark_kv_scientificAppWorkload); 
 	j_benchmark_add("/kv/benchmark_kv_AutoSysWorkload", benchmark_kv_AutoSysWorkload); 
        j_benchmark_add("/kv/benchmark_kv_MLWorkload", benchmark_kv_MLWorkload); 
  	j_benchmark_add("/kv/benchmark_kv_WriteIntensiveWorkload", benchmark_kv_WriteIntensiveWorkload);    
}
