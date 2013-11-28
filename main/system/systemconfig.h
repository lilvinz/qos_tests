#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H


#define SYSTEMCONFIG_STORED_SETTING _settings_list_ram[SETTINGS_SYS_SYSTEMTYPE]

typedef enum systemtype_t
{
    SYSTYPE_SINGLE          = 10,
    SYSTYPE_SINGLE_VISI     = 11,
//  SYSTYPE_MASTER          = 20,
//  SYSTYPE_SLAVE           = 30,
//  SYSTYPE_SLAVE_VISI      = 31,
    SYSTYPE_MESHNODE        = 40,
    SYSTYPE_MESHNODE_VISI   = 41
} SYSTEMTYPE_T;

void systemconfig_update_visibility(void);
void systemconfig_update_meshnode(void);

#endif
