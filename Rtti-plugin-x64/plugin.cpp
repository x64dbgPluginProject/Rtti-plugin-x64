#include "plugin.h"
#include "Lib\ini.h"
#include "config.h"
#include "Rtti.h"


PLUG_EXPORT void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {
    case MENU_AUTO_LABEL_VFTABLE:
		settings.auto_label_vftable = !settings.auto_label_vftable;
		SaveConfig();
        break;

    case MENU_DUMP_RTTI:
		DumpRtti(GUI_DUMP);
        break;

    default:
        break;
    }
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
    return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
}

//Do GUI/Menu related things here.
void pluginSetup()
{
	SetConfigPath();
	LoadConfig();

    int labelMenu = _plugin_menuadd(hMenu, "Auto-Label");
	_plugin_menuaddentry(labelMenu, MENU_AUTO_LABEL_VFTABLE, "vftable");
    _plugin_menuaddentry(hMenuDump, MENU_DUMP_RTTI, "&Dump Rtti");

	// Update the checked status
	_plugin_menuentrysetchecked(pluginHandle, MENU_AUTO_LABEL_VFTABLE, settings.auto_label_vftable);
}
