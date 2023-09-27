#include <iup.h>
#include <iup_config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "helpers.h"

char *getcurrfeed(void){
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

void setmetadata(void){
	char *feed = getcurrfeed();

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed \"%s\" --metadata", feed);
	librarian(command);

	FILE *out = fopen("out", "r");

	char title[100];
	fgets(title, 100, out);

	Ihandle *feedtitle = IupGetHandle("feedtitle");
	IupSetStrAttribute(feedtitle, "TITLE", title); 

	char author[100];
	fgets(author, 100, out);

	Ihandle *feedauthor = IupGetHandle("feedauthor");
	IupSetStrAttribute(feedauthor, "TITLE", author);

	char hyperlink[100];
	fgets(hyperlink, 100, out);

	Ihandle *feedhyperlink = IupGetHandle("feedhyperlink");
	IupSetStrAttribute(feedhyperlink, "TITLE", hyperlink);
	IupSetStrAttribute(feedhyperlink, "URL", hyperlink);

	char published[100];
	fgets(published, 100, out);

	Ihandle *feedpubdate = IupGetHandle("feedpubdate");
	IupSetStrAttribute(feedpubdate, "TITLE", published);

	char updated[100];
	fgets(updated, 100, out);

	fclose(out);

	Ihandle *feedupdated = IupGetHandle("feedupdated");
	IupSetStrAttribute(feedupdated, "TITLE", updated);

	Ihandle *feedbox = IupGetHandle("feedbox");
	IupRefresh(feedbox);
}

void obscure(char *feed){
	Ihandle *tree = IupGetHandle("tree");

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed \"%s\" --metadata", feed);

	librarian(command);

	FILE *out = fopen("out", "r");

	char title[100];
	fgets(title, 100, out);

	fclose(out);
	
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

void setitem(int pos){
	Ihandle *tree = IupGetHandle("tree");

	char *feed = getcurrfeed();
	
	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed \"%s\" --item %d", feed, pos);
	librarian(command);

	FILE *out = fopen("out", "r");

	char title[100];
	fgets(title, 100, out);
	
	char url[500];
	fgets(url, 500, out);

	char pubdate[100];
	fgets(pubdate, 100, out);

	char update[100];
	fgets(update, 100, out);

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

	fclose(out);
}

void updatefeed(void){
	char *feed = getcurrfeed();

	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed \"%s\" -update", feed);	
	int exitcode = librarian(command);

	FILE *out = fopen("out", "r");

	char status[10];
	fgets(status, 10, out);

	fclose(out);

	if(exitcode == 2){
		IupMessageError(NULL, "Erro de rede.");
		return;
	} else if(exitcode == 3){	
		IupMessageError(NULL, "Erro ao ler documento XML.");
		return;
	}

	Ihandle *tree = IupGetHandle("tree");
	int selected = IupGetInt(tree, "VALUE");

	if(strstr(status, "true") != NULL)
		IupMessage("Notificação", "Feed atualizado");
	else
		IupMessage("Notificação", "Nada de novo por aqui...");
}	

void highlight(char *feedurl){
	char command[1000];
	snprintf(command, 1000, "librarian.exe --feed \"%s\" --metadata", feedurl);
	librarian(command);

	FILE *out = fopen("out", "r");

	char name[100];
	fgets(name, 100, out);

	fclose(out);

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

		char command[1000];
		while(feedtoken != NULL){
			snprintf(command, 1000, "librarian.exe --feed \"%s\" --update", feedtoken);
			int exitcode = librarian(command);

			FILE *out = fopen("out", "r");

			char result[10];
			fgets(result, 10, out);

			fclose(out);

			switch(exitcode){
				case 2:
				IupMessageError(NULL, "Erro de rede.");
				return;
				case 3:
				IupMessageError(NULL, "Erro ao ler documento XML");
				return;
			}

			if(strstr(result, "true") != NULL)
				highlight(feedtoken);
				

			feedtoken = strtok(NULL, ",");
		}

		free(feedcopy);
	}

	free(catcopy);
}

int librarian(char *command){
	FILE *fp = fopen("out", "r");
	fclose(fp);

	if(fp != NULL)
		remove("out");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitcode = 0;
	GetExitCodeProcess(pi.hProcess, &exitcode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return (int) exitcode;
}
