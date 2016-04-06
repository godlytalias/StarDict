#include "dict.h"

static void
_app_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	sqlite3 *db = NULL;
	char *err_msg;

	   char *db_path = (char*)malloc(200);
	   char *res_path = app_get_data_path();
	   sprintf(db_path, "%sdictdata.db", res_path);
	   sqlite3_open_v2(db_path, &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	   free(res_path);
	   free(db_path);
	   sqlite3_exec(db, query, func, data, &err_msg);
	   sqlite3_free(err_msg);
	   sqlite3_close(db);
	   db = NULL;
}

static int
_sq_func(void *data, int argc, char **argv, char **azColName)
{
	return 0;
}

void
_init_tables()
{
	char query[128];
	sprintf(query, "create table if not exists keywords(words VARCHAR(1024));");
	_app_database_query(query, &_sq_func, NULL);
}

void
_clear_history_table(appdata_s *ad)
{
	char query[128];
	sprintf(query, "delete from keywords;");
	_app_database_query(query, &_sq_func, NULL);
}

void
_store_keyword(char *keyword)
{
	char query[128];
	sprintf(query, "delete from keywords where words like '%s';", keyword);
	_app_database_query(query, &_sq_func, NULL); // avoiding duplication, also we want to order
	                                             // keywords on basis of search
	sprintf(query, "insert into keywords VALUES('%s');", keyword);
	_app_database_query(query, &_sq_func, NULL);
	sprintf(query, "delete from keywords where words in (select words from keywords limit 1) and (select count(words) from keywords)>50;");
	_app_database_query(query, &_sq_func, NULL);
}

void _query_keywords(appdata_s *ad, int func(void*,int,char**,char**))
{
	char query[128];
	sprintf(query, "select * from keywords order by ROWID desc;");
	_app_database_query(query, func, ad);
}
