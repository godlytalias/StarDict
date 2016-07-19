#ifndef __dictwidget_H__
#define __dictwidget_H__

#include <widget_app.h>
#include <widget_app_efl.h>
#include <Elementary.h>
#include <string>
#include <app_preference.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "dictwidget"

#if !defined(PACKAGE)
#define PACKAGE "org.widget.dict"
#endif
using namespace std;

typedef struct widget_instance_data {
	Evas_Object *win;
	Evas_Object *conform, *word_layout;
	char *word;
	Ecore_Timer* widget_update_timer;
    char edj_path[1024];
    double day;
    int rowid;
    Eina_Bool wod: 1;
} widget_instance_data_s;

#endif /* __dictwidget_H__ */

void _move_word_db(void);
void _set_word_used(int);
void _get_word_of_day(int func(void*,int,char**,char**), void*);
void _reset_word_usage(void);
