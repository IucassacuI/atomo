#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iup.h>
#include <iup_config.h>
#include <math.h>
#include "callbacks.h"
#include "helpers.h"

int opmlimport_cb(void){
	IupMessage("ACTION", "a implementar import de OPML...");

	return IUP_DEFAULT;
}

int opmlexport_cb(void){
	IupMessage("ACTION", "a implementar export de OPML...");

	return IUP_DEFAULT;
}

int exit_cb(void){
	return IUP_CLOSE;
}

int addcat_cb(void){
	Ihandle *tree = IupGetHandle("tree");
	Ihandle *config = IupGetHandle("config");

	char cat[CAT_LIMIT] = "N/A";
	const char *categories;

	int status = IupGetParam("Adicionar categoria", NULL, 0, "%s\n", &cat);

	if(status == 0 || strcmp(cat, "N/A") == 0)
		return IUP_DEFAULT;
	
	IupSetAttribute(tree, "ADDBRANCH0", cat);
	
	categories = IupConfigGetVariableStr(config, "CAT", "LIST");

	char new[CAT_LIMIT];
	snprintf(new, NEWFEED_LIMIT, "%s,", cat);
	
	if(!categories){
		IupConfigSetVariableStr(config, "CAT", "LIST", new);
		return IUP_DEFAULT;
	}

	char *list = calloc(strlen(categories)+CAT_LIMIT, sizeof(char));
	strncpy(list, categories, strlen(categories)+CAT_LIMIT-1);

	strncat(list, new, strlen(list));
	IupConfigSetVariableStr(config, "CAT", "LIST", list);

	free(list);

	return IUP_DEFAULT; 
}

