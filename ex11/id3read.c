#include "id3read.h"

typedef struct dent_st {
  char *name;
  int subdir;
  unsigned char md5[16];
  int size;
} dent_t;

static void get_file_props(char *path, void *cb)
{
    int db = *(int*)cb;
    id3db_add(db,path);
}

int scan_files( int list, const char *path, void (*get_file_props) (char*path,void *cb), void *cb )
{
  DIR *dir;
  struct dirent *entry;
  dent_t dent;
  /*  struct stat statbuf; */

  dir = opendir(path);
  if(!dir) return 0;

  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name,".") == 0)
      continue;
    if(strcmp(entry->d_name,"..") == 0)
      continue;

    if( entry->d_type == DT_DIR ) {
      dent.subdir=-1;
      dent.name = strdup(entry->d_name);
      m_put( list, &dent );
    }
    else
      if( entry->d_type == DT_REG )
        {
          char *buf;
          asprintf(&buf,"%s/%s", path, entry->d_name );
	  get_file_props( buf, cb );
	  free(buf);
	}
  }
  closedir(dir);
  return list;
}

void scan_dir( char *path, void (*get_file_props) (char*path,void *cb), void *cb )
{
    dent_t *d;
    char *s;
    int p, root = m_create(10, sizeof(dent_t));
    scan_files( root, path, get_file_props, cb );

    m_foreach(root,p,d) {
	if( d->subdir ) {
	    asprintf(&s, "%s/%s", path, d->name );
	    scan_dir( s,get_file_props,cb );
	    free(s);
	}
	if( d->name ) free(d->name);
    }
    m_free(root);
}

int id3db_create(void)
{
    return m_create(100,sizeof(struct id3ent));
}

int id3db_add( int db, char *path )
{
    struct id3ent id;
    id.vs = get_id3_tag(path);
    if(! id.vs ) return 1;

    id.path = strdup(path);
    m_put(db,&id);
    return 0;
}

void id3db_print(int db)
{
    struct id3ent *d;
    int p;
    m_foreach(db, p, d )
	{
            char *album = v_get(d->vs, "album", 1 );
            char *title = v_get(d->vs, "title", 1 );

            printf( "A: %s\n", album );
            printf( "T: %s\n", title );
            printf( "F: %s\n", d->path );

	}
}

void id3db_free(int db)
{
   struct id3ent *d;
   int p;
   m_foreach(db, p, d ) {
       free( d->path );
       v_free( d->vs );
   }
   m_free(db);
}

void id3db_import( int db, char *root )
{
    scan_dir( root, get_file_props, &db );
}


int get_id3_tag(char *path)
{
  TagLib_File *file;
  TagLib_Tag *tag;
  char track[50];

  taglib_set_strings_unicode(1);
  file = taglib_file_new(path);

  if(file == NULL)
      return 0;

  int var = 0;

  tag = taglib_file_tag(file);
  if(tag != NULL) {
      /*
      printf("-- TAG --\n");
      printf("title   - \"%s\"\n", taglib_tag_title(tag));
      printf("artist  - \"%s\"\n", taglib_tag_artist(tag));
      printf("album   - \"%s\"\n", taglib_tag_album(tag));
      printf("year    - \"%i\"\n", taglib_tag_year(tag));
      printf("comment - \"%s\"\n", taglib_tag_comment(tag));
      printf("track   - \"%i\"\n", taglib_tag_track(tag));
      printf("genre   - \"%s\"\n", taglib_tag_genre(tag));
      */

      var = v_init();
      v_set( var, "title", taglib_tag_title(tag), -1 );
      v_set( var, "artist", taglib_tag_artist(tag), -1 );
      v_set( var, "album", taglib_tag_album(tag), -1 );
      sprintf( track, "%u",  taglib_tag_track(tag) );
      v_set( var, "track", track, -1 );
  }

  /*

  int seconds;
  int minutes;
  const TagLib_AudioProperties *properties;
  properties = taglib_file_audioproperties(file);
  if(properties != NULL) {
      seconds = taglib_audioproperties_length(properties) % 60;
      minutes = (taglib_audioproperties_length(properties) - seconds) / 60;

      printf("-- AUDIO --\n");
      printf("bitrate     - %i\n", taglib_audioproperties_bitrate(properties));
      printf("sample rate - %i\n", taglib_audioproperties_samplerate(properties));
      printf("channels    - %i\n", taglib_audioproperties_channels(properties));
      printf("length      - %i:%02i\n", minutes, seconds);
    }
  */

  taglib_tag_free_strings();
  taglib_file_free(file);
  return var;
}
