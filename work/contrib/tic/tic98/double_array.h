
typedef struct {
    double *ary;
    double low;
    double high;
    double binsize;
    int size;
} doublearraytype;

void da_init(doublearraytype *da,double low, double high, double binsize);
void da_free(doublearraytype *da);
void da_write(doublearraytype *da, const char filename[]);
void da_copy(doublearraytype *copy, doublearraytype *orig);
int    da_idx(doublearraytype *da,double val);
double da_val(doublearraytype *da,int p);

#define da_avg(ddd,p) ((da_val(ddd,p)+da_val(ddd,(p)+1))/2.0)
/*#define da_avg(ddd,p) da_val(ddd,p)*/