int remocat_cb(void){	
	Ihandle *config = IupGetHandle("config");
	Ihandle *tree = IupGetHandle("tree");

	int selected = IupGetInt(tree, "VALUE");

	char kindattr[ATTR_LIMIT];
	snprintf(kindattr, ATTR_LIMIT, "KIND%d", selected);

	char titleattr[ATTR_LIMIT];
	snprintf(titleattr, ATTR_LIMIT, "TITLE%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);
	char *title = IupGetAttribute(tree, titleattr);

	if(strcmp(kind, "BRANCH") || selected == 0){
		IupMessageError(NULL, "Selecione uma categoria.");
		return IUP_DEFAULT;
	}

	int status = IupAlarm("Remover categoria", "Tem ceteza?", "Sim", "Não", NULL);

	if(status != 1)
		return IUP_DEFAULT;
	
	IupSetAttribute(tree, "DELNODE", "SELECTED");

	const char *categories = IupConfigGetVariableStr(config, "CAT", "LIST");

	char *copy = calloc(strlen(categories), sizeof(char));
	strncpy(copy, categories, strlen(categories));

	char *new = calloc(strlen(categories), sizeof(char));

	char *cattoken = strtok(copy, ",");

	char formatted[CAT_LIMIT];

	while(cattoken != NULL){
		snprintf(formatted, CAT_LIMIT, "%s,", cattoken);

		if(strcmp(title, cattoken))
			strncat(new, formatted, strlen(categories));

		cattoken = strtok(NULL, ",");
	}

	IupConfigSetVariableStr(config, "CAT", "LIST", new);
	IupConfigSetVariableStr(config, "CAT", title, "");

	free(copy);
	free(new);

	return IUP_DEFAULT;
}

int addfeed_cb(void){
	Ihandle *tree = IupGetHandle("tree");
	Ihandle *config = IupGetHandle("config");
	
	int selected = IupGetInt(tree, "VALUE");

	char kindattr[ATTR_LIMIT];
	snprintf(kindattr, ATTR_LIMIT, "KIND%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);

	if(strcmp(kind, "BRANCH") || selected == 0){
		IupMessageError(NULL, "Selecione uma categoria");
		return IUP_DEFAULT;
	}
	
	char url[URL_LIMIT] = "N/A";

	Ihandle *clipboard = IupClipboard();

	char *text = IupGetAttribute(clipboard, "TEXT");

	if(text != NULL && (strstr(text, "https://") != NULL || strstr(text, "http://") != NULL))
		strncpy(url, text, URL_LIMIT);

	IupDestroy(clipboard);

	int status = IupGetParam("Adicionar feed", NULL, 0, "%s\n", &url);

	if(status == 0)
		return IUP_DEFAULT;

	char command1[CMD_LIMIT];
	snprintf(command1, CMD_LIMIT, "librarian.exe --feed \"%s\" --update", url);	
	status = librarian(command1);

	switch(status){
	case 1:
		IupMessageError(NULL, "Feed inválido");
		return IUP_DEFAULT;
	case 2:
		IupMessageError(NULL, "Falha na conexão");
		return IUP_DEFAULT;
	case 3:
		IupMessageError(NULL, "Falha ao ler documento XML");
		return IUP_DEFAULT;
	case 4:
		IupMessageError(NULL, "Diretório inacessível");
		return IUP_DEFAULT;
	case 5:
		IupMessageError(NULL, "Falha de unmarshalling");
		return IUP_DEFAULT;
	}

	char titleattr[ATTR_LIMIT];
	snprintf(titleattr, ATTR_LIMIT, "TITLE%d", selected);

	char *category = IupGetAttribute(tree, titleattr);
	const char *list = IupConfigGetVariableStr(config, "CAT", category);

	char item[URL_LIMIT];
	snprintf(item, URL_LIMIT, "%s,", url);

	if(!list){
		IupConfigSetVariableStr(config, "CAT", category, item);
	} else {
		char *copy = calloc(strlen(list)+URL_LIMIT, sizeof(char));
		strncpy(copy, list, strlen(list)+URL_LIMIT);
		strncat(copy, item, strlen(list)+URL_LIMIT);

		IupConfigSetVariableStr(config, "CAT", category, copy);
		free(copy);
	}

	char command2[CMD_LIMIT];
	snprintf(command2, CMD_LIMIT, "librarian.exe --feed \"%s\" --metadata", url);
	librarian(command2);

	char leaf[ATTR_LIMIT];
	snprintf(leaf, TITLE_LIMIT, "ADDLEAF%d", selected);

	char title[TITLE_LIMIT];

	FILE *out = fopen("out", "r");
	fgets(title, TITLE_LIMIT, out);
	fclose(out);

	IupSetAttribute(tree, leaf, title);

	return IUP_DEFAULT;
}

 int remofeed_cb(void){
	Ihandle *tree = IupGetHandle("tree");
	Ihandle *config = IupGetHandle("config");

	int selected = IupGetInt(tree, "VALUE");

	char kindattr[50];
	snprintf(kindattr, ATTR_LIMIT, "KIND%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);

	if(strcmp(kind, "BRANCH") == 0){
		IupMessageError(NULL, "Selecione um feed");
		return IUP_DEFAULT;
	}

	char parentattr[ATTR_LIMIT];
	snprintf(parentattr, ATTR_LIMIT, "PARENT%d", selected);

	int catid = IupGetInt(tree, parentattr);

	char titleattr[ATTR_LIMIT];
	snprintf(titleattr, ATTR_LIMIT, "TITLE%d", catid);

	char *category = IupGetAttribute(tree, titleattr);
	const char *feeds = IupConfigGetVariableStr(config, "CAT", category);

	char *copy = calloc(strlen(feeds), sizeof(char));
	strncpy(copy, feeds, strlen(feeds));

	char *new = calloc(strlen(feeds), sizeof(char));

	char *token = strtok(copy, ",");
	char *currfeed = getcurrfeed();
	char feed[URL_LIMIT];

	while(token != NULL){
		if(strcmp(token, currfeed) != 0){
			snprintf(feed, URL_LIMIT, "%s,", token);
			strncat(new, feed, strlen(feeds));
		}
		token = strtok(NULL, ",");
	}

	IupConfigSetVariableStr(config, "CAT", category, new);

	IupSetAttribute(tree, "DELNODE", "SELECTED");

	char command[CMD_LIMIT];
	snprintf(command, CMD_LIMIT, "librarian.exe --feed %s --remove", currfeed);
	librarian(command);

	free(copy);
	free(new);
	return IUP_DEFAULT;
}

int feedselection_cb(Ihandle *h, int selected, int status){
	Ihandle *itembox = IupGetHandle("itembox");
	Ihandle *list = IupGetHandle("list");
	Ihandle *tree = IupGetHandle("tree");

	IupSetAttribute(list, "1", NULL);

	char kindattr[ATTR_LIMIT];
	snprintf(kindattr, ATTR_LIMIT, "KIND%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);

	if(strcmp(kind, "BRANCH") == 0 || status == 0)
		return IUP_DEFAULT;

	setmetadata();

	char *feed = getcurrfeed();

	obscure(feed);

	char command[CMD_LIMIT];
	snprintf(command, CMD_LIMIT, "librarian.exe --feed \"%s\" --items", feed);
	librarian(command);

	FILE *out = fopen("out", "r");

	char *item = malloc(TITLE_LIMIT);

	int counter = 0;
	char cstr[5];

	while(fgets(item, TITLE_LIMIT, out)){
		counter++;
		snprintf(cstr, 5, "%d", counter);

		IupSetAttribute(list, cstr, item);
		IupMap(list);
		IupRefresh(itembox);
	}
	free(item);
	fclose(out);	

	return IUP_DEFAULT;
}

int itemselection_cb(Ihandle *item, char* text, int pos, int state){
	if(state == 0)
		return IUP_DEFAULT;

	pos--;
	setitem(pos);
	
	return IUP_DEFAULT;
}

int rclick_cb(Ihandle *h, int id){
	Ihandle *tree = IupGetHandle("tree");
	IupSetInt(tree, "VALUE", id);

	char kindattr[ATTR_LIMIT];
	snprintf(kindattr, ATTR_LIMIT, "KIND%d", id);

	char kind[ATTR_LIMIT];
	snprintf(kind, ATTR_LIMIT, "%s", IupGetAttribute(tree, kindattr));

	if((strcmp(kind, "LEAF") != 0) && (id != 0))
		return IUP_DEFAULT;

	Ihandle *upitem;

	if(id == 0)
	        upitem = IupItem("Atualizar todos", NULL);
	else
		upitem = IupItem("Atualizar feed", NULL);

	Ihandle *menu = IupMenu(upitem, NULL);

	if(id == 0)
		IupSetCallback(upitem, "ACTION", (Icallback) updatefeeds);
	else
		IupSetCallback(upitem, "ACTION", (Icallback) updatefeed);

	IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
	feedselection_cb(NULL, id, 1);

	return IUP_DEFAULT;
}

int themes_cb(void){
	Ihandle *config = IupGetHandle("config");

	enum {def, dos, hollywood, blue} selected = IupConfigGetVariableInt(config, "THEME", "CURRENT");
	int status = IupGetParam("Temas", NULL, 0, "%l|(Padrão)|DOS|Hollywood|Azulado|\n", &selected);

	if(status == 0)
		return IUP_DEFAULT;

	IupConfigSetVariableInt(config, "THEME", "CURRENT", selected);

	char *binname = IupGetGlobal("EXEFILENAME");
	IupExecute(binname, NULL);

	return IUP_CLOSE;
}

int timer_cb(void){
	Ihandle *config = IupGetHandle("config");

	int interval = IupConfigGetVariableInt(config, "TIMER", "INTERVAL");
	int unity = IupConfigGetVariableInt(config, "TIMER", "UNITY");

	int status = IupGetParam("Temporizador", NULL, 0, "Atualizar a cada: %i[1,]\n%l|Minutos|Horas|\n", &interval, &unity);

	if((interval <= 0) || (status != 1))
		return IUP_DEFAULT;

	IupConfigSetVariableInt(config, "TIMER", "INTERVAL", interval);
	IupConfigSetVariableInt(config, "TIMER", "UNITY", unity);

	int time = (interval*1000)*pow(60, unity+1);

	Ihandle *old = IupGetHandle("timer");
	IupDestroy(old);
	
	Ihandle *timer = IupTimer();
	IupSetHandle("timer", timer);
	IupSetCallback(timer, "ACTION_CB", (Icallback) updatefeeds);
	IupSetInt(timer, "TIME", time);
	IupSetAttribute(timer, "RUN", "YES");
	IupSetHandle("timer", timer);
	
	return IUP_DEFAULT;
}

int switch_cb(void){
	Ihandle *timer = IupGetHandle("timer");
	int time = IupGetInt(timer, "TIME");

	if(time == 0){
		IupMessage("Temporizador", "Ainda não definido");
		return IUP_DEFAULT;
	}

	int run = IupGetInt(timer, "RUN");

	run = !run;

	if(run == 1)
		IupMessage("Temporizador", "Ativado");
	else
		IupMessage("Temporizador", "Desativado");

	IupSetInt(timer, "RUN", run);

	return IUP_DEFAULT;
}
