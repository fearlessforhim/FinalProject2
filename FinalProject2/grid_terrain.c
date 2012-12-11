// grid_terrain.c	:	simple, flat terrain renders only sections near to camera

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>

#include "camera.h"
#include "geom.h"
#include "grutil.h"
#include "hashmap.h"
#include "precision.h"

#include "grid_terrain.h"

static int num_units_per_sector_edge = 3;
static int num_sectors_per_grid_edge = 3;
static Real grid_unit = 1;
static Sector* cur_sector = NULL;
static Sector** sectors = NULL;
static Hashmap* selected_tris = NULL;
static int sel_tri_hash_init_size = 64;
//static int sector_num_height = 1;
static Real selected_tri_alpha = 0.5;
static Real selected_tri_y_offset = 0.01;
static int _draw_op = GL_LINE_LOOP;
static Real terrain_color[3];
static int have_terrain_color = 0;
static Real selected_color[3] = {1, 1, 0};

void grid_terrain_set_unit(Real unit) {grid_unit = unit;}

void grid_terrain_set_color(const Real* color) {
	geom_vector3_copy(color, terrain_color);
	have_terrain_color = 1;
}

void grid_terrain_set_draw_op(int op) {
	_draw_op = op;
}

static Real randr() {
	return (Real)rand() / (Real)RAND_MAX;
}

int sector_num_cols() { return num_units_per_sector_edge; }
int sector_num_rows() { return num_units_per_sector_edge; }

static int grid_num_cols() {
	return num_sectors_per_grid_edge;
}

static int grid_num_rows() {
	return grid_num_cols();
}

static int sector_width() {
	return sector_num_cols() * grid_unit;
}

static int grid_width() {
	return sector_width() * grid_num_cols();
}

static int grid_num_sectors() {
	return grid_num_cols() * grid_num_rows();
}

static int sector_col(int sector_num) {
	return sector_num % grid_num_cols();
}

static int sector_row(int sector_num) {
	return sector_num / grid_num_rows();
}

static Sector* get_sector(int row, int col) {
	if(row < 0 || row >= grid_num_rows() || col < 0 || col >= grid_num_cols())
		return NULL;
	int i = row * grid_num_cols() + col;
	assert(i >= 0 && i < grid_num_sectors());
	return sectors[i];
}

int SectorTri_hash_func(void* key) {
	return hashmapHash(key, sizeof(SectorTri));
}

bool SectorTri_equals_func(void* keyA, void* keyB) {
	SectorTri* skA = (SectorTri*) keyA;
	SectorTri* skB = (SectorTri*) keyB;
	return skA->sector_num == skB->sector_num && skA->tri_num == skB->tri_num;
}

void grid_terrain_sector_iter(int num_adj_levels, SectorIter* iter_ret) {
	iter_ret->end = 1;		

	// invalid adj level
	if(num_adj_levels < 0) {
		return;
	}
	// only cur sector
	else if(num_adj_levels == 0) {
		iter_ret->sector_num = cur_sector->sector_num;
		iter_ret->begin = 1;		
		iter_ret->end = 0;
		return;
	}
	// iterate to the first valid sector in the requested adj level
	int cur_sector_col = sector_col(cur_sector->sector_num);
	int cur_sector_row = sector_row(cur_sector->sector_num);
	int nabor_row;
	for(nabor_row = -num_adj_levels; nabor_row <= num_adj_levels; ++nabor_row) {
		int row = cur_sector_row + nabor_row;
		int nabor_col;
		for(nabor_col = -num_adj_levels; nabor_col <= num_adj_levels; ++nabor_col) {
			int col = cur_sector_col + nabor_col;
			Sector* sector = get_sector(row, col);	
			if(sector) {
				iter_ret->sector_num = sector->sector_num;
				iter_ret->begin = 1;		
				iter_ret->end = 0;
				return;
			}
		}
	}
}

