#ifndef _grid_terrain_h_
#define _grid_terrain_h_

#include "precision.h"
#include "hashmap.h"

struct SectorTri {
	int sector_num;
	int tri_num;
};
typedef struct SectorTri SectorTri; 

struct SelectedTri {
	SectorTri key;
	int selected;
};
typedef struct SelectedTri SelectedTri; 

struct Sector {
	int sector_num;
	Real pos[3];
	Real color[3];
};
typedef struct Sector Sector;

struct SectorIter {
	int sector_num;
	int begin;
	int end;
};
typedef struct SectorIter SectorIter;

int sector_num_cols();
int sector_num_rows();

// sets triangle size of grid (default = 1)
void grid_terrain_set_unit(Real unit);

// sets terrain color prior to grid_terrain_create()
void grid_terrain_set_color(const Real* color);

/*
	descrip: 
		Creates a grid of flat terrain sectors, where number of sectors is NUM_SECTORS_PER_GRID_EDGE^2 (currently 20*20).
		TODO: change NUM_SECTORS_PER_GRID_EDGE to a user-definable value.
*/
void grid_terrain_create();

/* 
	descrip: 
		Sets the center drawn grid sector based on current camera eye position.

	see also: cam.lib/cam_get(eye, cen, up)
*/
void grid_terrain_update_cur_sector();

/* 
	descrip:
		Draws current terrain sector and num_adj_levels of adjacent neighbor sectors.
*/
void grid_terrain_draw(int num_adj_levels);

/* 
	descrip:
		Initializes param struct which can then be used to iterate num_adj_levels of sectors adjacent to the current sector, 
		including the current sector itself.

	notes:
		* if a valid first sector was found for the requested iteration 
		  then
				* iter_ret->sector_num is set to the first sector of the iteration
				* iter_ret->begin is set to 1
				* iter_ret->end is set to 0
		  else
			  iter_ret->end is set to 1
*/
void grid_terrain_sector_iter(int num_adj_levels, SectorIter* iter_ret);

/*
	descrip:
		Increments the param iterator to the next sector in the iteration.				

	notes:
		* if a valid next sector was found for the requested iteration 
		  then
				* iter_ret->sector_num is set to the next sector of the iteration
				* iter_ret->begin is set to 0
				* iter_ret->end is set to 0
		  else
			  iter_ret->end is set to 1
*/
void grid_terrain_sector_next(int num_adj_levels, SectorIter* iter);

/*
	descrip:
		Copies points of tri in grid at param sector and triangle number to param tri.

	returns:
		1 if tri points were set, else 0
		
*/
int grid_terrain_get_sector_tri(int sector_num, int tri_num, Real* tri_ret);

int grid_terrain_is_tri_selected(int sector_num, int tri_num);
void grid_terrain_select_tri(int sector_num, int tri_num, int select_state);

void grid_terrain_destroy();

void grid_terrain_dump_selected_tris();

Hashmap* grid_terrain_get_selected_tris();

// sets drawing of terrain triangles - one of : {GL_TRIANGLES, GL_LINE_LOOP}
void grid_terrain_set_draw_op(int op);

// exported hash funcs that can be used to create hashmaps of grid sector tris
int SectorTri_hash_func(void* key);
bool SectorTri_equals_func(void* keyA, void* keyB);


#endif
