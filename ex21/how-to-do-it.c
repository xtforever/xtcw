#if 0
/* create the gui
   PLAN:

a   []adam [x]amon []tola []samson                checkbox-list
b   (x)2.0  ()2.1                                 radiobox-list

c    UP   [1 a-capacity check]                   string-list
          [2 a-usb programming]
    ||    [3 a-deep discharge protection]
          [4 a-cmdline interface]
    DN    [                     ]

d   [1]
e   [a-capcitiy check] short key
f   []adam []amon [x]tola [x]samson
g   (x)2.0  ()2.1

h  [ DESC ] long description
i  [ save ]


how.wcChildren: xa
*xa.wcClass: VBox
*xa.wcChildren: a b c d e f g h i

*xa.a.wcClass: Checkbox
*xa.a.strings: adam amon tola samson
*xa.b.data: filter_version
*xa.a.callback: filter_component

*xa.b.wcClass: Radiobox
*xa.b.strings: 2.0 2.1
*xa.b.data: filter_version
*xa.b.callback: filter_version

*xa.c.wcClass: Stringlist
*xa.c.data: requirement_names
*xa.c.callback: requirement_select

*xa.d.wcClass: Label
*xa.d.data: req_id

*xa.e.wcClass: Lineinput
*xa.e.data: req_name

*xa.f.wcClass: Checkbox
*xa.f.strings: adam amon tola samson
*xa.f.data:    req_components

*xa.g.wcClass: Radiobox
*xa.g.strings: 2.0 2.1
*xa.g.callback: req_versions

*xa.h.wcClass: Textbox
*xa.h.data:    req_desc

*xa.i.wcClass: Button
*xa.i.label:   save requirement
*xa.i.callback: requirement_save


/* callbacks */
requirement_save()
requirement_select()
filter_component()
filter_version()

/* variables */
req_desc : String
req_versions: Stringlist
req_components: Stringlist
req_name: String
req_id: Int32

/* MAIN */
char *options="f:rcfile";
Widget main = gui_init(argc,argv,options);
add_callback( main, requirement_save );
add_callback( main, requirement_select );
add_callback( main, filter_component );
add_callback( main, filter_version );




/* Datenstrukturen

   Wenn ein Widget Daten aus einer "Variablen"
   auslesen möchte, werden folgende Aktionen
   notwendig:

   Umwandeln des Variablen-Namens in einen Quark (int)
     int q = XStringToQuark("name");

   Zugeordnete Variable finden
     int v = *mv_var(q);

   Wert aus der Varibalen auslesen
     char *val = v_kget(v, 1);

   ----------------------------------------------------
   *mv_var/v_kget:

   INPUT    name: Name der Variablen, ein String
   OUTPUT   val : Erster Eintrag der zugeordneten
                  Stringliste

   q = XStringToQuark(name)
   p = MV.map[q]              Hashmap lookup
   i = MV.data                Verweis auf Array
                              für Daten für Variable
   A = ML[i]                  ML globales Array
   e = A[p]                   Eintrag der Variablen,
                              Daten des Signal-Handlers
   v = e->data                Index der Daten zur Var.

   SL = ML[v]                 Stringlist der Var.
   str = SL[1]                Erster Eintrag

*/



   Die Globale Haschmap(int->int) "MV" wird vom Modul
   "micro_vars" verwaltet.
   Strings werden durch die Funktion "XStringToQuark"
   in "int" umgewandelt.






 */
#endif


int GLOBALS;
int requirement_names;
int requirement_index;
int q_requirement_names;
int REQ = 0;
char *var_names[] = { "req_id",
                      "req_name",
                      "req_desc",
                      "req_versions",
                      "req_components" };

load_requirements() {
    REQ = read_file( vg_string("rcfile"), REQ_DESC );
}

/* copy from database to local memory */
requirement_select(Widget w, void *u, void *c)
{
    int select_line = (int)c;
    int rec = INT(REQ, select_line);
    int p; char **str;
    m_foreach( REQ_DESC, p, str )
        v_copy( 0, *str,  rec,  *str );
}

/* copy from local memory to database */
requirement_save(Widget w, void *u, void *c)
{
    int p; char **str;
    int rec = vg_int("req_id");
    int rec = INT(REQ, select_line);
    m_foreach( REQ_DESC, p, str )
        v_copy( rec, *str, 0, *str );
}


/* erstelle eine stringliste
   "requirement_names"
   aus dem Feld "req_name" in der
   Datenbank "REQ"
   Zudem wird ein Index der verwendeten
   Records der Datenbank erstellt.

   rq_names = select req_name from REQ where
              req_version is in $VERSION_LIST

*/
void rq_filter( int k_filter, char *field )
{
    int *rec, p;
    v_kclr(requirement_names);
    m_clear(requirement_index);

    /* ein string aus filter_list muss auch im feld
       req_version enthalten sein */
    m_foreach( REQ, p, rec ) {
        char *str  = v_get( *rec, field, 1 );
        char *name = v_get( *rec, "req_name",1 );
        if( v_contains(k_filter, str) > 0 ) {
                v_kset(requirement_names, name, -1 );
                m_put(requirement_index, &p   );
        }
    }
}

/* show only database entries filter by string |c| */
void filter_version(Widget w, void *u, void *c)
{
    int k_filter = (int) c;
    rq_filter( k_filter, "req_version" );
    mv_write( q_requirement_names, requirement_names );
}

/* show only database entries filter by string |c| */
void filter_component(Widget w, void *u, void *c)
{
    int k_filter = (int) c;
    rq_filter( k_filter, "req_components" );
    mv_write( q_requirement_names, requirement_names );
}


void init_variables(void)
{
    /* GLOBAL GUI STATE */
    GLOBALS = v_init();

    /* convert var_names to mstringlist */
    RQ_FIELDS = m_create(10,sizeof(char*));
    m_write(RQ_FIELDS, 0, var_names, ALEN(var_names) );

    requirement_index = m_create(10,sizeof(int));

    requirement_names =
        v_lookup(GLOBALS, "requirement_names" );
    q_requirement_names = XStringToQuark("requirement_names");
    *mv_var(q_requirement_names) = requirement_names;

    { /* create variables for each field
         assign variables to global variables (rcs)
       */
        int p; char **str;
        m_foreach(RQ_FIELDS, p, str ) {
            int key = v_lookup(GLOBALS,*str);
            int q = XrmStringToQuark(*str);
            *mv_var( q ) = key;
        }
    }
}
