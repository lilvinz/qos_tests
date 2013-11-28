
#include "stdheader.h"
#include "systemconfig.h"

#include "settings.h"
#include "visibility.h"
#include "mesh.h"


void systemconfig_update_visibility(void)
{
    SYSTEMTYPE_T systemtype_new = SYSTEMCONFIG_STORED_SETTING;;

    switch (SYSTEMCONFIG_STORED_SETTING)
    {
        // Single transformations
    case SYSTYPE_SINGLE:
        systemtype_new = SYSTYPE_SINGLE_VISI;
        break;
        // Meshnetwork transformations
    case SYSTYPE_MESHNODE:
        systemtype_new = SYSTYPE_MESHNODE_VISI;
        break;
    case SYSTYPE_MESHNODE_VISI:
        // this is the highest state of evolution
        break;
    }

    if ((uint32_t) systemtype_new != SYSTEMCONFIG_STORED_SETTING)
    {
        SYSTEMCONFIG_STORED_SETTING = (uint32_t)systemtype_new;
        SETTINGS_Save(SETTINGS_SYS_SYSTEMTYPE);
    }
}


void systemconfig_update_meshnode(void)
{
    SYSTEMTYPE_T systemtype_new = SYSTEMCONFIG_STORED_SETTING;

    switch (SYSTEMCONFIG_STORED_SETTING)
    {
        // Single transformations
    case SYSTYPE_SINGLE:
        systemtype_new = SYSTYPE_MESHNODE;
        break;
    case SYSTYPE_SINGLE_VISI:
        systemtype_new = SYSTYPE_MESHNODE_VISI;
        break;
    case SYSTYPE_MESHNODE_VISI:
        // this is the highest state of evolution
        break;
    }

    if ((uint32_t) systemtype_new != SYSTEMCONFIG_STORED_SETTING)
    {
        SYSTEMCONFIG_STORED_SETTING = (uint32_t)systemtype_new;
        SETTINGS_Save(SETTINGS_SYS_SYSTEMTYPE);
    }
}
