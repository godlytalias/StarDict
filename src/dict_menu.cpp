#include "dict.h"

static void
ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
}

void
_popup_del(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *popup = (Evas_Object *)data;
   evas_object_del(popup);
}

static Eina_Bool
_item_pop(void *data, Elm_Object_Item *item EINA_UNUSED)
{
   appdata_s *ad = (appdata_s*)data;
   if (ad->url_list && ad->url_list->prev)
      elm_layout_signal_emit(ad->app_layout, "elm,dict,back,show", "elm");
   return EINA_TRUE;
}

static void
_webkit_navigation_start(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ewk = (Evas_Object*)data;
	Ewk_Policy_Decision *policy = (Ewk_Policy_Decision*)event_info;
	const char *url = ewk_policy_decision_url_get(policy);
	if (url && !strncmp(url, "http", 4) && !_check_internet())
	{
		ewk_view_stop(ewk);
		Evas_Object *popup = elm_popup_add(ewk);
		elm_object_part_text_set(popup, "title,text", "StarDict");
		elm_object_text_set(popup, "No Network Connection!");
		elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
		Evas_Object *ok = elm_button_add(popup);
		elm_object_text_set(ok, "OK");
		evas_object_smart_callback_add(ok, "clicked", _popup_del, popup);
		elm_object_part_content_set(popup, "button1", ok);
		evas_object_show(popup);
	}
	else if(url && !strncmp(url, "mailto:", 7))
	{
		ewk_view_stop(ewk);
		app_control_h app_control;
		app_control_create(&app_control);
		char *mail_address = strdup(url);
		const char *subject = "StarDict User Query";

		app_control_set_operation(app_control, APP_CONTROL_OPERATION_COMPOSE);
		app_control_set_uri(app_control, mail_address);
		app_control_add_extra_data(app_control, APP_CONTROL_DATA_SUBJECT, subject);
		app_control_set_launch_mode(app_control, APP_CONTROL_LAUNCH_MODE_GROUP);
		app_control_send_launch_request(app_control, NULL, NULL);
		app_control_destroy(app_control);
		free(mail_address);
	}
}

static void
_font_size_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	double val = elm_slider_value_get(obj);
	Ewk_Settings *settings = ewk_view_settings_get(ad->ewk);
	ewk_settings_default_font_size_set(settings, (int)val);
	preference_set_int("fontsize", (int)val);
}

