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
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Lukas Toenne
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef __BKE_CACHE_LIBRARY_H__
#define __BKE_CACHE_LIBRARY_H__

/** \file BKE_cache_library.h
 *  \ingroup bke
 */

#include "DNA_cache_library_types.h"

struct ListBase;
struct Main;
struct Object;
struct Scene;
struct EvaluationContext;
struct ParticleSystem;
struct DupliCache;

struct ClothModifierData;

struct CacheLibrary *BKE_cache_library_add(struct Main *bmain, const char *name);
struct CacheLibrary *BKE_cache_library_copy(struct CacheLibrary *cachelib);
void BKE_cache_library_free(struct CacheLibrary *cachelib);
void BKE_cache_library_unlink(struct CacheLibrary *cachelib);

void BKE_cache_library_make_object_list(struct Main *bmain, struct CacheLibrary *cachelib, struct ListBase *lb);

typedef struct CacheLibraryObjectsIterator {
	ListBase objects;
	LinkData *cur;
} CacheLibraryObjectsIterator;

void BKE_object_cache_iter_init(CacheLibraryObjectsIterator *iter, struct CacheLibrary *cachelib);
bool BKE_object_cache_iter_valid(CacheLibraryObjectsIterator *iter);
void BKE_object_cache_iter_next(CacheLibraryObjectsIterator *iter);
void BKE_object_cache_iter_end(CacheLibraryObjectsIterator *iter);
struct Object *BKE_object_cache_iter_get(CacheLibraryObjectsIterator *iter);

typedef struct CacheLibraryItemsIterator {
	struct Object *ob;
	struct CacheItem *items;
	int totitems;
	
	struct CacheItem *cur;
} CacheLibraryItemsIterator;

void BKE_cache_item_iter_init(CacheLibraryItemsIterator *iter, struct Object *ob);
bool BKE_cache_item_iter_valid(CacheLibraryItemsIterator *iter);
void BKE_cache_item_iter_next(CacheLibraryItemsIterator *iter);
void BKE_cache_item_iter_end(CacheLibraryItemsIterator *iter);

#if 0
typedef void (*CacheGroupWalkFunc)(void *userdata, struct CacheLibrary *cachelib, const struct CacheItemPath *path);
void BKE_cache_library_walk(struct CacheLibrary *cachelib, CacheGroupWalkFunc walk, void *userdata);
#endif

const char *BKE_cache_item_name_prefix(int type);
void BKE_cache_item_name(struct Object *ob, int type, int index, char *name);
int BKE_cache_item_name_length(struct Object *ob, int type, int index);
eCacheReadSampleResult BKE_cache_read_result(int ptc_result);

struct CacheItem *BKE_cache_library_find_item(struct CacheLibrary *cachelib, struct Object *ob, int type, int index);
struct CacheItem *BKE_cache_library_add_item(struct CacheLibrary *cachelib, struct Object *ob, int type, int index);
void BKE_cache_library_remove_item(struct CacheLibrary *cachelib, struct CacheItem *item);
void BKE_cache_library_clear(struct CacheLibrary *cachelib);

bool BKE_cache_library_validate_item(struct CacheLibrary *cachelib, struct Object *ob, int type, int index);
void BKE_cache_library_group_update(struct Main *bmain, struct CacheLibrary *cachelib);

/* ========================================================================= */

bool BKE_cache_archive_path_test(const char *path, ID *id, Library *lib);
void BKE_cache_archive_path(const char *path, ID *id, Library *lib, char *result, int max);

void BKE_cache_library_dag_recalc_tag(struct EvaluationContext *eval_ctx, struct Main *bmain);

bool BKE_cache_test_dupligroup(struct Main *bmain, eCacheLibrary_EvalMode eval_mode, struct Group *dupgroup);
bool BKE_cache_read_dupligroup(struct Main *bmain, struct Scene *scene, float frame, eCacheLibrary_EvalMode eval_mode, struct Group *dupgroup, struct DupliCache *dupcache);

#endif