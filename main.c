#include <iup.h>
#include <iup_config.h>
#include <stdlib.h>
#include "ui.h"

Ihandle *hbox, *dialog;
Ihandle *config;

int main(int argc, char **argv){
	IupOpen(&argc, &argv);

	IupSetGlobal("UTF8MODE", "YES");

	IupSetLanguage("PORTUGUESE");

	config = IupConfig();
	IupSetAttribute(config, "APP_NAME", "Atomo");
	IupConfigLoad(config);
	IupSetHandle("config", config);

	int interval = IupConfigGetVariableInt(config, "TIMER", "INTERVAL");
	int unity = IupConfigGetVariableInt(config, "TIMER", "UNITY");

	int time = (interval*1000)*pow(60, unity+1);

	Ihandle *timer = IupTimer();
	IupSetInt(timer, "TIME", time);
	IupSetCallback(timer, "ACTION_CB", (Icallback) updatefeeds);
	IupSetAttribute(timer, "RUN", "YES");
	IupSetHandle("timer", timer);

	settheme();

	inittree();
	Ihandle *tree = IupGetHandle("tree");

	initfeedbox();
	Ihandle *feedbox = IupGetHandle("feedbox");

	inititembox();
	Ihandle *itembox = IupGetHandle("itembox");

	initentrybox();
	Ihandle *entrybox = IupGetHandle("entrybox");

	Ihandle *filler1 = IupFill();
	IupSetAttribute(filler1, "SIZE", "5");

	Ihandle *filler2 = IupFill();
	IupSetAttribute(filler2, "SIZE", "5");

	Ihandle *filler3 = IupFill();
	IupSetAttribute(filler3, "SIZE", "5");

	Ihandle *sep = IupFlatSeparator();
	IupSetAttribute(sep, "ORIENTATION", "HORIZONTAL");
	IupSetAttribute(sep, "EXPAND", "HORIZONTAL");

	Ihandle *vbox = IupVbox(filler1, feedbox, filler2, sep, filler3, itembox, entrybox, NULL);

	Ihandle *filler4 = IupFill();
	IupSetAttribute(filler4, "SIZE", "10");

	Ihandle *container = IupHbox(filler4, vbox, NULL);
	hbox = IupHbox(tree, container, NULL);
	dialog = IupDialog(hbox);

	IupSetAttributes(dialog, "SIZE=HALFxHALF, TITLE=Atomo");
	IupSetHandle("dialog", dialog);

	drawmenu();

	IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

     	drawtree();

	IupMainLoop();

	IupConfigSave(config);
	IupDestroy(config);

	IupDestroy(timer);

	IupClose();
	return EXIT_SUCCESS;
}
