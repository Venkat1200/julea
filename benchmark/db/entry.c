/*
 * JULEA - Flexible storage framework
 * Copyright (C) 2019-2020 Michael Straßberger
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
#include <julea-db.h>

#include "benchmark.h"

#include "common.c"

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
_benchmark_db_delete(BenchmarkRun* run, gchar const* namespace, gboolean use_batch, gboolean use_index_all, gboolean use_index_single)
{
	gboolean ret;
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	g_autoptr(GError) b_s_error = NULL;
	g_autoptr(JDBSchema) b_scheme = NULL;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);

	b_scheme = _benchmark_db_prepare_scheme(namespace, false, use_index_all, use_index_single, batch, delete_batch);

	g_assert_nonnull(b_scheme);
	g_assert_nonnull(run);

/**********************************/
	gint n = ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER));
    gdouble latency;
	guint perc;
	double latencies[n];
/**********************************/
	while (j_benchmark_iterate(run))
	{
		_benchmark_db_insert(NULL, b_scheme, NULL, true, false, false, false);

		j_benchmark_timer_start(run);

		for (gint i = 0; i < ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER)); i++)
		{
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/
			
			g_autoptr(JDBEntry) entry = j_db_entry_new(b_scheme, &b_s_error);
			g_autofree gchar* string = _benchmark_db_get_identifier(i);
			g_autoptr(JDBSelector) selector = j_db_selector_new(b_scheme, J_DB_SELECTOR_MODE_AND, &b_s_error);

			ret = j_db_selector_add_field(selector, "string", J_DB_SELECTOR_OPERATOR_EQ, string, 0, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

			ret = j_db_entry_delete(entry, selector, batch, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

			if (!use_batch)
			{
				ret = j_batch_execute(batch);
				g_assert_true(ret);
			}
			/**********************************/
			g_timer_stop(func_timer);
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
		perc = (int)((gdouble)0.90 *(gdouble)n);
		if (perc>=n)perc = n - 1;
		run->percLatency90 = latencies[perc];
		
		//-/
		run->latency=0;
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

	ret = j_batch_execute(delete_batch);
	g_assert_true(ret);

	run->operations = ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER));
}

static void
_benchmark_db_update(BenchmarkRun* run, gchar const* namespace, gboolean use_batch, gboolean use_index_all, gboolean use_index_single)
{
	gboolean ret;
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	g_autoptr(GError) b_s_error = NULL;
	g_autoptr(JDBSchema) b_scheme = NULL;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);

	b_scheme = _benchmark_db_prepare_scheme(namespace, false, use_index_all, use_index_single, batch, delete_batch);

	g_assert_nonnull(b_scheme);
	g_assert_nonnull(run);

	_benchmark_db_insert(NULL, b_scheme, NULL, true, false, false, false);

/**********************************/
	gint n=((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER));
    gdouble latency;
	guint perc;
	double latencies[n];
/**********************************/

	
	
	while (j_benchmark_iterate(run))
	{
		for (gint i = 0; i < ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER)); i++)
		{
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/
			
			gint64 i_signed = (((i + N_PRIME) * SIGNED_FACTOR) % CLASS_MODULUS) - CLASS_LIMIT;
			g_autoptr(JDBSelector) selector = j_db_selector_new(b_scheme, J_DB_SELECTOR_MODE_AND, &b_s_error);
			g_autoptr(JDBEntry) entry = j_db_entry_new(b_scheme, &b_s_error);
			g_autofree gchar* string = _benchmark_db_get_identifier(i);

			g_assert_null(b_s_error);

			ret = j_db_entry_set_field(entry, "sint", &i_signed, 0, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

			ret = j_db_selector_add_field(selector, "string", J_DB_SELECTOR_OPERATOR_EQ, string, 0, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

			ret = j_db_entry_update(entry, selector, batch, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

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

                       }else{
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
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	g_assert_true(ret);

	run->operations = ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER));
}

static void
_benchmark_db_workloadScientific(BenchmarkRun* run, gchar const* namespace, gboolean use_index_all, gboolean use_index_single)
{
	
	gboolean ret;
	g_autoptr(JBatch) delete_batch = NULL;
	g_autoptr(JBatch) batch = NULL;
	g_autoptr(JSemantics) semantics = NULL;
	g_autoptr(GError) b_s_error = NULL;
	g_autoptr(JDBSchema) b_scheme = NULL;

	semantics = j_benchmark_get_semantics();
	delete_batch = j_batch_new(semantics);
	batch = j_batch_new(semantics);

	b_scheme = _benchmark_db_prepare_scheme(namespace, false, use_index_all, use_index_single, batch, delete_batch);

	g_assert_nonnull(b_scheme);
	g_assert_nonnull(run);

	_benchmark_db_insert(NULL, b_scheme, NULL, true, false, false, false);

/**********************************/
	gint n = ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER));
   	gdouble latency;
	guint perc;
	double latencies[n];