void grid_terrain_sector_next(int num_adj_levels, SectorIter* iter) {
//printf("beg grid_terrain_sector_next(): iter->sector_num: %d\n", iter->sector_num);
	iter->end = 1;		

	// invalid adj level
	if(num_adj_levels < 0) {
		return;
	}
	// only cur sector
	else if(num_adj_levels == 0) {
		return;
	}

	// iterate to the iter sector, then next valid sector in the requested adj level
	int found_iter_sector = 0;
	int cur_sector_col = sector_col(cur_sector->sector_num);
	int cur_sector_row = sector_row(cur_sector->sector_num);
	int nabor_row;
	for(nabor_row = -num_adj_levels; nabor_row <= num_adj_levels; ++nabor_row) {
		int row = cur_sector_row + nabor_row;
		int nabor_col;
		for(nabor_col = -num_adj_levels; nabor_col <= num_adj_levels; ++nabor_col) {
			int col = cur_sector_col + nabor_col;
			Sector* sector = get_sector(row, col);	
//printf("grid_terrain_sector_next(): row: %d, col: %d: checking sector %d\n", row, col, sector->sector_num);

			if(sector->sector_num == iter->sector_num) {
				found_iter_sector = 1;
				continue;
			}

			if(sector && found_iter_sector) {
				iter->sector_num = sector->sector_num;
				iter->begin = 0;		
				iter->end = 0;
				return;
			}
		}
	}
}

int grid_terrain_get_sector_tri(int sector_num, int tri_num, Real* tri_ret) {
	int col = sector_col(sector_num);
	int row = sector_row(sector_num);
	Sector* sector = get_sector(row, col);

	float x0 = -(grid_unit * sector_num_cols()) / 2 + sector->pos[0];
	float y = sector->pos[1];
	float z = -(grid_unit * sector_num_rows()) / 2 + sector->pos[2];
	int tri = 0;
	int i;
	for(i = 0; i <= sector_num_rows(); ++i) {
		float x = x0;
		int j;
		for(j = 0; j <= sector_num_cols(); ++j) {
			if(tri == tri_num) {
				geom_vector3_set(tri_ret + 0, x, y, z);
				geom_vector3_set(tri_ret + 3, x, y, z + grid_unit);
				geom_vector3_set(tri_ret + 6, x + grid_unit, y, z + grid_unit);
				return 1;
			}
			++tri;
			if(tri == tri_num) {
				geom_vector3_set(tri_ret + 0, x, y, z);
				geom_vector3_set(tri_ret + 3, x + grid_unit, y, z + grid_unit);
				geom_vector3_set(tri_ret + 6, x + grid_unit, y, z);
				return 1;
			}
			++tri;
		
			x += grid_unit;
		}
		z += grid_unit;
	}
	return 0;
}

void grid_terrain_select_tri(int sector_num, int tri_num, int select_state) {
//printf("beg	grid_terrain_select_tri(): sector_num: %d, tri_num: %d, select_state: %d\n", sector_num, tri_num, select_state);
	// look for existing sector in edit hash, create if not found
	SectorTri key = {sector_num, tri_num};
	SelectedTri* st = hashmapGet(selected_tris, &key); 
	if(st == NULL) {
		st = malloc(sizeof(SelectedTri));
		st->key.sector_num = sector_num;
		st->key.tri_num = tri_num;
		hashmapPut(selected_tris, &st->key, st);
//printf("put new hash entry\n");
	}
	st->selected = select_state;
}

static int pos_in_sector(Sector* sector, Real* pos) {
	Real d = sector_width() / 2;
	return 
		pos[0] >= sector->pos[0]-d && pos[0] <= sector->pos[0]+d &&
		pos[2] >= sector->pos[2]-d && pos[2] <= sector->pos[2]+d;
}

/*
	nabor sector dirs:
	0: left  (-x)
	1: front (+z)
	2: right (+x)
	3: back  (-z)
*/
static void get_nabor_dir(int nabor_num, int* dir_ret) {
	switch(nabor_num) {
	case 0: dir_ret[0] = -1, dir_ret[1] =  0; break;
	case 1: dir_ret[0] =  0, dir_ret[1] =  1; break;
	case 2: dir_ret[0] =  1, dir_ret[1] =  0; break;
	case 3: dir_ret[0] =  0, dir_ret[1] = -1; break;
	default:
		assert(0);
	}
}

