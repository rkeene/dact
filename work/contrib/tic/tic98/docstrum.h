#ifndef __DOCSTRUM_H
#define __DOCSTRUM_H

#include "marklist.h"


#define DEFAULT_K        5
#define DEFAULT_MIN_AREA 0.5 /* mm^2 */

typedef struct {
    int num;
    double xd,yd;
} neighbourlisttype;

typedef struct {
    int num;
    double x,y;
    short num_neighs;
    int group;
    neighbourlisttype *neighs;
} neighbourtype;

int calc_docstrum(marktype *image, double *skew);
int calc_docstrum_list(marklistptr list, double *skew, int area);

double calc_docstrum_spacing(marklistptr list, 
			     neighbourtype *neighs,
			     double orientation,
			     double *spacing_within,
			     double *spacing_between);

void prune_neighbours_angle_distance(neighbourtype *neighs,
		      int number_marks,
		      double orientation,
		      double orient_threshold,
		      double lesser);

void prune_neighbours_distance(neighbourtype *neighs,
		      int number_marks,
		      double distance);


void cluster_into_groups(neighbourtype *neighs,const int number_marks,
			       int *numgroups);

int label_group(neighbourtype *neighs,
		 const int marknum,
		 const int currentgroup,
		 const int total);

void pack_groups(neighbourtype *neighs, int *num_groups, const int total);
int  dump_group(const neighbourtype *neighs,const int currentgroup,
		const int total);

void group_reset_classifications(marklistptr list);


void view_connections_groups(marktype image,
			neighbourtype *neighs,
			int number_marks,
			int number_groups);

marklistptr groups_to_marklist(marklistptr list,
			       neighbourtype *neighs,
			       int number_marks,
			       int number_groups,
			       double skew,
			       double within);

marklistptr docstrum_order(marklistptr list);

#endif
