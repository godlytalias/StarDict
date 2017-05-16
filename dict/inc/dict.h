#ifndef __dict_H__
#define __dict_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <string>
#include <app.h>
#include <net_connection.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <EWebKit.h>
#include <sqlite3.h>
#include <app_preference.h>
#include <wav_player.h>
#include "clsStarDict.h"
#include "flite.h"
#include <sound_manager.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "dict"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.dict"
#endif

#define EDJ_FILE "edje/dict.edj"
#define GRP_MAIN "main"

struct _history_list {
	char *url;
	struct _history_list *prev;
};

typedef struct appdata {
        clsSDict *StarDict;
        _history_list *url_list;
        char **pred;
        char *search_word;
        Elm_Genlist_Item_Class *itc;
        Elm_Object_Item **pred_item;
        Ecore_Idler *pred_idler, *showinidler;
        Evas_Object *win;
        Evas_Object *label;
        Evas_Object *layout, *app_layout;
        Evas_Object *entry;
        Evas_Object *button;
        Evas_Object *naviframe;
        Evas_Object *genlist;
        Evas_Object *title_toolbar;
        Evas_Object *lookup_list;
        Evas_Object *progress_bar;
        Evas_Object *ewk;
        Evas_Coord mouse_x, mouse_y;
        unsigned int mouse_down_time;
        int wav_id;
        Eina_Bool push_flag :1;
        Eina_Bool widget_launch :1;
} appdata_s;

#define GROUP_MAIN "main"
using namespace std;

void create_ctxpopup_more_button_cb(void *, Evas_Object *, void *);
void _show_result(void *, Evas_Object *, void *);
void _query_keywords(appdata_s *, int func(void*,int,char**,char**));
void _store_keyword(char *);
void _init_tables();
void _popup_del(void *, Evas_Object *, void *);
bool _check_internet(void);
void _clear_history_table(appdata_s *);
void app_get_resource(const char *, char *, int );
#endif /* __dict_H__ */