/**********************************/

	
	j_benchmark_timer_start(run);

	while (j_benchmark_iterate(run))
	{
		
		for (gint i = 0; i < ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER)); i++)
		{
			
			/**********************************/
			g_autoptr(GTimer) func_timer = NULL;
			func_timer = g_timer_new();
                        g_timer_start(func_timer);
			/**********************************/
			for(int ii=0; ii<100; ii++){
			_benchmark_db_workload_insert(run, NULL, "benchmark_insert", false, false, false, true);
			
			JDBType field_type;
			g_autofree gpointer field_value;
			gsize field_length;
			g_autoptr(JDBIterator) iterator;
			g_autofree gchar* string = _benchmark_db_get_identifier(i);
			g_autoptr(JDBSelector) selector = j_db_selector_new(b_scheme, J_DB_SELECTOR_MODE_AND, &b_s_error);

			ret = j_db_selector_add_field(selector, "string", J_DB_SELECTOR_OPERATOR_EQ, string, 0, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

			iterator = j_db_iterator_new(b_scheme, selector, &b_s_error);
			g_assert_nonnull(iterator);
			g_assert_null(b_s_error);

			ret = j_db_iterator_next(iterator, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);

			ret = j_db_iterator_get_field(iterator, "string", &field_type, &field_value, &field_length, &b_s_error);
			g_assert_true(ret);
			g_assert_null(b_s_error);
			}
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
		run->latency=0;
		for (guint iin = 0; iin< n; iin++)
		run->latency = run->latency + latencies[iin];
		run->latency = run->latency/n;
		/**********************************/

		
	}

	j_benchmark_timer_stop(run);

	ret = j_batch_execute(delete_batch);
	g_assert_true(ret);

	run->operations = ((use_index_all || use_index_single) ? N : (N / N_GET_DIVIDER));
}
static void
benchmark_db_workloadScientific(BenchmarkRun* run)
{
	_benchmark_db_workloadScientific(run, "benchmark_get_simple", false, false);
}
/*
static void
benchmark_db_workloadStreaming(BenchmarkRun* run)
{
	_benchmark_db_get_simple(run, "benchmark_get_simple", false, false);
}

static void
benchmark_db_workloadML(BenchmarkRun* run)
{
	_benchmark_db_get_simple(run, "benchmark_get_simple", false, false);
}

static void
benchmark_db_workloadAutoSys(BenchmarkRun* run)
{
	_benchmark_db_get_simple(run, "benchmark_get_simple", false, false);
}*/

static void
benchmark_db_insert(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert", false, false, false, true);
}

static void
benchmark_db_insert_batch(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_batch", true, false, false, true);
}

static void
benchmark_db_insert_index_single(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_index_single", false, false, true, true);
}

static void
benchmark_db_insert_batch_index_single(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_batch_index_single", true, false, true, true);
}

static void
benchmark_db_insert_index_all(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_index_all", false, true, false, true);
}

static void
benchmark_db_insert_batch_index_all(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_batch_index_all", true, true, false, true);
}

static void
benchmark_db_insert_index_mixed(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_index_mixed", false, true, true, true);
}

static void
benchmark_db_insert_batch_index_mixed(BenchmarkRun* run)
{
	_benchmark_db_insert(run, NULL, "benchmark_insert_batch_index_mixed", true, true, true, true);
}

static void
benchmark_db_delete(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete", false, false, false);
}

static void
benchmark_db_delete_batch(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_batch", true, false, false);
}

