#include "dict.h"
extern "C"
{
   cst_voice* register_cmu_us_kal();
}
cst_voice* voice;
static void _show_result_url(void *data);

string test(string test1, appdata_s* ad)
{
   return ad->StarDict->lookupWords(test1,true);
}

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
   ui_app_exit();
}

static void
win_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   if (evas_object_visible_get(ad->genlist))
      evas_object_hide(ad->genlist);
}

void
app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
   char *res_path = app_get_resource_path();
   if (res_path) {
      snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
      free(res_path);
   }
}

static void
_entry_clicked(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_entry_input_panel_show(ad->entry);
   elm_object_focus_set(ad->entry, EINA_TRUE);
}

static void
_item_selected(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Elm_Object_Item *item = (Elm_Object_Item*)event_info;
   char *search = (char*)elm_object_item_data_get(item);
   elm_entry_entry_set(ad->entry, search);
   elm_entry_cursor_end_set(ad->entry);
   elm_genlist_item_selected_set(item, EINA_FALSE);
   _show_result(ad, NULL, NULL);
}

static void
_prediction_item_del_cb(void *data, Evas_Object *obj)
{
	char *s = (char*)data;
	free(s);
}

static char*
_prediction_item_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	char *s = (char*)data;
	if (!strcmp(part, "elm.text"))
		return strdup(s);
	return NULL;
}

static Eina_Bool
_show_predictions(void *data)
{
   appdata_s *ad = (appdata_s*)data;
   int pred = 0;
   preference_get_int("prediction", &pred);
   if (!pred) return ECORE_CALLBACK_CANCEL;

   int count = 0;
   char* query = (char*) strdup(elm_entry_entry_get(ad->entry));
   int len = strlen(query);
   for (int i = 0; i < len; i++)
   {
	   query[i] = tolower(query[i]);
   }

   ad->StarDict->get_suggestions(query, &ad->pred, count);

   if (count > 0)
	   for(int i = 0; (i < count) && (i < 49); i++) {
		   if (!ad->pred_item[i]) {
			   ad->pred_item[i] = elm_genlist_item_append(ad->genlist, ad->itc, strdup(ad->pred[i]), NULL, ELM_GENLIST_ITEM_NONE, _item_selected, ad);
		   }
		   else {
			   char *s = (char*)elm_object_item_data_get(ad->pred_item[i]);
			   free(s);
		       s = strdup(ad->pred[i]);
		       elm_object_item_data_set(ad->pred_item[i], s);
			   elm_genlist_item_update(ad->pred_item[i]);
		   }
	   }
   if (count == 0)
   {
	   elm_genlist_clear(ad->genlist);
	   for (int i = 0; i < 50; i++)
		   ad->pred_item[i] = NULL;
   }
   else if (count < 49)
   {
	   for (int i = count; i < 49; i++)
	   {
		   elm_object_item_del(ad->pred_item[i]);
		   ad->pred_item[i] = NULL;
	   }
   }

   if (count > 0)
   {
	  elm_layout_signal_emit(ad->layout, "elm,dict,prediction,show", "elm");
      evas_object_show(ad->genlist);
   }
   else
   {
	  elm_layout_signal_emit(ad->layout, "elm,dict,prediction,hide", "elm");
      evas_object_hide(ad->genlist);
   }
   elm_object_focus_set(ad->entry, EINA_TRUE);
   for (int i = 0; i < count; i++)
	   free(ad->pred[i]);
   free(query);
   return ECORE_CALLBACK_DONE;
}

static void
_entry_changed(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   if (ad->pred_idler) {
      ecore_idler_del(ad->pred_idler);
      ad->pred_idler = NULL;
   }
   if (elm_win_rotation_get(ad->win) == 0)
      ad->pred_idler = ecore_idler_add(_show_predictions, data);
}

static void
_item_pop(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_naviframe_item_pop(ad->naviframe);
}

