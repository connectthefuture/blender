/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Blender Foundation (2014).
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/makesrna/intern/rna_depsgraph.c
 *  \ingroup RNA
 */

#include <stdlib.h>

#include "BLI_utildefines.h"
#include "BLI_path_util.h"

#include "RNA_define.h"
#include "RNA_enum_types.h"

#include "rna_internal.h"

#include "DEG_depsgraph.h"

#include "BKE_depsgraph.h"

#ifdef RNA_RUNTIME

#include "DEG_depsgraph_debug.h"

static void rna_Depsgraph_debug_graphviz(Depsgraph *graph, const char *filename)
{
	FILE *f = fopen(filename, "w");
	if (f == NULL)
		return;
	
	DEG_debug_graphviz(graph, f, "Depsgraph", false);
	
	fclose(f);
}

typedef struct DepsgraphEvalDebugInfo {
	const char *filename;
	int step;
	const Depsgraph *graph;
} DepsgraphEvalDebugInfo;

/* generic debug output function */
static void rna_Depsgraph_debug_simulate_cb(DepsgraphEvalDebugInfo *info, const char *message)
{
	char filename[FILE_MAX];
	FILE *f;
	
	BLI_snprintf(filename, sizeof(filename), "%s_%04d", info->filename, info->step);
	f = fopen(filename, "w");
	if (f == NULL)
		return;
	
	DEG_debug_graphviz(info->graph, f, message, true);
	
	fclose(f);
	
	++info->step;
}

static void rna_Depsgraph_debug_simulate(Depsgraph *graph, const char *filename, int context_type)
{
	EvaluationContext eval_ctx;
	DepsgraphEvalDebugInfo debug_info;
	debug_info.filename = filename;
	debug_info.step = 0;
	debug_info.graph = graph;
	
	DEG_debug_eval_init(&debug_info,
	                    (DEG_DebugEvalCb)rna_Depsgraph_debug_simulate_cb);
	
	DEG_graph_flush_updates(graph);

	eval_ctx.mode = context_type;
	DEG_evaluate_on_refresh(&eval_ctx, graph);
	
	DEG_debug_eval_end();
}

#else

static void rna_def_depsgraph(BlenderRNA *brna)
{
	StructRNA *srna;
	FunctionRNA *func;
	PropertyRNA *parm;
	
	static EnumPropertyItem context_type_items[] = {
		{DAG_EVAL_VIEWPORT, "VIEWPORT", 0, "Viewport", "Viewport Display"},
		{DAG_EVAL_RENDER, "RENDER", 0, "Render", "Render Engine DB Conversion"},
		{DAG_EVAL_PREVIEW, "PREVIEW", 0, "Preview", "Preview rendering"},
		{0, NULL, 0, NULL, NULL}
	};
	
	srna = RNA_def_struct(brna, "Depsgraph", NULL);
	RNA_def_struct_ui_text(srna, "Dependency Graph", "");
	
	func = RNA_def_function(srna, "debug_graphviz", "rna_Depsgraph_debug_graphviz");
	parm = RNA_def_string_file_path(func, "filename", NULL, FILE_MAX, "File Name",
	                                "File in which to store graphviz debug output");
	RNA_def_property_flag(parm, PROP_REQUIRED);
	
	func = RNA_def_function(srna, "debug_simulate", "rna_Depsgraph_debug_simulate");
	parm = RNA_def_string_file_path(func, "filename", NULL, FILE_MAX, "File Name",
	                                "File in which to store graphviz debug output");
	RNA_def_property_flag(parm, PROP_REQUIRED);
	parm = RNA_def_enum(func, "context_type", context_type_items, DAG_EVAL_VIEWPORT, "Context Type", "");
}

void RNA_def_depsgraph(BlenderRNA *brna)
{
	rna_def_depsgraph(brna);
}

#endif