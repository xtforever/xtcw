#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

void murphy_ready( int is_ready );
void murphy_log( char *s );

/* from GUI to MURPHY */
void murphy_set_param( int mix_air, int fg, int o2 );

/* joab to gui */
void  joab_ready( int is_ready );
void  focus_cmd(char *s);

/* alle werte von zorg aktualisieren */
void zorg_update_all(void);


extern XtAppContext APP_CON;


#endif