static void
_go_back(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   _history_list *item = ad->url_list;
   ad->push_flag = EINA_FALSE;

   ewk_view_url_request_set(ad->ewk, item->prev->url, EWK_HTTP_METHOD_POST, NULL, NULL);
   ad->url_list = item->prev;
   free(item->url);
   free(item);
}

static void
_app_exit_confirm(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Eina_List *list = elm_naviframe_items_get(ad->naviframe);
   elm_layout_signal_emit(ad->layout, "elm,dict,prediction,hide", "elm");
   if(eina_list_count(list) > 1)
   {
      if (list) eina_list_free(list);
      elm_naviframe_item_pop(ad->naviframe);
      return;
   }
   if (list) eina_list_free(list);
   if (elm_toolbar_selected_item_get(ad->title_toolbar) != elm_toolbar_first_item_get(ad->title_toolbar))
   {
      elm_toolbar_item_selected_set(elm_toolbar_first_item_get(ad->title_toolbar), EINA_TRUE);
      return;
   }
   if (ad->url_list->prev && !ad->url_list->prev->prev)
     _go_back(ad, NULL, NULL);
   else
     {
       elm_entry_entry_set(ad->entry, "");
       ewk_view_contents_set(ad->ewk, "", 1, NULL, NULL, NULL);
     }
   if (ad->pred_idler)
     {
       ecore_idler_del(ad->pred_idler);
       ad->pred_idler = NULL;
     }
   evas_object_hide(ad->genlist);
   elm_object_focus_set(ad->entry, EINA_FALSE);

   Evas_Object *popup = elm_popup_add(ad->naviframe);
   elm_object_style_set(popup, "toast");
   elm_object_text_set(popup, "Press back again to exit");
   eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _item_pop, ad);
   elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "timeout", _popup_del, popup);
   evas_object_smart_callback_add(popup, "block,clicked", _popup_del, popup);
   evas_object_show(popup);
}

char *trimwhitespace(char *str)
{
   char *end;

   // Trim leading space
   while(isspace(*str)) str++;

   if(*str == 0)  // All spaces?
      return str;

   // Trim trailing space
   end = str + strlen(str) - 1;
   while(end > str && isspace(*end)) end--;

   // Write new null terminator
   *(end+1) = 0;

   return str;
}

static void
_push_history(appdata_s *ad, char *url)
{
	if (!ad->url_list)
	{
		ad->url_list = (_history_list*)malloc(sizeof(_history_list));
		ad->url_list->url = url;
		ad->url_list->prev = NULL;
	}
	else
	{
		_history_list *item = (_history_list*)malloc(sizeof(_history_list));
		item->url = url;
		item->prev = ad->url_list;
		ad->url_list = item;
	}
}

bool
_check_internet()
{
	bool internet_available = false;
	connection_h connection;
	connection_type_e type;
	if (connection_create(&connection) == CONNECTION_ERROR_NONE)
	{
	    if(connection_get_type(connection, &type) == CONNECTION_ERROR_NONE){
	        if(type == CONNECTION_TYPE_WIFI || type == CONNECTION_TYPE_CELLULAR){
	            internet_available = true;
	        } else {
	            internet_available = false;
	        }
	    }
	    connection_destroy(connection);
	}
	return internet_available;
}

