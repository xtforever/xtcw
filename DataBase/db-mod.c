#include <stdio.h>

/*

 */
struct db3_ctx {
    int types,     /* MARRAY OF INT */
        fields,    /* MARRAY OF MARRAY  Field-Data   */
        names;     /* MARRAY OF STRINGS Field-Names  */

    int conv;      /* MARRAY OF struct db3_converter */
    int cur_id;
    char *table_name;
};


int   db3_open(char *table);
void  db3_close(void);
void  db3_read(int h, int pos);
void  db3_new(int h);
int   db3_count(int h);
int   db3_current(int h);
void  db3_write(int h);

enum db3_typenum {
    DB3_INT, DB3_FLOAT, DB3_STRING, DB3_BLOB;
};

char *db3_types[] = {
    [DB3_INT] = "int",
    [DB3_FLOAT] = "float",
    [DB3_STRING] = "string",
    [DB3_BLOB] = "blob",

};


struct db3_converter;

/* returns: -1 on failure, 0 if successful */
typedef int (*converter_func) (struct db3_converter *conv, int in_m, int out_m );

enum converter_dir { FROM_STR, TO_STR };
struct db3_converter {
    enum db3_typenum typ;
    converter_func func[2];
    void *data;
};

/* copy field <num> as string - handle to/from buffer */
int db_entry( int h, int num, int buf_m, enum converter_dir dir )
{

    struct db3_ctx *db = get_ctx(h);
    int field_type = INT( db->types, num );
    struct db3_converter *conv = mls(db->conv, field_type );
    int field_buf = INT(DB->fields, num );
    conv->func[dir]( conv, field_buf, buf_m );
}

/* BLOB converter: entry to filename for blob */
