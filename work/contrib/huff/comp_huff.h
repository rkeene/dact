#ifndef _COMP_HUFF_H
#define _COMP_HUFF_H

typedef struct _huff_node huff_node;
typedef struct _huff_dict_entry huff_dict_entry;

struct _huff_node
{
        huff_node       *child;
        huff_node       *left_parent;
        huff_node       *right_parent;
        long long	count;
	unsigned char	value;
};




/* Am I on crack ? 256 ints to hold 256 BITS ??. Err no. I used to use 
   32 chars (256bits) for this and some bit manipulation but it was realllllly
   slow. Since this gets called a LOT it's not a big deal to use 1k of mem
   to hold a dictionary :)
*/
struct _huff_dict_entry
{
        unsigned char	bits;
	unsigned int	entry[256];		
};
/* comp_huff.c */
huff_node *new_huff_node(unsigned int count, huff_node *left, huff_node *right, unsigned char value);
huff_dict_entry *new_huff_dict_entry(void);
void huff_dict_write_bit(huff_dict_entry *to_mod, unsigned char bit_to_add);
void huff_write_out(huff_dict_entry *to_write, char *out_block, unsigned int *g);
int comp_huff_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
int huff_node_qsort_compare(huff_node **a, huff_node **b);
int comp_huff_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
int comp_huff_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
huff_node *huff_build_tree(int header[256], huff_node *tree_top[256], int blk_size);
void huff_build_dict(huff_node *root_node, huff_node *tree_top[256], huff_dict_entry *dictionary[256]);
void dict_free(huff_dict_entry *dictionary[256]);
void tree_free(huff_node *root);
/* comp_huff_adaptive.c */
int comp_huff_adaptive_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
int comp_huff_adaptive_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);
int comp_huff_adaptive_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size);

#endif