static void
benchmark_db_delete_index_single(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_index_single", false, false, true);
}

static void
benchmark_db_delete_batch_index_single(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_batch_index_single", true, false, true);
}

static void
benchmark_db_delete_index_all(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_index_all", false, true, false);
}

static void
benchmark_db_delete_batch_index_all(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_batch_index_all", true, true, false);
}

static void
benchmark_db_delete_index_mixed(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_index_mixed", false, true, true);
}

static void
benchmark_db_delete_batch_index_mixed(BenchmarkRun* run)
{
	_benchmark_db_delete(run, "benchmark_delete_batch_index_mixed", true, true, true);
}

static void
benchmark_db_update(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update", false, false, false);
}

static void
benchmark_db_update_batch(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_batch", true, false, false);
}

static void
benchmark_db_update_index_single(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_index_single", false, false, true);
}

static void
benchmark_db_update_batch_index_single(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_batch_index_single", true, false, true);
}

static void
benchmark_db_update_index_all(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_index_all", false, true, false);
}

static void
benchmark_db_update_batch_index_all(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_batch_index_all", true, true, false);
}

static void
benchmark_db_update_index_mixed(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_index_mixed", false, true, true);
}

static void
benchmark_db_update_batch_index_mixed(BenchmarkRun* run)
{
	_benchmark_db_update(run, "benchmark_update_batch_index_mixed", true, true, true);
}

void
benchmark_db_entry(void)
{

	j_benchmark_add("/db/entry/insert", benchmark_db_insert);
	j_benchmark_add("/db/entry/insert-batch", benchmark_db_insert_batch);
	j_benchmark_add("/db/entry/insert-index-single", benchmark_db_insert_index_single);
	j_benchmark_add("/db/entry/insert-batch-index-single", benchmark_db_insert_batch_index_single);
	j_benchmark_add("/db/entry/insert-index-all", benchmark_db_insert_index_all);
	j_benchmark_add("/db/entry/insert-batch-index-all", benchmark_db_insert_batch_index_all);
	j_benchmark_add("/db/entry/insert-index-mixed", benchmark_db_insert_index_mixed);
	j_benchmark_add("/db/entry/insert-batch-index-mixed", benchmark_db_insert_batch_index_mixed);
	j_benchmark_add("/db/entry/delete", benchmark_db_delete);
	j_benchmark_add("/db/entry/delete-batch", benchmark_db_delete_batch);
	j_benchmark_add("/db/entry/delete-index-single", benchmark_db_delete_index_single);
	j_benchmark_add("/db/entry/delete-batch-index-single", benchmark_db_delete_batch_index_single);
	j_benchmark_add("/db/entry/delete-index-all", benchmark_db_delete_index_all);
	j_benchmark_add("/db/entry/delete-batch-index-all", benchmark_db_delete_batch_index_all);
	j_benchmark_add("/db/entry/delete-index-mixed", benchmark_db_delete_index_mixed);
	j_benchmark_add("/db/entry/delete-batch-index-mixed", benchmark_db_delete_batch_index_mixed);
	j_benchmark_add("/db/entry/update", benchmark_db_update);
	j_benchmark_add("/db/entry/update-batch", benchmark_db_update_batch);
	j_benchmark_add("/db/entry/update-index-single", benchmark_db_update_index_single);
	j_benchmark_add("/db/entry/update-batch-index-single", benchmark_db_update_batch_index_single);
	j_benchmark_add("/db/entry/update-index-all", benchmark_db_update_index_all);
	j_benchmark_add("/db/entry/update-batch-index-all", benchmark_db_update_batch_index_all);
	j_benchmark_add("/db/entry/update-index-mixed", benchmark_db_update_index_mixed);
	j_benchmark_add("/db/entry/update-batch-index-mixed", benchmark_db_update_batch_index_mixed);
	j_benchmark_add("/db/entry/workload 1(Scientific app)", benchmark_db_workloadScientific);
/*	j_benchmark_add("/db/entry/workload 2(Streaming)", benchmark_db_workloadStreaming);
	j_benchmark_add("/db/entry/workload 3(Machine Learning)", benchmark_db_workloadML);
	j_benchmark_add("/db/entry/workload 4(Autonomous Sys)", benchmark_db_workloadAutoSys);
*/}