static void
_webkit_navigation_start(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Ewk_Policy_Decision *policy = (Ewk_Policy_Decision*)event_info;
   const char *url = ewk_policy_decision_url_get(policy);
   if (url && !strncmp(url, "stardict:pronounce", 18))
   {
      ewk_view_stop(ad->ewk);
      flite_init();
      voice = register_cmu_us_kal();
      char *query = ad->url_list->url;
      char *data_path = app_get_data_path();
      char wav_path[PATH_MAX];
      sprintf(wav_path, "%s/sound.wav", data_path);
      if (query)
      {
	 wav_player_stop(ad->wav_id);
         flite_text_to_speech(query + 8, voice, wav_path);
         wav_player_start(wav_path, SOUND_TYPE_MEDIA, NULL, NULL, &(ad->wav_id));
      }
      free(data_path);
      return;
   }
   if (url && !strncmp(url, "bword://", 8))
   {
      ewk_view_stop(ad->ewk);
      elm_entry_entry_set(ad->entry, url+8);
      elm_layout_signal_emit(ad->layout, "elm,dict,progress,hide", "elm");
      evas_object_data_set(ad->ewk, "url", strdup(url));
      _show_result_url(ad);
   }
   else if (url && !strncmp(url, "http", 4))
   {
	  if (_check_internet())
	  {
	    evas_object_data_set(ad->ewk, "url", strdup(url));
	    elm_layout_signal_emit(ad->layout, "elm,dict,progress,show", "elm");
	  }
	  else
	  {
		  ewk_view_stop(ad->ewk);
		  Evas_Object *popup = elm_popup_add(ad->ewk);
		  elm_object_part_text_set(popup, "title,text", "StarDict");
		  elm_object_text_set(popup, "No Network Connection!");
		  elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
		  Evas_Object *ok = elm_button_add(popup);
		  elm_object_text_set(ok, "OK");
		  evas_object_smart_callback_add(ok, "clicked", _popup_del, popup);
		  elm_object_part_content_set(popup, "button1", ok);
		  evas_object_show(popup);
	  }
   }
   else if (url && !strcmp(url, "stardict:about:blank"))
   {
	   elm_entry_entry_set(ad->entry, "");
	   ewk_view_url_request_set(ad->ewk, "about:blank", EWK_HTTP_METHOD_POST, NULL, NULL);
	   ad->push_flag = EINA_TRUE;
   }
}

static void
_webkit_loading_start(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   appdata_s *ad = (appdata_s*)data;
   elm_progressbar_value_set(ad->progress_bar, 0);
}

static void
_webkit_loading_commited(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   appdata_s *ad = (appdata_s*)data;
   if (evas_object_visible_get(ad->progress_bar)) //checking http url alternatively by checking
   {                                              //visiblity of progress bar
	   char *url = (char*)evas_object_data_get(ad->ewk, "url");
	   if (ad->push_flag)
	       _push_history(ad, url);
	   else
	       ad->push_flag = EINA_TRUE;
   }
   if (ad->url_list && ad->url_list->prev)
      elm_layout_signal_emit(ad->app_layout, "elm,dict,back,show", "elm");
   else
      elm_layout_signal_emit(ad->app_layout, "elm,dict,back,hide", "elm");
}

static void
_webkit_loading_done(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   appdata_s *ad = (appdata_s*)data;
   elm_layout_signal_emit(ad->layout, "elm,dict,progress,hide", "elm");
}

static void
_webkit_loading(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   appdata_s *ad = (appdata_s*)data;
   elm_progressbar_value_set(ad->progress_bar, ewk_view_load_progress_get(ad->ewk));
}

void
_show_result(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   const char *word = elm_entry_entry_get(ad->entry);
   if (ad->search_word) {
	   free(ad->search_word);
	   ad->search_word = NULL;
   }
   char url[2048];
   strcpy(url, "bword://");
   strcat(url, word);
   ewk_view_url_request_set(ad->ewk, url, EWK_HTTP_METHOD_POST, NULL, NULL);
}

