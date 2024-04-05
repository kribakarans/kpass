#include <string.h>

#include "kpass.h"
#include "logit.h"

/**
 * Description:
 *   Check sqlite3 database integrity
 *
 * SQLite3 exec callback format:
 *   int callback(void* data, int num_columns, char** column_values, char** column_names);
 *
 * Parameters:
 *   @data         : The pointer to the data that was passed as the data parameter of sqlite3_exec
 *   @ncolumn      : The number of columns in the current row of the result set
 *   @column_value : An array of strings containing the values of the columns in the current row
 *   @column_name  : An array of strings containing the names of the columns in the result set
 *
 * Return value:
 *    @retval - RETSXS on success, RETERR on error
 *
 * If @retval is non-zero, sqlite3_exec() consider as failure and it will abort the query execution.
 */
static int cb_dbcheck(void *data, int ncolumn, char **column_value, char **column_name)
{
	int i = 0;

	for (i = 0; i < ncolumn; i++) {
		if (strcmp(column_name[i], "integrity_check") == 0) {
			if (strcmp(column_value[i], "ok") == 0) {
				return RETSXS;
			}
		}
	}

	return RETERR;
}

/**
 * Description:
 *   Run PRAGMA integrity_check.
 *   Examines the internal structure of the database file, looking for any corruption or inconsistencies in the data and structure.
 *   It checks the structure of b-tree pages, the structure of tables and indexes, and makes sure that pointers and references within
 *   the database are valid.
 *
 *   "ok"   : Indicates that the integrity check passed successfully and the database appears to be structurally sound.
 *   "error": Indicates that an error was found during the integrity check, suggesting that the database might be corrupted.
 *   "not an error":
 *            Indicates that the integrity check completed successfully but found some issues that don't necessarily indicate corruption.
 *            For example, this might include database pages that were allocated but never used.
 *
 * If callback returns non-zero, sqlite3_exec() consider as failure and it will abort the query execution.
 */
static int kpass_db_check(const char *path)
{
	int  retval = -1;
	sqlite3 *db = NULL;
	char   *err = NULL;
	char *query = NULL;

	do {
		return_if_null(path);

		if (sqlite3_open(path, &db) != SQLITE_OK) {
			kpass_error("sqlite3_open() failed: %s", sqlite3_errmsg(db));
			sqlite3_close(db);
			retval = RETERR;
			break;
		}

		query = "PRAGMA integrity_check;";
		if (sqlite3_exec(db, query, cb_dbcheck, NULL, &err) != SQLITE_OK) {
			kpass_error("sqlite3_exec() failed: %s", err);
			sqlite3_free(err);
			sqlite3_close(db);
			retval = RETERR;
			break;
		}

		sqlite3_close(db);
		retval = RETSXS;
	} while(0);

	logit_retval();

	return retval;
}

/**
 * Description:
 * - Open database
 * - Create required tables
 * - Close database
 *
 * Arguments:
 *   @path - database file path
 *
 * Return value:
 *   @retval - RETSXS on success
 *           - RETERR on failure
 */
int kpass_db_init(const char *path)
{
	int  retval = -1;
	sqlite3 *db = NULL;
	char   *err = NULL;
	char *query = NULL;

	do {
		return_if_null(path);

		logit("database: %s", path);

		/* Using existing database */
		if (access(path, F_OK|R_OK|W_OK) == RETSXS) {
			retval = RETSXS;
			break;
		}

		if (sqlite3_open(path, &db) != SQLITE_OK) {
			kpass_error("sqlite3_open() failed: %s", sqlite3_errmsg(db));
			sqlite3_close(db);
			retval = RETERR;
			break;
		}

		query = "CREATE TABLE KPASS_ENTRIES ("
		        "ID        INTEGER PRIMARY KEY, "
		        "TAG       TEXT, "
		        "NAME      TEXT, "
		        "USERNAME  TEXT, "
		        "PASSWORD  TEXT, "
		        "URL       TEXT, "
		        "NOTES     TEXT, "
		        "TIMESTAMP DATE DEFAULT (datetime('now','localtime')));";
		if (sqlite3_exec(db, query, NULL, NULL, &err) != SQLITE_OK) {
			kpass_error("sqlite3_exec() failed: %s", err);
			sqlite3_free(err);
			sqlite3_close(db);
			retval = RETERR;
			break;
		}

		sqlite3_close(db);
		retval = RETSXS;
	} while(0);

	logit_retval();

	return retval;
}

/**
 * Description:
 *   Open database connections
 *
 * Arguments:
 *   @path - database file path
 *
 * Return value:
 *   @retval - On success, return pointer to the sqlite3 db connection
 *           - On failure, return NULL
 */