static void
ctxpopup_item_select_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	const char *title_label = elm_object_item_text_get((Elm_Object_Item *) event_info);
	appdata_s *ad = (appdata_s*)data;

	Evas_Object *popup = elm_popup_add(elm_object_top_widget_get(obj));
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	elm_object_part_text_set(popup, "title,text", title_label);
	if (!strcmp(title_label, "About"))
	{
		Evas_Object *webview = ewk_view_add(ad->naviframe);;
		evas_object_smart_callback_add(webview, "policy,navigation,decide", _webkit_navigation_start, webview);
		Elm_Object_Item *item;
		evas_object_size_hint_weight_set(webview, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(webview, EVAS_HINT_FILL, EVAS_HINT_FILL);
		string about_html = "<!doctype HTML><html><head><script>window.oncontextmenu = function(event) { event.preventDefault(); \
				    event.stopPropagation(); return false;}</script><style> a{-webkit-tap-highlight-color: rgba(43 ,130 ,143 , 0.26);} \
				    a {text-decoration: none !important;} a:link, a:hover, a:active{color:#1D1FE7} i {color:#B14E27}</style></head> \
				    <meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no, \
				    minimum-scale=1.0, maximum-scale=1.0\"><body>";
		about_html += "<div data-role=\"page\" id=\"foo5\" data-position=\"fixed\">    <div data-role=\"header\" data-position=\"fixed\" \
				data-tap-toggle=\"false\" data-theme=\"b\">         <h1 style = 'font-size: 1.5em;'>About This App</h1>     </div> \
				<div data-role=\"content\">        <style>            h3{color:brown}        </style>        <ee style=\"color: brown; \
				font-size: large;font-weight: bold;\">What is this app?</ee>        <p>This app was developed with an aim to provide offline access \
				to the WordWeb dictionary bundled with 280,000 words and phrases. User can also access the Encyclopedia links by clicking on \
				words/phrases under the encyclopedia section of the word.</p>         <h3>Features</h3>         <ol>            <li>Huge Database \
				powered by Wordweb</li>            <li>Noun, Verb, Interjection, Adjectives, Synonyms, Anonyms</li>            <li>Phonetics, \
				Derived forms, Similar sounding words</li>            <li>Seperate sections with Wikipedia links</li>            <li>Scan selection \
				word</li>            <li>Predictions of Keywords</li>            <li>Search History</li>                        <li>Word of the day \
				widget</li>            <p>... and much more to come</p>        </ol>         <h3>Developers</h3>         <ol>            \
				<li style=\"margin-bottom: 3%;\"><a style=\"text-decoration: none;\" href=\"https://www.facebook.com/godly.t.alias\"> \
				Godly T.Alias</a>            </li>            <li style=\"margin-bottom: 3%;\"><a style=\"text-decoration: none;\" \
				href=\"https://www.facebook.com/divyesh.purohit.58\">Divyesh Purohit</a>            </li>            \
				<li><a style=\"text-decoration: none;\" href=\"https://www.facebook.com/prasoon.singhsengar\">Prasoon Singh</a> \
				</li>        </ol>         <h3> Credits </h3> We would like to credit Stardict for their research on dictionaries and an \
				Opensource library containing rich set of APIs , wiihout that making this app would be impossible         <h3> \
				Report the Bugs </h3>         <p>We would love to see what bugs you face and would like to resolve them as soon as we can</p>  \
				<p>please report the bugs to</p> <p><a href='mailto:godlytalias@yahoo.co.in'>godlytalias@yahoo.co.in</a> / \
				<a href='mailto:purohit.div@gmail.com'>purohit.div@gmail.com</a> / \
				<a href='mailto:prasoonsigh16@gmail.com'>prasoonsigh16@gmail.com</a></p>    </div></div>";
		about_html += "</body></html>";
		ewk_view_contents_set(webview, about_html.c_str(), about_html.size(), NULL, NULL, NULL);
		elm_layout_signal_emit(ad->app_layout, "elm,dict,back,hide", "elm");
		item = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, webview, NULL);
		elm_naviframe_item_pop_cb_set(item, _item_pop, ad);
		elm_naviframe_item_title_enabled_set(item, EINA_FALSE, EINA_TRUE);
		elm_ctxpopup_dismiss(obj);
		evas_object_del(popup);
		return;
	}
	else if (!strcmp(title_label, "Help"))
	{
                 Evas_Object *webview = ewk_view_add(ad->naviframe);;
                 evas_object_smart_callback_add(webview, "policy,navigation,decide", _webkit_navigation_start, webview);
                 Elm_Object_Item *item;
                 evas_object_size_hint_weight_set(webview, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                 evas_object_size_hint_align_set(webview, EVAS_HINT_FILL, EVAS_HINT_FILL);
                 string help = "<!doctype HTML><html><head> <style>a{-webkit-tap-highlight-color: rgba(43, 130, 143, 0.26);} \
				a{text-decoration: none !important;}a:link, a:hover, a:active{color: #1D1FE7}i{color: #B14E27}</style></head> \
				<meta charset=\ 'utf-8'><meta name=\ 'viewport' content=\ 'width=device-width, initial-scale=1.0, user-scalable=no, \
				minimum-scale=1.0, maximum-scale=1.0'><body> <div data-role=\ 'page' id=\ 'foo5' data-position=\ 'fixed'> \
				<div data-role=\ 'header' data-position=\ 'fixed' data-tap-toggle=\ 'false' data-theme=\ 'b'> \
				<h1 style='font-size: 1.5em;'>Help</h1> </div><div data-role=\ 'content'> <style>h3{color: brown}</style> \
				<h3>Search Words</h3> Users can enter the keywords that they want to search in the searchfield, \
				then press the search key in the keyboard to get the results. Users can also navigate through the links in the result \
				text by clicking the links. <h3>Search Preferences</h3> Users will get upto 10 predictions of entered word listed in \
				alphabetical order. Users can toggle the prediction on and off through application menu. 'On clicking the prediction, \
				users can get the meaning of the word. Prediction won't be available in landscape mode. <h3> Encyclopedia search </h3> \
				Stardict dictionary provides links for related words and encyclopedia links for words which need more explanation. \
				If user have the internet access, users can browse the encylopedia by clicking on the links listed against the \
				Encyclopedia heading. Other links for related words can be explored without internet connection. \
				<h3>Word Pronouncing </h3> Users can get the pronunciation of all words contained in the database. \
				You need to adjust the media volume from system settings. The Voice would of a US Male, in future we \
				might provide downloadable voice modules of different accents. <h3>Word of the Day </h3> This feature helps \
				users to learn one unfamiliar word daily. Users can add Word of the day widget packaged with this application in \
				homescreen and it will display words and its definitions which will be updated automatically everyday. Users can click on \
				the widget to open the application and to navigate to related words.<h3>Search History</h3> Users can get the search \
				history from the lookup tab, User can switch to the lookup tab either by swiping on screen or by clicking on the tabs. \
				In the lookup tab the last 50 searched words will be listed. User can select the words from the list to see the \
				meanings of the words. If user wants to clear the search history, it can be done from the clear history option \
				in application menu. <br></div></div></body></html>";
                 ewk_view_contents_set(webview, help.c_str(), help.size(), NULL, NULL, NULL);
                 elm_layout_signal_emit(ad->app_layout, "elm,dict,back,hide", "elm");
                 item = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, webview, NULL);
                 elm_naviframe_item_pop_cb_set(item, _item_pop, ad);
                 elm_naviframe_item_title_enabled_set(item, EINA_FALSE, EINA_TRUE);
                 elm_ctxpopup_dismiss(obj);
                 evas_object_del(popup);
                 return;
	}
	else if (!strcmp(title_label, "Turn off predictions"))
	{
		preference_set_int("prediction", 0);
		elm_ctxpopup_dismiss(obj);
		if (ad->pred_idler)
		{
			ecore_idler_del(ad->pred_idler);
			ad->pred_idler = NULL;
		}
		evas_object_hide(ad->ctxpopup);
		evas_object_del(popup);
		return;
	}
	else if (!strcmp(title_label, "Turn on predictions"))
	{
		preference_set_int("prediction", 1);
		elm_ctxpopup_dismiss(obj);
		if (ad->pred_idler)
		{
			ecore_idler_del(ad->pred_idler);
			ad->pred_idler = NULL;
		}
		evas_object_hide(ad->ctxpopup);
		evas_object_del(popup);
		return;
	}
	else if (!strcmp(title_label, "Clear History"))
	{
		elm_ctxpopup_dismiss(obj);
		_clear_history_table(ad);
		elm_list_clear(ad->lookup_list);
		elm_list_go(ad->lookup_list);
		Evas_Object *toast_p = elm_popup_add(ad->naviframe);
		elm_object_style_set(toast_p, "toast");
		elm_object_text_set(toast_p, "Cleared search history!");
		elm_popup_timeout_set(toast_p, 2.0);
		elm_popup_allow_events_set(toast_p, EINA_TRUE);
		evas_object_smart_callback_add(toast_p, "timeout", _popup_del, toast_p);
		evas_object_show(toast_p);
		evas_object_del(popup);
		return;
	}
	else if (!strcmp(title_label, "Font Size"))
	{
		Evas_Object *layout = elm_layout_add(popup);
		char edj_path[PATH_MAX];
		app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
		elm_layout_file_set(layout, edj_path, "app_standard_layout");
		Evas_Object *slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 14, 30);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		evas_object_smart_callback_add(slider, "changed", _font_size_changed_cb, ad);
		elm_layout_content_set(layout, "elm.swallow.content", slider);
		elm_object_content_set(popup, layout);
		Ewk_Settings *settings = ewk_view_settings_get(ad->ewk);
		elm_slider_value_set(slider, ewk_settings_default_font_size_get(settings));
	}

	Evas_Object *button = elm_button_add(popup);
	elm_object_text_set(button, "OK");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_show(button);
	evas_object_show(popup);
	evas_object_smart_callback_add(button, "clicked", _popup_del, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	elm_ctxpopup_dismiss(obj);
}

