#ifndef SCRIPTS_H
#define SCRIPTS_H

typedef enum SCRIPT_WINDOW_TYPE {
    PRE_INSTALL_SCRIPT,
    POST_INSTALL_SCRIPT    
} SCRIPT_WINDOW_TYPE;

void new_script_window_with_title(SCRIPT_WINDOW_TYPE type);
void setup_scripts_window_items();

#endif