sqlite3 *kpass_db_open(const char *path)
{
	sqlite3  *db = NULL;
	void *retval = NULL;

	do {
		if (path == NULL) {
			kpass_error("invalid database path");
			retval = NULL;
			break;
		}

		if (access(path, F_OK|R_OK|W_OK) != RETSXS) {
			kpass_error("database is not exist: %s", path);
			retval = NULL;
			break;
		}

		if (kpass_db_check(path) != RETSXS) {
			kpass_error("database is corrupted");
			retval = NULL;
			break;
		}

		if (sqlite3_open(path, &db) != SQLITE_OK) {
			kpass_error("sqlite3_open() failed: %s", sqlite3_errmsg(db));
			sqlite3_close(db);
			retval = NULL;
			break;
		}

		logit("%p", db);
		retval = db;
	} while(0);

	return retval;
}

void kpass_db_close(sqlite3 *db)
{
	if (db != NULL) {
		logit("%p", db);
		sqlite3_close(db);
	}

	return;
}

/**
 * Description:
 *   Commit SQL query.
 *
 * Retval:
 *   Return number of rows affected.
 */
int kpass_db_commit(char *fmt, ...)
{
	int  retval = -1;
	int  nitems = -1;
	va_list  ap = {0};
	char   *err = NULL;
	char *query = NULL;

	do {
		return_if_null(kpass.db);

		va_start(ap, fmt);

		query = sqlite3_vmprintf(fmt, ap);
		if (query == NULL) {
			kpass_error("sqlite3_vmprintf() failed: failed to allocate memory for sql statement (%s)", sqlite3_errmsg(kpass.db));
			retval = RETERR;
			break;
		}

		logit("%s", query);
		va_end(ap);

		retval = sqlite3_exec(kpass.db, query, NULL, NULL, &err);
		if (retval != SQLITE_OK) {
			kpass_error("sqlite3_exec() failed: %s", err);
			sqlite3_free(query);
			sqlite3_free(err);
			retval = RETERR;
			break;
		}

		nitems = sqlite3_changes(kpass.db);
		logit("affected %d rows", nitems);
		sqlite3_free(query);

		retval = nitems;
	} while(0);

	return retval;
}

/**
 * Similar to sqlite3_exec() with with SQL statement support.
 *
 * int kpass_db_exec(
 *   int (*callback)(void*, int, char**, char**), - Callback function
 *   void *arg,                                   - 1st argument to callback
 *   char *fmt, ...                               - Variadic SQL statement
 * );
 */
int kpass_db_exec(int (*callback)(void*, int, char**, char**), void *arg, char *fmt, ...)
{
	int  retval = -1;
	va_list  ap = {0};
	char   *err = NULL;
	char *query = NULL;

	do {
		return_if_null(kpass.db);

		va_start(ap, fmt);

		query = sqlite3_vmprintf(fmt, ap);
		if (query == NULL) {
			kpass_error("sqlite3_vmprintf() failed: failed to allocate memory for sql statement (%s)", sqlite3_errmsg(kpass.db));
			retval = RETERR;
			break;
		}

		logit("%s", query);
		va_end(ap);

		retval = sqlite3_exec(kpass.db, query, callback, arg, &err);
		if (retval != SQLITE_OK) {
			kpass_error("sqlite3_exec() failed: %s", err);
			sqlite3_free(query);
			sqlite3_free(err);
			retval = RETERR;
			break;
		}

		//nitems = sqlite3_changes(kpass.db);
		//logit("affected %d rows", nitems);
		sqlite3_free(query);
	} while(0);

	return retval;
}

/**
 * Callback of db_count()
 */
static int cb_dbcount(void *data, int argc, char **argv, char **cname)
{
    int *nrows = (int *)data;

    *nrows = atoi(argv[0]);

    return 0;
}

/**
 * Description:
 *   Return nrows of given SQL statement.
 *
 * Retval:
 *   Return number of rows.
 */
int kpass_db_count(char *fmt, ...)
{
	int  retval = -1;
	int  nitems = -1;
	va_list  ap = {0};
	char   *err = NULL;
	char *query = NULL;

	do {
		return_if_null(kpass.db);

		va_start(ap, fmt);

		query = sqlite3_vmprintf(fmt, ap);
		if (query == NULL) {
			kpass_error("sqlite3_vmprintf() failed: failed to allocate memory for sql statement (%s)", sqlite3_errmsg(kpass.db));
			retval = RETERR;
			break;
		}

		logit("%s", query);
		va_end(ap);

		retval = sqlite3_exec(kpass.db, query, cb_dbcount, &nitems, &err);
		if (retval != SQLITE_OK) {
			kpass_error("sqlite3_exec() failed: %s", err);
			sqlite3_free(query);
			sqlite3_free(err);
			retval = RETERR;
			break;
		}

		logit("nitems: %d", nitems);
		sqlite3_free(query);
		retval = nitems;
	} while(0);

	return retval;
}

/* EOF */
