#include <string.h>
#include <ctype.h>
#include "callbacks.h"

Ihandle *menu;
Ihandle *file_submenu, *fitems, *opmlimport_item, *opmlexport_item, *exit_item;
Ihandle *edit_submenu, *eitems, *addcat_item, *remocat_item, *addfeed_item, *remofeed_item;
Ihandle *options_submenu, *oitems, *themes_item, *timer_item, *switch_item;

void drawmenu(void){
	Ihandle *dialog = IupGetHandle("dialog");
	
	opmlimport_item = IupItem("Importar OPML...", NULL);
	opmlexport_item = IupItem("Exportar OPML...", NULL);
	exit_item = IupItem("Sair", NULL);

	IupSetCallback(opmlimport_item, "ACTION", (Icallback) opmlimport_cb);
	IupSetCallback(opmlexport_item, "ACTION", (Icallback) opmlexport_cb);
	IupSetCallback(exit_item, "ACTION", (Icallback) exit_cb);

	fitems = IupMenu(opmlimport_item, opmlexport_item, exit_item, NULL);
	file_submenu = IupSubmenu("Arquivo", fitems);

	addfeed_item = IupItem("Adicionar feed...", NULL);
	remofeed_item = IupItem("Remover feed", NULL);
	addcat_item = IupItem("Adicionar categoria...", NULL);
	remocat_item = IupItem("Remover categoria", NULL);

	IupSetCallback(addfeed_item, "ACTION", (Icallback) addfeed_cb);
	IupSetCallback(remofeed_item, "ACTION", (Icallback) remofeed_cb);
	IupSetCallback(addcat_item, "ACTION", (Icallback) addcat_cb);
	IupSetCallback(remocat_item, "ACTION", (Icallback) remocat_cb);

	eitems = IupMenu(addcat_item, remocat_item, addfeed_item, remofeed_item, NULL);
	edit_submenu = IupSubmenu("Editar", eitems);

	themes_item = IupItem("Tema...", NULL);
	timer_item = IupItem("Temporizador", NULL);
	switch_item = IupItem("Ativar/desativar temporizador", NULL);
	
	IupSetCallback(themes_item, "ACTION", (Icallback) themes_cb);
	IupSetCallback(timer_item, "ACTION", (Icallback) timer_cb);
	IupSetCallback(switch_item, "ACTION", (Icallback) switch_cb);
	
	oitems = IupMenu(themes_item, timer_item, switch_item, NULL);
	options_submenu = IupSubmenu("Opções", oitems);

	menu = IupMenu(file_submenu, edit_submenu, options_submenu, NULL);

	IupSetAttributeHandle(dialog, "MENU", menu);
}

void inittree(void){
	Ihandle *tree = IupTree();
	IupSetAttribute(tree, "MAXSIZE", "150x");
	IupSetCallback(tree, "RIGHTCLICK_CB",(Icallback) rclick_cb);
	IupSetHandle("tree", tree);
}

void inititembox(void){
	Ihandle *label = IupLabel("Item: ");

	Ihandle *list = IupList(NULL);
	IupSetAttributes(list, "DROPDOWN=YES, SIZE=150");
	IupSetCallback(list, "ACTION", (Icallback) itemselection_cb);
	IupSetHandle("list", list);

	Ihandle *itembox = IupHbox(label, list, NULL);
	IupSetHandle("itembox", itembox);
}

void initentrybox(void){
	Ihandle *title = IupLabel("N/A");
	Ihandle *tlabel = IupLabel("Título: ");

	Ihandle *pubdate = IupLabel("N/A");
	Ihandle *plabel = IupLabel("Publicado: ");

	Ihandle *update = IupLabel("N/A");
	Ihandle *ulabel = IupLabel("Atualizado: ");

	Ihandle *hyperlink = IupLink("example.com", "N/A");
	Ihandle *hlabel = IupLabel("Hyperlink: ");

	Ihandle *titlebox = IupHbox(tlabel, title, NULL);
	Ihandle *pubbox = IupHbox(plabel, pubdate, NULL);
	Ihandle *upbox = IupHbox(ulabel, update, NULL);
	Ihandle *hyperbox = IupHbox(hlabel, hyperlink, NULL);

	Ihandle *fill = IupFill();
	IupSetAttribute(fill, "SIZE", "5");
	
	Ihandle *box = IupVbox(fill, titlebox, pubbox, upbox, hyperbox, NULL);

	IupSetHandle("entrytitle", title);
	IupSetHandle("entrypubdate", pubdate);
	IupSetHandle("entryupdate", update);
	IupSetHandle("entryhyperlink", hyperlink);
	IupSetHandle("entrybox", box);
}

