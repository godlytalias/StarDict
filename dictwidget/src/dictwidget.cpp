#include <tizen.h>
#include <app.h>
#include "dictwidget.h"
#include "clsStarDict.h"

static void
_launch_stardict(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	app_control_h handler;
	app_control_create(&handler);
	app_control_set_app_id(handler, "org.tizen.dict");
	app_control_set_operation(handler, APP_CONTROL_OPERATION_VIEW);
	app_control_add_extra_data(handler, "launch_info", "widget_launch");
	app_control_send_launch_request(handler, NULL, NULL);
	app_control_destroy(handler);
}

static void
_launch_stardict_search(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	app_control_h handler;
	app_control_create(&handler);
	app_control_set_app_id(handler, "org.tizen.dict");
	app_control_set_operation(handler, APP_CONTROL_OPERATION_VIEW);
	app_control_add_extra_data(handler, "launch_info", "widget_launch");
	app_control_add_extra_data(handler, "search", wid->word);
	app_control_send_launch_request(handler, NULL, NULL);
	app_control_destroy(handler);
}

int
_store_wordstamp(void *data, int argc, char **argv, char **azColName)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->word = strdup(argv[0]);
	wid->rowid = atoi(argv[1]);
	preference_set_double("day", wid->day);
	preference_set_string("word", wid->word);
	return 0;
}

static char*
capitalize(char *string)
{
	int i = 0;
	while(string[i] != '\0') {
		string[i] = toupper(string[i]);
		i++;
	}
	return string;
}

static void
_query_word_of_day(widget_instance_data_s *wid, Eina_Bool force)
{
	double prev_day = -2.0;
	bool exist;
	wid->day = floor(ecore_time_unix_get() / 86400.0);
	preference_is_existing("word", &exist);
	if (exist) {
		preference_get_double("day", &prev_day);
		preference_get_string("word", &wid->word);
	}
	else force = EINA_TRUE;
	if (wid->day != prev_day)
	{
		if (wid->word)
		{
			free(wid->word);
			wid->word = NULL;
		}
		_get_word_of_day(&_store_wordstamp, (void*)wid);
		_set_word_used(wid->rowid);
		if (!wid->word) {
			_reset_word_usage();
			_get_word_of_day(&_store_wordstamp, (void*)wid);
			_set_word_used(wid->rowid);
		}
		force = EINA_TRUE;
	}
	if (force)
	{
		char *str;
		string result;
		clsSDict *StarDict;
		StarDict = new clsSDict;
		StarDict->loadDics("/opt/usr/apps/org.tizen.dict/shared/dict");
		result = StarDict->lookupWords(wid->word, false);
		delete StarDict;
		str = (char*)malloc(sizeof(char) * result.length());
		strcpy(str, result.c_str());
		elm_layout_text_set(wid->word_layout, "elm.text.word", capitalize(wid->word));
		elm_layout_text_set(wid->word_layout, "elm.text.definition", str);
		free(str);
	}
}