static void
_show_result_url(void *data)
{
   appdata_s *ad = (appdata_s*)data;
   elm_object_focus_set(ad->entry, EINA_FALSE);
   const char *query = elm_entry_entry_get(ad->entry);
   char *query_dup = strdup(query);
   if (!query_dup) return; // malloc failed , why usually shouldn't happen :)
   query_dup = trimwhitespace(query_dup);
   string query_trimmed (query_dup);

   if (!elm_layout_content_get(ad->layout, "elm.swallow.result"))
      elm_layout_content_set(ad->layout, "elm.swallow.result", ad->ewk);

   if (strlen(query_dup) == 0)
   {
      Evas_Object *toast_popup = elm_popup_add(ad->naviframe);
      elm_object_style_set(toast_popup, "toast");
      elm_object_text_set(toast_popup, "Please type something");
      elm_popup_allow_events_set(toast_popup, EINA_TRUE);
      elm_popup_timeout_set(toast_popup, 2.0);
      evas_object_smart_callback_add(toast_popup, "timeout", _popup_del, toast_popup);
      evas_object_show(toast_popup);
      free(query_dup);
      return;
   }

   string result_formatted = "<!doctype HTML><html><head><style> a{-webkit-tap-highlight-color: rgba(43 ,130 ,143 , 0.26);} a {text-decoration: none !important;} a:link, a:hover, a:active{color:#1D1FE7} i {color:#B14E27}</style></head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0\"><body>";
   string result = test(query_trimmed, ad);
   if (result.length())
       result_formatted += "<ol style='padding-left:0px;' id='pronounce'><b>Pronounce:</b>&nbsp;<a href='stardict:pronounce'><img src='speaker.png' alt='Pronounce it' style='vertical-align:middle;'></a></ol>";
   if (!result.length()) result = "<br/>Sorry, We don't have data of <font color='red'><i> " + query_trimmed + " </i></font> in our offline database. <a href='http://en.wikipedia.org/wiki/" + query_trimmed + "'><br/>Click Here </a> to search it online.";
   else
   {
	   _store_keyword(query_dup);
	   char *url = (char*)evas_object_data_get(ad->ewk, "url");
	   if (ad->push_flag)
		   _push_history(ad, url);
	   else
		   ad->push_flag = EINA_TRUE;
   }
    result_formatted += result + "<script>var a = document.getElementsByTagName('a'); for (var idx = 0; idx < a.length; ++idx) { a[idx].setAttribute('oncontextmenu', 'return false;');}</script></body></html>";
   if (ad->pred_idler)
   {
      ecore_idler_del(ad->pred_idler);
      ad->pred_idler = NULL;
   }
   evas_object_hide(ad->genlist);
   elm_layout_signal_emit(ad->layout, "elm,dict,prediction,hide", "elm");
   ewk_view_contents_set(ad->ewk, result_formatted.c_str(), result_formatted.length(), NULL, NULL, "file:///opt/usr/apps/org.tizen.dict_hindi/res/js/");
   return;
}

static void
_webkit_mouse_down(void *data,
            Evas *evas EINA_UNUSED,
            Evas_Object *obj,
            void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_object_focus_set(ad->entry, EINA_FALSE);
}

static void
_mouse_down(void *data,
            Evas *evas EINA_UNUSED,
            Evas_Object *obj,
            void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down*)event_info;
   ad->mouse_x = ev->canvas.x;
   ad->mouse_y = ev->canvas.y;
   ad->mouse_down_time = ev->timestamp;
}

static void
_mouse_up(void *data,
          Evas *evas EINA_UNUSED,
          Evas_Object *obj,
          void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Eina_List *list = elm_naviframe_items_get(ad->naviframe);
   if (eina_list_count(list) > 1)
   {
	   eina_list_free(list);
	   return;
   }
   eina_list_free(list);
   int x_del, y_del;
   Evas_Coord y, h;
   Elm_Object_Item *item;
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*)event_info;
   if ((ev->timestamp - ad->mouse_down_time) > 400) return;
   x_del = ev->canvas.x - ad->mouse_x;
   y_del = ev->canvas.y - ad->mouse_y;
   evas_object_geometry_get(ad->entry, NULL, &y, NULL, &h);
   if (abs(x_del) < (2 * abs(y_del))) return;
   if (abs(x_del) < 100) return;
   if (x_del < 0 && (ev->canvas.y > (y+h) || ad->mouse_y > (y+h)))
      item = elm_toolbar_last_item_get(ad->title_toolbar);
   else if (x_del > 0)
      item = elm_toolbar_first_item_get(ad->title_toolbar);
   else return;

   elm_toolbar_item_selected_set(item, EINA_TRUE);
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
   ui_app_exit();
   return EINA_TRUE;
}

