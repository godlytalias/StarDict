#include "dictwidget.h"
#include <sqlite3.h>
#define TABLE_NAME "words"

static void
_word_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	sqlite3 *db = NULL;
	char *err_msg;

	   char *db_path = (char*)malloc(200);
	   char *res_path = app_get_data_path();
	   sprintf(db_path, "%swords.db", res_path);
	   sqlite3_open_v2(db_path, &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	   free(res_path);
	   free(db_path);
	   sqlite3_exec(db, query, func, data, &err_msg);
	   sqlite3_free(err_msg);
	   sqlite3_close(db);
	   db = NULL;
}

void
_move_word_db() //as we want db file to be in data folder to write access
{
	FILE *fsrc, *fdst;
	char buf[16384];
	char src[PATH_MAX], dst[PATH_MAX];
	size_t num;
	char *respath = app_get_resource_path();
	char *dstpath = app_get_data_path();
	sprintf(src, "%swords.db", respath);
	sprintf(dst, "%swords.db", dstpath);
	fdst = fopen(dst, "rb");
	if (!fdst)
	  {
		fdst = fopen(dst, "wb");
		fsrc = fopen(src, "rb");
		while ((num = fread(buf, 1, sizeof(buf), fsrc)) > 0)
		  {
		    fwrite(buf, 1, num, fdst);
		  }
		fclose(fsrc);
	  }
	fclose(fdst);
	free(respath);
	free(dstpath);
}

static int
_sq_func(void *data, int argc, char **argv, char **azColName)
{
	return 0;
}

void
_get_word_of_day(int func(void*,int,char**,char**), void *data)
{
	char query[1024];
	sprintf(query, "select word,rowid from (select word,used,rowid from %s where used=0) order by RANDOM() LIMIT 1;", TABLE_NAME);
	_move_word_db();
	_word_database_query(query, func, data);
}

void
_set_word_used(int rowid)
{
	char query[1024];
	sprintf(query, "update %s set used=1 where rowid=%d", TABLE_NAME, rowid);
	_word_database_query(query, _sq_func, NULL);
}

void
_reset_word_usage()
{
	char query[1024];
	sprintf(query, "update %s set used=0", TABLE_NAME);
	_word_database_query(query, _sq_func, NULL);
}