static Sector* get_sector_nabor(int sector_num, int nabor_num) {
	int col = sector_col(sector_num);
	int row = sector_row(sector_num);
	int dir[2];
	get_nabor_dir(nabor_num, dir);
	int nabor_col = col + dir[0];
	int nabor_row = row + dir[1];
	return get_sector(nabor_row, nabor_col);
}

static Sector* sector_from_pos(Real* pos) {
	if(pos_in_sector(cur_sector, pos))
		return cur_sector;
	int i;
	for(i = 0; i < 4; ++i) {
		Sector* nabor = get_sector_nabor(cur_sector->sector_num, i);
		if(nabor!= NULL && pos_in_sector(nabor, pos))
			return nabor;
	}
	return NULL;
}

static Sector* create_sector(int sector_num) {
	Sector* sector = (Sector*) malloc(sizeof(Sector));

	sector->sector_num = sector_num;

	// calc pos
	int col = sector_col(sector_num);
	int row = sector_row(sector_num);
	Real pos[3] = {col * sector_width(), 0, row * sector_width()};

	// adjust sector pos from grid corner to grid center
	Real grid_half_dim[3] = {grid_width()/2, 0, grid_width()/2};
	geom_vector3_sub(pos, grid_half_dim, sector->pos);

	// init sector data
	if(have_terrain_color) {
		geom_vector3_copy(terrain_color, sector->color);
	}
	else {
		Real rr = randr();
		Real rg = randr();
		Real rb = randr();

		Real r = (rr>=0.5) ? 0.5 + randr()*0.5 : 0.25;
		Real g = (rg>=0.5) ? 0.5 + randr()*0.5 : 0.25;
		Real b = (rb>=0.5) ? 0.5 + randr()*0.5 : 0.25;

		Real color[3] = {r, g, b};
		geom_vector3_copy(color, sector->color);
	}

	return sector;
}

static void create_sectors() {
	int num_sectors = grid_num_sectors();	
	sectors = (Sector**) malloc(sizeof(Sector*) * num_sectors);
	int i;
	for(i = 0; i < num_sectors; ++i) {
		sectors[i] = create_sector(i);
	}

	int center_row = grid_num_rows() / 2;
	int center_sector = center_row * grid_num_cols() + grid_num_cols() / 2;
	cur_sector = sectors[center_sector];
}

void grid_terrain_create() {
	create_sectors();

	// selected tri hash
	selected_tris = hashmapCreate(sel_tri_hash_init_size, SectorTri_hash_func, SectorTri_equals_func);
}

void grid_terrain_destroy() {
	// free all sel_tri entries
	HashmapIter iter;
	hashmapIter(selected_tris, &iter);
	while(!iter.end) {
		free(iter.value);
		hashmapNext(selected_tris, &iter);
	}
	// free hash
	hashmapFree(selected_tris);
	selected_tris = NULL;

	// TODO 
//	destroy_sectors();
}

void grid_terrain_update_cur_sector() {
	Real eye[3], cen[3], up[3];
	camera_get(eye, cen, up);
	Sector* sector = sector_from_pos(eye);
	if(sector) {
		if(sector != cur_sector)
			printf("set new cur sector: %d\n", sector->sector_num);
		cur_sector = sector;
	}
}

int grid_terrain_is_tri_selected(int sector_num, int tri_num) {
	SectorTri key = {sector_num, tri_num};
	SelectedTri* hash_st = (SelectedTri*) hashmapGet(selected_tris, &key);
	if(hash_st == NULL) 
		return 0;
	return hash_st->selected;
}

/*
static void draw_sector_num(Sector* sector) {
	char txt[256];
	sprintf(txt, "sector %d", sector->sector_num);
	glRasterPos3f(sector->pos[0], sector->pos[1] + sector_num_height, sector->pos[2]);
	fontutil_draw_string(txt);
}
*/

