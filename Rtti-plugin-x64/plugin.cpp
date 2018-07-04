#include "plugin.h"
#include "Lib\ini.h"
#include "Lib\config.h"
#include "Lib\Rtti.h"

#define RTTI_COMMAND "rtti"

// Get the current window selection, aligns it to 4 byte boundaries and dumps it
void DumpRttiWindow(int hWindow)
{
	if (!DbgIsDebugging())
	{
		dputs("You need to be debugging to use this command");
		return;
	}
	SELECTIONDATA sel;
	GuiSelectionGet(hWindow, &sel);
	duint alignedStart = sel.start - (sel.start % (sizeof duint));

	RTTI klass(alignedStart);
	klass.PrintVerbose();
}

// 'rtti <addr>' command
static bool cbRttiCommand(int argc, char* argv[])
{
	if (argc > 2)
	{
		dprintf("Usage: rtti <address>\n");
		return false;
	}

	// command 'rtti' - Assume the selected bytes from the dump window
	if (argc == 1)
	{
		DumpRttiWindow(GUI_DUMP);
	}
	// command 'rtti <address>'
	else if (argc == 2)
	{
		duint addr = 0;
		int numFieldsAssigned = sscanf_s(argv[1], "%x", &addr);

		if (numFieldsAssigned != 1)
		{
			dprintf("Usage: rtti <address>\n");
			return false;
		}

		RTTI klass(addr);
		
		if (klass.)
		klass.PrintVerbose();
	}

	return true;
}

PLUG_EXPORT void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {
    case MENU_AUTO_LABEL_VFTABLE:
		settings.auto_label_vftable = !settings.auto_label_vftable;
		SaveConfig();
        break;

    case MENU_DUMP_RTTI:
		DumpRttiWindow(GUI_DUMP);
        break;

    default:
        break;
    }
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	_plugin_registercommand(pluginHandle, RTTI_COMMAND, cbRttiCommand, true);
    return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
	_plugin_unregistercommand(pluginHandle, RTTI_COMMAND);
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