void
_on_search_field_focus(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_layout_signal_emit(ad->layout,"elm,dict,focused","elm");
}

void
_on_search_field_unfocus(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_layout_signal_emit(ad->layout,"elm,dict,unfocused","elm");
}

static Evas_Object*
_search_screen(appdata_s *ad)
{
   char edj_path[PATH_MAX];
   ad->layout = elm_layout_add(ad->win);
   evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
   elm_layout_file_set(ad->layout, edj_path, GROUP_MAIN);
   evas_object_show(ad->layout);
   ad->pred_idler = NULL;

   ad->entry = elm_entry_add(ad->layout);
   elm_scroller_policy_set(ad->entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_scrollable_set(ad->entry, EINA_TRUE);
   elm_entry_single_line_set(ad->entry, EINA_TRUE);
   elm_entry_prediction_allow_set(ad->entry, EINA_FALSE);
   elm_object_part_content_set(ad->layout, "elm.swallow.entry", ad->entry);
   elm_object_part_text_set(ad->entry, "elm.guide", " Search");
   elm_entry_input_panel_return_key_type_set(ad->entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
   evas_object_smart_callback_add(ad->entry, "activated", _show_result, ad);
   evas_object_smart_callback_add(ad->entry, "clicked", _entry_clicked, ad);
   evas_object_event_callback_add(ad->entry, EVAS_CALLBACK_KEY_UP, _entry_changed, ad);
   evas_object_smart_callback_add(ad->entry, "focused", _on_search_field_focus, ad);
   evas_object_smart_callback_add(ad->entry, "unfocused", _on_search_field_unfocus, ad);

   ad->progress_bar = elm_progressbar_add(ad->layout);
   elm_layout_content_set(ad->layout, "elm.swallow.progress", ad->progress_bar);
   evas_object_size_hint_weight_set(ad->progress_bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->progress_bar, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_hide(ad->progress_bar);

   ad->ewk = ewk_view_add(evas_object_evas_get(ad->layout));
   int val = 20;
   Ewk_Settings *settings = ewk_view_settings_get(ad->ewk);
   if (!preference_get_int("fontsize", &val))
	   ewk_settings_default_font_size_set(settings, val);
   evas_object_smart_callback_add(ad->ewk, "load,started", _webkit_loading_start, ad);
   evas_object_smart_callback_add(ad->ewk, "load,progress", _webkit_loading, ad);
   evas_object_smart_callback_add(ad->ewk, "load,finished", _webkit_loading_done, ad);
   evas_object_smart_callback_add(ad->ewk, "load,committed", _webkit_loading_commited, ad);
   evas_object_smart_callback_add(ad->ewk, "policy,navigation,decide", _webkit_navigation_start, ad);
   evas_object_size_hint_weight_set(ad->ewk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->ewk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(ad->ewk, EVAS_CALLBACK_MOUSE_DOWN, _webkit_mouse_down, ad);

   char *init_url = (char*)malloc(sizeof(char)*32);
   strcpy(init_url, "stardict:about:blank");
   _push_history(ad, init_url);
   return ad->layout;
}

static Eina_Bool
_entry_focus_idler(void *data)
{
   appdata_s *ad = (appdata_s*)data;
   elm_object_focus_set(ad->entry, EINA_TRUE);
   return ECORE_CALLBACK_DONE;
}

static void
_show_search_screen(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_object_content_set(ad->naviframe, ad->layout);
   if (ad->url_list && ad->url_list->prev)
      elm_layout_signal_emit(ad->app_layout, "elm,dict,back,show", "elm");
   else
      elm_layout_signal_emit(ad->app_layout, "elm,dict,back,hide", "elm");
}

static void
_splash_finished_cb(void *data, Evas_Object *obj, const char *part, const char *emission)
{
   appdata_s *ad = (appdata_s*)data;
   if (!ad->search_word)
	   ecore_idler_add(_entry_focus_idler, ad);
   else
   {
	   elm_entry_entry_set(ad->entry, ad->search_word);
	   evas_object_smart_callback_call(ad->entry, "activated", NULL);
   }
   evas_object_freeze_events_set(ad->app_layout, EINA_FALSE);
   elm_layout_content_set(ad->layout, "elm.swallow.result", ad->ewk);
}

static void
_lookup_word_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Elm_Object_Item *item = (Elm_Object_Item*)event_info;
   const char *keyword = elm_object_item_text_get(item);
   elm_toolbar_item_selected_set(elm_toolbar_first_item_get(ad->title_toolbar), EINA_TRUE);
   elm_entry_entry_set(ad->entry, keyword);
   evas_object_smart_callback_call(ad->entry, "activated", NULL);
   elm_entry_cursor_end_set(ad->entry);
}

static int
_get_lookupwords(void *data, int argc, char **argv, char **azColName)
{
   appdata_s *ad = (appdata_s*)data;
   elm_list_item_append(ad->lookup_list, argv[0], NULL, NULL, _lookup_word_sel_cb, ad);
   return 0;
}

static void
_show_lookup_results(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   Evas_Object *nf = ad->naviframe;
   Elm_Object_Item *it;

   it = (Elm_Object_Item*)event_info;
   ad->lookup_list = elm_list_add(nf);
   elm_list_mode_set(ad->lookup_list, ELM_LIST_COMPRESS);
   _query_keywords(ad, _get_lookupwords);

   Evas_Object *layout = elm_layout_add(ad->naviframe);
   if (elm_list_first_item_get(ad->lookup_list))
   {
      Evas_Object *bg = elm_bg_add(layout);
      elm_bg_color_set(bg, 250, 250, 250);
      elm_layout_theme_set(layout, "layout", "application", "default");
      elm_list_go(ad->lookup_list);
      elm_layout_content_set(layout, "elm.swallow.content", ad->lookup_list);
      elm_layout_content_set(layout, "elm.swallow.bg", bg);
   }
   else
   {
      elm_layout_theme_set(layout, "layout", "nocontents", "default");
      elm_layout_text_set(layout, "elm.text", "No searches done yet!");
   }
   Evas_Object *old_content = elm_object_content_unset(nf);
   if (old_content == ad->layout)
	   evas_object_hide(ad->layout);
   else evas_object_del(old_content);
   elm_object_content_set(nf, layout);
   elm_layout_signal_emit(ad->app_layout, "elm,dict,back,hide", "elm");
}

static void
_tab_focused(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   if (elm_toolbar_selected_item_get(ad->title_toolbar) == elm_toolbar_first_item_get(ad->title_toolbar))
   {
      elm_layout_content_set(ad->layout, "elm.swallow.result", ad->ewk);
   }
}

static Evas_Object*
_create_tabbar(appdata_s *ad)
{
   Elm_Object_Item *item;

   ad->title_toolbar = elm_toolbar_add(ad->naviframe);
   elm_object_style_set(ad->title_toolbar, "tabbar");
   elm_toolbar_shrink_mode_set(ad->title_toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_toolbar_transverse_expanded_set(ad->title_toolbar, EINA_TRUE);

   _search_screen(ad);
   item = elm_toolbar_item_append(ad->title_toolbar, NULL, "Search", _show_search_screen, ad);
   elm_toolbar_item_append(ad->title_toolbar, NULL, "Lookup", _show_lookup_results, ad);
   elm_toolbar_select_mode_set(ad->title_toolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_toolbar_item_selected_set(item, EINA_TRUE);
   evas_object_smart_callback_add(ad->title_toolbar, "item,focused", _tab_focused, ad);

   return ad->title_toolbar;
}

static Eina_Bool
_show_win_idler(void *data)
{
	   appdata_s *ad = (appdata_s*)data;
	   evas_object_show(ad->win);
	   if (!ad->widget_launch)
		   elm_layout_signal_emit(ad->app_layout, "elm,splash,show", "elm");
	   else
		   elm_layout_signal_emit(ad->app_layout, "elm,nosplash,show", "elm");
	   ad->showinidler = NULL;
	   return ECORE_CALLBACK_CANCEL;
}

static void
create_base_gui(appdata_s *ad)
{
   Elm_Object_Item *nf_item;
   Evas_Object *menu_btn, *back_btn;
   char edj_path[PATH_MAX];
   ad->win = elm_win_add(NULL, PACKAGE, ELM_WIN_BASIC);
   elm_win_alpha_set(ad->win, EINA_TRUE);
   elm_win_autodel_set(ad->win, EINA_TRUE);
   ad->url_list = NULL;
   ad->push_flag = EINA_TRUE;

   ad->pred = new char*[100];
   ad->pred_item = new Elm_Object_Item*[50];
   for (int i=0; i<100; i++)
   {
      ad->pred[i] = NULL;
   }
   for (int i=0; i<50; i++)
   {
	   ad->pred_item[i] = NULL;
   }
   ad->wav_id = 0;
   Evas_Object *conform = elm_conformant_add(ad->win);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(ad->win, conform);
   evas_object_show(conform);
   elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
   elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_TRANSPARENT);
   app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

   if (elm_win_wm_rotation_supported_get(ad->win)) {
      int rots[4] = { 0, 90, 180, 270 };
      elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
   }

   evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
   evas_object_smart_callback_add(ad->win, "rotation,changed", win_rotate_cb, ad);
   ad->app_layout = elm_layout_add(ad->win);
   elm_layout_file_set(ad->app_layout, edj_path, "applayout");
   elm_layout_text_set(ad->app_layout, "elm.app.name", "StarDict");
   elm_layout_signal_callback_add(ad->app_layout, "elm,dict,splash,finish", "elm", _splash_finished_cb, ad);

   ad->naviframe = elm_naviframe_add(ad->app_layout);
   evas_object_size_hint_weight_set(ad->naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->naviframe, EVAS_HINT_FILL, EVAS_HINT_FILL);
   eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, _app_exit_confirm, ad);
   nf_item = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, NULL, "tabbar/notitle");
   eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
   elm_naviframe_item_title_enabled_set(nf_item, EINA_FALSE, EINA_TRUE);
   elm_naviframe_item_pop_cb_set(nf_item, naviframe_pop_cb, ad);

   menu_btn = elm_button_add(ad->naviframe);
   elm_object_style_set(menu_btn, "naviframe/more/default");
   evas_object_smart_callback_add(menu_btn, "clicked", create_ctxpopup_more_button_cb, ad);
   elm_object_item_part_content_set(nf_item, "toolbar_more_btn", menu_btn);
   evas_object_show(menu_btn);

   back_btn = elm_button_add(ad->naviframe);
   elm_object_style_set(back_btn, "naviframe/end_btn/default");
   elm_object_text_set(back_btn, "Back");
   evas_object_smart_callback_add(back_btn, "clicked", _go_back, ad);
   elm_layout_content_set(ad->app_layout, "elm.swallow.button", back_btn);

   Evas_Object *tabbar = _create_tabbar(ad);
   elm_object_item_part_content_set(nf_item, "tabbar", tabbar);
   elm_layout_content_set(ad->app_layout, "elm.swallow.content", ad->naviframe);
   evas_object_event_callback_add(ad->app_layout, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, ad);
   evas_object_event_callback_add(ad->app_layout, EVAS_CALLBACK_MOUSE_UP, _mouse_up, ad);

   elm_object_content_set(conform, ad->app_layout);
   evas_object_freeze_events_set(ad->app_layout, EINA_TRUE);

   ad->itc = elm_genlist_item_class_new();
   ad->itc->func.text_get = _prediction_item_text_get_cb;
   ad->itc->func.del = _prediction_item_del_cb;
   ad->genlist = elm_genlist_add(ad->naviframe);
   elm_genlist_mode_set(ad->genlist, ELM_LIST_COMPRESS);
   elm_object_focus_allow_set(ad->genlist, EINA_FALSE);
   elm_genlist_homogeneous_set(ad->genlist, EINA_TRUE);
   elm_genlist_block_count_set(ad->genlist, 7);
   evas_object_size_hint_weight_set(ad->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(ad->layout, "elm.swallow.prediction", ad->genlist);
   elm_object_content_set(conform, ad->app_layout);
   elm_win_conformant_set(ad->win, EINA_TRUE);
   ad->showinidler = ecore_idler_add(_show_win_idler, ad);

   //setting default sound type
   sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
}

static bool
app_create(void *data)
{
   /* Hook to take necessary actions before main event loop starts
                Initialize UI resources and application's data
                If this function returns true, the main loop of application starts
                If this function returns false, the application is terminated */
   appdata_s *ad = (appdata_s*)data;

   _init_tables();
   ad->search_word = NULL;
   bool exist = false;
   ad->widget_launch = EINA_FALSE;
   preference_is_existing("prediction", &exist);
   if (!exist) preference_set_int("prediction", 1);
   create_base_gui(ad);
   ad->StarDict = new clsSDict;
   char *path = app_get_shared_resource_path();
   char dic_path[1024];
   sprintf(dic_path, "%s/../dict", path);
   ad->StarDict->loadDics(dic_path);
   free(path);
   return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	appdata_s *ad = (appdata_s*)data;
   char *launch_info;
   app_control_get_extra_data(app_control, "launch_info", &launch_info);
   if (strcmp(launch_info, "widget_launch") == 0)
	   ad->widget_launch = EINA_TRUE;
   app_control_get_extra_data(app_control, "search", &ad->search_word);
}

static void
app_pause(void *data)
{
   /* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
   /* Take necessary actions when application becomes visible. */
	   appdata_s *ad = (appdata_s*)data;
	   if (ad->search_word && !ad->showinidler)
	   {
		   elm_entry_entry_set(ad->entry, ad->search_word);
		   evas_object_smart_callback_call(ad->entry, "activated", NULL);
	   }
}

static void
app_terminate(void *data)
{
   appdata_s *ad = (appdata_s*)data;
   if (ad->url_list)
   {
	   _history_list *item;
	   while(ad->url_list->prev)
	   {
		   item = ad->url_list;
		   ad->url_list = item->prev;
		   free(item->url);
		   free(item);
	   }
	   free(ad->url_list->url);
	   free(ad->url_list);
   }
   if (ad->pred_idler)
	   ecore_idler_del(ad->pred_idler);
   if (ad->showinidler)
	   ecore_idler_del(ad->showinidler);
   delete[] ad->pred;
   delete[] ad->pred_item;
   delete ad->StarDict;
   /* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
   /*APP_EVENT_LANGUAGE_CHANGED*/
   char *locale = NULL;
   system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
   elm_language_set(locale);
   free(locale);
   return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
   /*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
   return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
   /*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
   /*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
   /*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
   appdata_s ad;
   int ret = 0;

   ui_app_lifecycle_callback_s event_callback = {0,};
   app_event_handler_h handlers[5] = {NULL, };

   event_callback.create = app_create;
   event_callback.terminate = app_terminate;
   event_callback.pause = app_pause;
   event_callback.resume = app_resume;
   event_callback.app_control = app_control;

   ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
   ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
   ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
   ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
   ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
   ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

   ret = ui_app_main(argc, argv, &event_callback, &ad);

   return ret;
}
