#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

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

	char cat[50] = "N/A";
	const char *categories;

	int status = IupGetParam("Adicionar categoria", NULL, 0, "%s\n", &cat);

	if(status == 0 || strcmp(cat, "N/A") == 0)
		return IUP_DEFAULT;
	
	IupSetAttribute(tree, "ADDBRANCH0", cat);
	
	categories = IupConfigGetVariableStr(config, "CAT", "LIST");

	char new[50];
	snprintf(new, 50, "%s,", cat);
	
	if(!categories){
		IupConfigSetVariableStr(config, "CAT", "LIST", new);
		return IUP_DEFAULT;
	}

	char *list = calloc(strlen(categories)+50, sizeof(char));
	strncpy(list, categories, strlen(categories)+50);

	strncat(list, new, strlen(list));
	IupConfigSetVariableStr(config, "CAT", "LIST", list);

	free(list);

	return IUP_DEFAULT; 
}

int remocat_cb(void){
	
	Ihandle *config = IupGetHandle("config");
	Ihandle *tree = IupGetHandle("tree");

	int selected = IupGetInt(tree, "VALUE");

	char kindattr[50];
	snprintf(kindattr, 50, "KIND%d", selected);

	char titleattr[50];
	snprintf(titleattr, 50, "TITLE%d", selected);

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

	char formatted[50];

	while(cattoken != NULL && isalpha(cattoken[0])){
		snprintf(formatted, 50, "%s,", cattoken);

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

	char kindattr[50];
	snprintf(kindattr, 50, "KIND%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);

	if(strcmp(kind, "BRANCH") || selected == 0){
		IupMessageError(NULL, "Selecione uma categoria");
		return IUP_DEFAULT;
	}
	
	char url[500] = "N/A";

	int status = IupGetParam("Adicionar feed", NULL, 0, "%s\n", &url);

	if(status == 0)
		return IUP_DEFAULT;
	char command1[1000];
	snprintf(command1, 1000, "librarian.exe --feed %s --update", url);
	
	status = system(command1);

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

	char titleattr[50];
	snprintf(titleattr, 50, "TITLE%d", selected);

	char *category = IupGetAttribute(tree, titleattr);

	const char *list = IupConfigGetVariableStr(config, "CAT", category);

	char item[500];
	snprintf(item, 500, "%s,", url);

	if(!list){
		IupConfigSetVariableStr(config, "CAT", category, item);
	} else {
		char *copy = calloc(strlen(list)+500, sizeof(char));
		strncpy(copy, list, strlen(list)+500);
		strncat(copy, item, strlen(list)+500);

		IupConfigSetVariableStr(config, "CAT", category, copy);
		free(copy);
	}

	char command2[1000];
	snprintf(command2, 1000, "librarian.exe --feed %s --metadata", url);

	char leaf[50];
	snprintf(leaf, 50, "ADDLEAF%d", selected);

	char title[100];

	FILE *pipe = popen(command2, "r");

	if(pipe == NULL){
		IupMessageError(NULL, "Cano nulo.");
		return IUP_DEFAULT;
	}
	
	fgets(title, 100, pipe);
	pclose(pipe);

	IupSetAttribute(tree, leaf, title);

	return IUP_DEFAULT;
}

static char *getcurrfeed(void){
	Ihandle *tree = IupGetHandle("tree");
	Ihandle *config = IupGetHandle("config");

	int selected = IupGetInt(tree, "VALUE");

	char parentattr[50];
	snprintf(parentattr, 50, "PARENT%d", selected);
	int parentid = IupGetInt(tree, parentattr);
 
	selected -= parentid;
 
	char titleattr[50];
	snprintf(titleattr, 50, "TITLE%d", parentid);

	char *cat = IupGetAttribute(tree, titleattr);

	const char *feeds = IupConfigGetVariableStr(config, "CAT", cat);

	char *copy = calloc(strlen(feeds), sizeof(char));
	strncpy(copy, feeds, strlen(feeds));

	int commacount = 0;

	for(int index = 0; index < strlen(copy); index++)
		commacount += copy[index] == ',';

	char *token = strtok(copy, ",");

	for(int cycle = 0; cycle < commacount-selected; cycle++)
		token = strtok(NULL, ",");

	return token;
}

int remofeed_cb(void){
	Ihandle *tree = IupGetHandle("tree");
	Ihandle *config = IupGetHandle("config");

	int selected = IupGetInt(tree, "VALUE");

	char kindattr[50];
	snprintf(kindattr, 50, "KIND%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);

	if(strcmp(kind, "BRANCH") == 0){
		IupMessageError(NULL, "Selecione um feed");
		return IUP_DEFAULT;
	}

	char delnode[50];
	snprintf(delnode, 50, "DELNODE%d", selected);
	IupSetAttribute(tree, delnode, "");

	char parentattr[50];
	snprintf(parentattr, 50, "PARENT%d", selected);

	int catid = IupGetInt(tree, parentattr);

	char titleattr[50];
	snprintf(titleattr, 50, "TITLE%d", catid);

	char *category = IupGetAttribute(tree, titleattr);
	const char *feeds = IupConfigGetVariableStr(config, "CAT", category);

	char *copy = calloc(strlen(feeds), sizeof(char));
	strncpy(copy, feeds, strlen(feeds));

	char *new = calloc(strlen(feeds), sizeof(char));

	char *token = strtok(copy, ",");
	char *currfeed = getcurrfeed();
	char feed[500];

	while(token != NULL){
		if(strcmp(token, currfeed) != 0){
			snprintf(feed, 500, "%s,", token);
			strncat(new, feed, strlen(feeds));
		}
		token = strtok(NULL, ",");
	}

	IupConfigSetVariableStr(config, "CAT", category, new);
	
	free(copy);
	free(new);
	return IUP_DEFAULT;
}

void setmetadata(void){
	char *feed = getcurrfeed();

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed %s --metadata", feed);

	FILE *pipe = popen(command, "r");
	if(pipe == NULL){
		IupMessageError(NULL, "null pipe");
		return;
	}

	char title[100];
	fgets(title, 100, pipe);

	Ihandle *feedtitle = IupGetHandle("feedtitle");
	IupSetStrAttribute(feedtitle, "TITLE", title); 

	char author[100];
	fgets(author, 100, pipe);

	Ihandle *feedauthor = IupGetHandle("feedauthor");
	IupSetStrAttribute(feedauthor, "TITLE", author);

	char hyperlink[100];
	fgets(hyperlink, 100, pipe);

	Ihandle *feedhyperlink = IupGetHandle("feedhyperlink");
	IupSetStrAttribute(feedhyperlink, "TITLE", hyperlink);
	IupSetStrAttribute(feedhyperlink, "URL", hyperlink);

	char published[100];
	fgets(published, 100, pipe);

	Ihandle *feedpubdate = IupGetHandle("feedpubdate");
	IupSetStrAttribute(feedpubdate, "TITLE", published);

	char updated[100];
	fgets(updated, 100, pipe);

	Ihandle *feedupdated = IupGetHandle("feedupdated");
	IupSetStrAttribute(feedupdated, "TITLE", updated);

	Ihandle *feedbox = IupGetHandle("feedbox");
	IupRefresh(feedbox);
}

void obscure(char *feed){
	Ihandle *tree = IupGetHandle("tree");

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed %s --metadata", feed);

	FILE *pipe = popen(command, "r");

	char title[100];
	fgets(title, 100, pipe);

	pclose(pipe);
	
	int nodes = IupGetInt(tree, "COUNT");

	char titleattr[50];
	char name[100];
	for(int node = 0; node < nodes; node++){
	snprintf(titleattr, 50, "TITLE%d", node);
	snprintf(name, 100, "%s", IupGetAttribute(tree, titleattr));

	if(!strcmp(title, name))
		nodes = node;
	}

	char colorattr[50];
	snprintf(colorattr, 50, "COLOR%d", nodes);
	char color[50];
	snprintf(color, 50, "%s", IupGetAttribute(tree, colorattr));

	if(!strcmp(color, "0 0 255"))
		IupSetAttribute(tree, colorattr, IupGetGlobal("DLGFGCOLOR"));
}

int feedselection_cb(Ihandle *h, int selected, int status){
	Ihandle *itembox = IupGetHandle("itembox");
	Ihandle *list = IupGetHandle("list");
	Ihandle *tree = IupGetHandle("tree");

	IupSetAttribute(list, "1", NULL);

	char kindattr[50];
	snprintf(kindattr, 50, "KIND%d", selected);

	char *kind = IupGetAttribute(tree, kindattr);

	if(strcmp(kind, "BRANCH") == 0 || status == 0)
		return IUP_DEFAULT;

	setmetadata();

	char *feed = getcurrfeed();

	obscure(feed);

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed %s --items", feed);

	FILE *pipe = popen(command, "r");

	if(pipe == NULL){
		IupMessageError(NULL, "null pipe");
		return IUP_DEFAULT;
	}

	char *item = malloc(1000);

	int counter = 0;
	char cstr[5];

	while(fgets(item, 1000, pipe)){
		counter++;
		snprintf(cstr, 5, "%d", counter);

		IupSetAttribute(list, cstr, item);
		IupMap(list);
		IupRefresh(itembox);
	}

	free(item);
	return IUP_DEFAULT;
}

void setitem(int pos){
	Ihandle *tree = IupGetHandle("tree");

	char *feed = getcurrfeed();
	
	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed %s --item %d", feed, pos);

	FILE *pipe = popen(command, "r");

	if(pipe == NULL){
		IupMessageError(NULL, "null pipe");
		return;
	}

	char title[100];
	fgets(title, 100, pipe);
	
	char url[500];
	fgets(url, 500, pipe);

	char pubdate[100];
	fgets(pubdate, 100, pipe);

	char update[100];
	fgets(update, 100, pipe);

	Ihandle *entrytitle = IupGetHandle("entrytitle");
	IupSetStrAttribute(entrytitle, "TITLE", title);
	
	Ihandle *hyperlink = IupGetHandle("entryhyperlink");
	IupSetStrAttribute(hyperlink, "TITLE", url);
	IupSetStrAttribute(hyperlink, "URL", url);

	Ihandle *entrypubdate = IupGetHandle("entrypubdate");
	IupSetStrAttribute(entrypubdate, "TITLE", pubdate);

	Ihandle *entryupdate = IupGetHandle("entryupdate");
	IupSetStrAttribute(entryupdate, "TITLE", update);

	Ihandle *entrybox = IupGetHandle("entrybox");
	IupRefresh(entrybox);

	pclose(pipe);
}

int itemselection_cb(Ihandle *item, char* text, int pos, int state){
	if(state == 0)
		return IUP_DEFAULT;

	pos--;
	setitem(pos);
	
	return IUP_DEFAULT;
}

void updatefeed(void){
	char *feed = getcurrfeed();

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed %s -update", feed);
	
	FILE *pipe = popen(command, "r");

	char status[10];
	fgets(status, 10, pipe);

	int exitcode = pclose(pipe);

	if(exitcode == 2){
		IupMessageError(NULL, "Erro de rede.");
		return;
	} else if(exitcode == 3){	
		IupMessageError(NULL, "Erro ao ler documento XML.");
		return;
	}

	Ihandle *tree = IupGetHandle("tree");
	int selected = IupGetInt(tree, "VALUE");

	if(strcmp(status, "true") == 0){
		IupMessage("Notificação", "Feed atualizado");
		feedselection_cb(NULL, selected, 1);
	} else
		IupMessage("Notificação", "Nada de novo por aqui...");
}	

int rclick_cb(Ihandle *h, int id){
	Ihandle *tree = IupGetHandle("tree");
	IupSetInt(tree, "VALUE", id);

	char kindattr[10];
	snprintf(kindattr, 10, "KIND%d", id);

	char kind[50];
	snprintf(kind, 50, "%s", IupGetAttribute(tree, kindattr));

	if(strcmp(kind, "LEAF") != 0)
		return IUP_DEFAULT;

	Ihandle *upitem = IupItem("Atualizar", NULL);
	Ihandle *menu = IupMenu(upitem, NULL);

	IupSetCallback(upitem, "ACTION", (Icallback) updatefeed);

	IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);


	return IUP_DEFAULT;

}

int themes_cb(void){

	enum {def, dos, hollywood, blue} selected;
	int status = IupGetParam("Temas", NULL, 0, "%l|(Padrão)|DOS|Hollywood|Azulado|\n", &selected);

	if(status == 0)
		return IUP_DEFAULT;

	Ihandle *config = IupGetHandle("config");
		
	switch(selected){
		case def:
			IupConfigSetVariableStr(config, "THEME", "CURRENT", "DEFAULT");
			break;
		case dos:
			IupConfigSetVariableStr(config, "THEME", "CURRENT", "DOS");
			break;
		case hollywood:
			IupConfigSetVariableStr(config, "THEME", "CURRENT", "HOLLYWOOD");
			break;
		case blue:
			IupConfigSetVariableStr(config, "THEME", "CURRENT", "BLUE");
			break;
			 }

			 char *binname = IupGetGlobal("EXEFILENAME");

			 char command[1000];
			 snprintf(command, 1000, "cmd /c start %s", binname);

			 system(command);

			 return IUP_CLOSE;
}

void highlight(char *feedurl){
	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed %s --metadata", feedurl);

	char name[100];
	FILE *pipe = popen(command, "r");
	fgets(name, 100, pipe);
	pclose(pipe);

	Ihandle *tree = IupGetHandle("tree");
	int nodes = IupGetInt(tree, "COUNT");

	char titleattr[50];
	char title[100];
	char colorattr[50];
	for(int node = 0; node < nodes; node++){
		snprintf(titleattr, 50, "TITLE%d", node);
		snprintf(title, 100, "%s", IupGetAttribute(tree, titleattr));
		snprintf(colorattr, 50, "COLOR%d", node);
		
		if(!strcmp(title, name))
			IupSetAttribute(tree, colorattr, "0 0 255");
	}
}

void updatefeeds(void){
	Ihandle *config = IupGetHandle("config");
	Ihandle *tree = IupGetHandle("tree");

	const char *cats = IupConfigGetVariableStr(config, "CAT", "LIST");
	char *catcopy = calloc(strlen(cats)+1, sizeof(char));

	strncpy(catcopy, cats, strlen(cats)+1);

	int commacount = 0;
	for(int chr = 0; chr <= strlen(catcopy); chr++)
		commacount += catcopy[chr] == ',';

	char *catlist[commacount];

	char *token = strtok(catcopy, ",");

	for(int cat = 0; cat < commacount; cat++){
		catlist[cat] = token;
		token = strtok(NULL, ",");
	}

	for(int cat = 0; cat < commacount; cat++){
		const char *feedlist = IupConfigGetVariableStr(config, "CAT", catlist[cat]);
		char *feedcopy = calloc(strlen(feedlist)+1, sizeof(char));
		strncpy(feedcopy, feedlist, strlen(feedlist)+1);

					char *feedtoken = strtok(feedcopy, ",");
		while(feedtoken != NULL){

			char command[1000];
			snprintf(command, 1000, "librarian.exe --feed %s --update", feedtoken);
			FILE *pipe = popen(command, "r");
			char result[10];
			fgets(result, 10, pipe);

			int exitcode = pclose(pipe);

			switch(exitcode){
				case 2:
				IupMessageError(NULL, "Erro de rede.");
				return;
				case 3:
				IupMessageError(NULL, "Erro ao ler documento XML");
				return;
			}
			
			if(!strcmp(result, "true"))
				highlight(feedtoken);

			feedtoken = strtok(NULL, ",");
		}

		free(feedcopy);
	}

	free(catcopy);
}

int timer_cb(void){
	Ihandle *config = IupGetHandle("config");

	int interval = IupConfigGetVariableInt(config, "TIMER", "INTERVAL");
	int unity = IupConfigGetVariableInt(config, "TIMER", "UNITY");
	bool initialize;

	int status = IupGetParam("Temporizador", NULL, 0, "Atualizar a cada: %i\n%l|Minutos|Horas|\nInit: %b\n", &interval, &unity, &initialize);

	if((interval <= 0) || (status != 1))
		return IUP_DEFAULT;

	if(initialize){
		IupConfigSetVariableInt(config, "TIMER", "INTERVAL", interval);
		IupConfigSetVariableInt(config, "TIMER", "UNITY", unity);
	}

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