static int
widget_instance_create(widget_context_h context, bundle *content, int w, int h, void *user_data)
{
	widget_instance_data_s *wid = (widget_instance_data_s*) malloc(sizeof(widget_instance_data_s));
	int ret;

	if (content != NULL) {
		/* Recover the previous status with the bundle object. */

	}

	Evas_Object *layout;
	char *res_path = app_get_resource_path();
	sprintf(wid->edj_path, "%s/edje/widget.edj", res_path);
	free(res_path);
	_move_word_db();
	wid->day = -1.0;
	wid->word = NULL;
	wid->rowid = -1;
	wid->widget_update_timer = NULL;
	/* Window */
	ret = widget_app_get_elm_win(context, &wid->win);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return WIDGET_ERROR_FAULT;
	}

	evas_object_resize(wid->win, w, h);
	if (w == h)
		wid->wod = EINA_TRUE;
	else
		wid->wod = EINA_FALSE;

	wid->conform = elm_conformant_add(wid->win);
	elm_app_base_scale_set(1.8);
	evas_object_size_hint_weight_set(wid->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(wid->win, wid->conform);
	evas_object_show(wid->conform);

	if (wid->wod)
	{
		layout = elm_layout_add(wid->conform);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_file_set(layout, wid->edj_path, "full_widget_layout");
		elm_layout_signal_callback_add(layout, "elm,layout,search,clicked", "elm", _launch_stardict, NULL);
		elm_layout_signal_callback_add(layout, "elm,layout,word,clicked", "elm", _launch_stardict_search, wid);

		wid->word_layout = elm_layout_add(layout);
		elm_layout_file_set(wid->word_layout, wid->edj_path, "word_layout");
		elm_layout_content_set(layout, "elm.widget.swallow", wid->word_layout);
		/* Show window after base gui is set up */
		_query_word_of_day(wid, EINA_TRUE);
	}
	else
	{
		layout = elm_layout_add(wid->conform);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_file_set(layout, wid->edj_path, "widget_layout");
		elm_layout_signal_callback_add(layout, "elm,layout,action,clicked", "elm", _launch_stardict, NULL);
	}

	elm_object_content_set(wid->conform, layout);
	evas_object_show(layout);
	evas_object_show(wid->win);

	widget_app_context_set_tag(context, wid);
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_destroy(widget_context_h context, widget_app_destroy_type_e reason, bundle *content, void *user_data)
{
	widget_instance_data_s *wid = NULL;
	widget_app_context_get_tag(context,(void**)&wid);

	if (wid->win)
		evas_object_del(wid->win);

	if (wid->word)
	{
		free(wid->word);
		wid->word = NULL;
	}
	free(wid);

	return WIDGET_ERROR_NONE;
}

static Eina_Bool
_widget_update(void *data)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	_query_word_of_day(wid, EINA_FALSE);
	return ECORE_CALLBACK_RENEW;
}

static int
widget_instance_pause(widget_context_h context, void *user_data)
{
	/* Take necessary actions when widget instance becomes invisible. */
	widget_instance_data_s *wid = NULL;
	widget_app_context_get_tag(context,(void**)&wid);
	if (wid->widget_update_timer) {
		ecore_timer_del(wid->widget_update_timer);
		wid->widget_update_timer = NULL;
	}
	return WIDGET_ERROR_NONE;

}

static int
widget_instance_resume(widget_context_h context, void *user_data)
{
	/* Take necessary actions when widget instance becomes visible. */
	widget_instance_data_s *wid = NULL;
	widget_app_context_get_tag(context,(void**)&wid);
	_query_word_of_day(wid, EINA_FALSE);
	if (wid->widget_update_timer) {
		ecore_timer_del(wid->widget_update_timer);
		wid->widget_update_timer = NULL;
	}
	wid->widget_update_timer = ecore_timer_add(60, _widget_update, wid);
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_update(widget_context_h context, bundle *content,
                             int force, void *user_data)
{
	/* Take necessary actions when widget instance should be updated. */
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_resize(widget_context_h context, int w, int h, void *user_data)
{
	/* Take necessary actions when the size of widget instance was changed. */
	return WIDGET_ERROR_NONE;
}

static void
widget_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_LANGUAGE_CHANGED */
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
}

static void
widget_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_REGION_FORMAT_CHANGED */
}

static widget_class_h
widget_app_create(void *user_data)
{
	/* Hook to take necessary actions before main event loop starts.
	   Initialize UI resources.
	   Make a class for widget instance.
	*/
	app_event_handler_h handlers[5] = {NULL, };

	widget_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
		APP_EVENT_LANGUAGE_CHANGED, widget_app_lang_changed, user_data);
	widget_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
		APP_EVENT_REGION_FORMAT_CHANGED, widget_app_region_changed, user_data);

	widget_instance_lifecycle_callback_s ops = {
		.create = widget_instance_create,
		.destroy = widget_instance_destroy,
		.pause = widget_instance_pause,
		.resume = widget_instance_resume,
		.update = widget_instance_update,
		.resize = widget_instance_resize,
	};

	return widget_app_class_create(ops, user_data);
}

static void
widget_app_terminate(void *user_data)
{
	/* Release all resources. */
}

int
main(int argc, char *argv[])
{
	widget_app_lifecycle_callback_s ops = {0,};
	int ret;

	ops.create = widget_app_create;
	ops.terminate = widget_app_terminate;

	ret = widget_app_main(argc, argv, &ops, NULL);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "widget_app_main() is failed. err = %d", ret);
	}

	return ret;
}