static void
move_more_ctxpopup(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *win;
	Evas_Coord w, h;
	int pos = -1;
	Evas_Object *ctxpopup = (Evas_Object*)data;

	win = elm_object_top_widget_get(ctxpopup);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctxpopup, (w / 2), h);
			break;
		case 90:
			evas_object_move(ctxpopup,  (h / 2), w);
			break;
		case 270:
			evas_object_move(ctxpopup, (h / 2), w);
			break;
	}
}

void
create_ctxpopup_more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win;
	appdata_s *ad = (appdata_s*)data;
	int pred = 0;

	Evas_Object *ctxpopup = elm_ctxpopup_add(ad->naviframe);
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);
	elm_object_style_set(ctxpopup, "more/default");
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", ctxpopup_dismissed_cb, ctxpopup);

	win = elm_object_top_widget_get(ad->naviframe);
	evas_object_smart_callback_add(win, "rotation,changed", move_more_ctxpopup, ctxpopup);

	preference_get_int("prediction", &pred);
	if (pred)
		elm_ctxpopup_item_append(ctxpopup, "Turn off predictions", NULL, ctxpopup_item_select_cb, ad);
	else
		elm_ctxpopup_item_append(ctxpopup, "Turn on predictions", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "Font Size", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "Clear History", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "Help", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "About", NULL, ctxpopup_item_select_cb, ad);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	move_more_ctxpopup(ctxpopup, ctxpopup, NULL);
	evas_object_show(ctxpopup);
	if(ad->pred_idler)
	  {
	    ecore_idler_del(ad->pred_idler);
	    ad->pred_idler = NULL;
	  }
	evas_object_hide(ad->ctxpopup);
}