static void draw_sector(Sector* sector) {
	glEnable(GL_BLEND);

	Real alpha;
	Real yoff;
	float x0 = -(grid_unit * sector_num_cols()) / 2 + sector->pos[0];
	float y = sector->pos[1];
	float z = -(grid_unit * sector_num_rows()) / 2 + sector->pos[2];
	int tri_num = 0;
	int i;
	for(i = 0; i <= sector_num_rows(); ++i) {
		float x = x0;
		int j;
		for(j = 0; j <= sector_num_cols(); ++j) {
			GLenum draw_op;

			// first tri
			Real* tri_color = sector->color;
			int selected = grid_terrain_is_tri_selected(sector->sector_num, tri_num);
//if(selected) printf("*** draw selected tri %d.%d\n", sector->sector_num, tri_num);
			if(selected) {
				draw_op = GL_TRIANGLES;
				alpha = selected_tri_alpha;
				yoff = selected_tri_y_offset;
				tri_color = selected_color;
			}
			else {
//				draw_op = GL_LINE_LOOP;
				draw_op = _draw_op;
				alpha = 1.0;
				yoff = 0.0;
			}
			glColor4r(tri_color[0], tri_color[1], tri_color[2], alpha);
			glBegin(draw_op);
				glVertex3r(x, y + yoff, z);
				glVertex3r(x, y + yoff, z + grid_unit);
				glVertex3r(x+grid_unit, y + yoff, z + grid_unit);
				++tri_num;
			glEnd();

			// 2nd tri
			tri_color = sector->color;
			selected = grid_terrain_is_tri_selected(sector->sector_num, tri_num);
//if(selected) printf("*** draw selected tri %d.%d\n", sector->sector_num, tri_num);
			if(selected) {
				draw_op = GL_TRIANGLES;
				alpha = selected_tri_alpha;
				yoff = selected_tri_y_offset;
				tri_color = selected_color;
			}
			else {
//				draw_op = GL_LINE_LOOP;
				draw_op = _draw_op;
				alpha = 1.0;
				yoff = 0.0;
			}
			glColor4r(tri_color[0], tri_color[1], tri_color[2], alpha);
			glBegin(draw_op);
				glVertex3r(x, y + yoff, z);
				glVertex3r(x+grid_unit, y + yoff, z + grid_unit);
				glVertex3r(x+grid_unit, y + yoff, z);
				++tri_num;
			glEnd();
			grutil_inc_verts_drawn(6);
			x += grid_unit;

		}
		z += grid_unit;
	}

	glDisable(GL_BLEND);
//	draw_sector_num(sector);
}

void grid_terrain_draw(int num_adj_levels) {
	// draw current sector and N levels of adjacent nabors
	glDisable(GL_LIGHTING);

	draw_sector(cur_sector);

	// draw nabors
	int cur_sector_col = sector_col(cur_sector->sector_num);
	int cur_sector_row = sector_row(cur_sector->sector_num);
	int nabor_row;
	for(nabor_row = -num_adj_levels; nabor_row <= num_adj_levels; ++nabor_row) {
		int row = cur_sector_row + nabor_row;
		int nabor_col;
		for(nabor_col = -num_adj_levels; nabor_col <= num_adj_levels; ++nabor_col) {
			int col = cur_sector_col + nabor_col;
			Sector* sector = get_sector(row, col);	
			if(sector)
				draw_sector(sector);
		}
	}
	glEnable(GL_LIGHTING);
}

void grid_terrain_dump_selected_tris() {
	int i = 0;
	HashmapIter iter;
	hashmapIter(selected_tris, &iter);
	while(!iter.end) {
		SelectedTri* st = (SelectedTri*) iter.value;
		printf(" hash entry %02d: sector: %d, tri: %d, selected: %d\n", i++, st->key.sector_num, st->key.tri_num, st->selected);
		hashmapNext(selected_tris, &iter);
	}
}

Hashmap* grid_terrain_get_selected_tris() {
	return selected_tris;
}