void drawtree(void){
	Ihandle *config = IupGetHandle("config");
	Ihandle *tree = IupGetHandle("tree");

	IupSetCallback(tree, "SELECTION_CB", (Icallback) feedselection_cb);

	IupSetAttribute(tree, "TITLE", "Feeds");

	const char *list = IupConfigGetVariableStr(config, "CAT", "LIST");

	if(!list){
		return;
	}

	char *catcopy = calloc(strlen(list), sizeof(char));
	strncpy(catcopy, list, strlen(list));
	catcopy[strlen(list)] = 0;

	int commacount = 0;
	for(int i = 0; i <= strlen(catcopy); i++)
		commacount += catcopy[i] == ',';

	char *cattoken = strtok(catcopy, ",");
	char *catlist[commacount];

	int counter = 0;

	while(cattoken != NULL){
		catlist[counter] = cattoken;
		cattoken = strtok(NULL, ",");
		counter++;
	}

	for(int i = 0; i < counter; i++){
		IupSetAttribute(tree, "ADDBRANCH0", catlist[i]);

		const char *feedlist = IupConfigGetVariableStr(config, "CAT", catlist[i]);
		if(feedlist == NULL)
			return;

		char *feedcopy = calloc(strlen(feedlist), sizeof(char));
		strncpy(feedcopy, feedlist, strlen(feedlist));

		char *feedtoken = strtok(feedcopy, ",");
		while(feedtoken != NULL){
			char command[1000];
			snprintf(command, 1000, "librarian.exe --feed %s --metadata", feedtoken);

			char title[100];
			
			FILE *pipe = popen(command, "r");
			if(pipe == NULL){
				free(feedcopy);
				IupMessageError(NULL, "Null pipe");
				return;
			}

			fgets(title, 100, pipe);
			pclose(pipe);

			IupSetAttribute(tree, "ADDLEAF1", title);

			feedtoken = strtok(NULL, ",");
		}

		free(feedcopy);
	}

	free(catcopy);
}

void initfeedbox(void){
	Ihandle *tlabel = IupLabel("Feed: ");
	Ihandle *title = IupLabel("N/A");

	Ihandle *alabel = IupLabel("De: ");
	Ihandle *author = IupLabel("N/A");

	Ihandle *plabel = IupLabel("Publicado: ");
	Ihandle *published = IupLabel("N/A");

	Ihandle *ulabel = IupLabel("Atualizado: ");
	Ihandle *updated = IupLabel("N/A");

	Ihandle *thbox = IupHbox(tlabel, title, NULL);
	Ihandle *chbox = IupHbox(alabel, author, NULL);
	Ihandle *phbox = IupHbox(plabel, published, NULL);
	Ihandle *uhbox = IupHbox(ulabel, updated, NULL);

	Ihandle *box = IupVbox(thbox, chbox, phbox, uhbox, NULL);
		
	IupSetHandle("feedtitle", title);
	IupSetHandle("feedauthor", author);
	IupSetHandle("feedpubdate", published);
	IupSetHandle("feedupdated", updated);
	IupSetHandle("feedbox", box);
}

void settheme(void){
	Ihandle *config = IupGetHandle("config");

	const char *theme = IupConfigGetVariableStr(config, "THEME", "CURRENT");

	if(theme == NULL)
		theme = "DEFAULT";

	char *dbgcolor;
	char *tbgcolor;
	char *tfgcolor;
	char *font;

	if(strcmp(theme, "DOS") == 0){
		dbgcolor = "0 0 0";
		tfgcolor = "255 255 255";
		tbgcolor = "0 0 0";
		font = "Consolas";
	} else if(strcmp(theme, "HOLLYWOOD") == 0){
		dbgcolor = "0 0 0";
		tfgcolor = "0 255 0";
		tbgcolor = "0 0 0";
		font = "Courier";
	} else if(strcmp(theme, "DEFAULT") == 0){
		dbgcolor = "255 255 255";
		tfgcolor = "0 0 0";
		tbgcolor = "255 255 255";
		font = "Arial";
	} else if(strcmp(theme, "BLUE") == 0){
		dbgcolor = "#1D2B41";
		tbgcolor = "#1D2B41";
		tfgcolor = "#6894AE";
		font = "Calibri";
	}

	IupSetGlobal("DLGFGCOLOR", tfgcolor);
	IupSetGlobal("DLGBGCOLOR", dbgcolor);
	IupSetGlobal("TXTBGCOLOR", tbgcolor);
	IupSetGlobal("TXTFGCOLOR", tfgcolor);
	IupSetGlobal("DEFAULTFONTFACE", font);
}
