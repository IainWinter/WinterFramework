#include "Data.h"
#include "sql/sqlite3.h"

int error_check(int result)
{
	if (   result != SQLITE_OK
		&& result != SQLITE_ROW
		&& result != SQLITE_DONE)
	{
		log_io("Failed sql query. sqlite3 error code: %d", result);
	}

	return result;
}

#define sql(stmt) error_check(stmt)

void exec_query(sqlite3* conn, const char* fmt, va_list args)
{
	char sql[10000];
	memset(sql, 0, 10000);

	vsprintf(sql, fmt, args);

	sqlite3_stmt* stmt;
	sql(sqlite3_prepare(conn, sql, -1, &stmt, nullptr));
	
	int result = sql(sqlite3_step(stmt));
	sql(sqlite3_finalize(stmt));

	if (result != SQLITE_DONE)
	{
		log_io("Failed sql query. sqlite3 error code: %d. Query: '%s'", result, sql);
	}
}

void run_query(sqlite3* conn, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	exec_query(conn, fmt, args);
	va_end(args);
}

void run_single_query(const char* fmt, ...)
{
	sqlite3* conn;
	sql(sqlite3_open("local.db", &conn));

	va_list args;
	va_start(args, fmt);
	exec_query(conn, fmt, args);
	va_end(args);

	sql(sqlite3_close(conn));
}

void CreateDBAndTablesIfNeeded()
{
	sqlite3* conn;
	sql(sqlite3_open("local.db", &conn));
	
	run_query(conn, ""
		"CREATE TABLE IF NOT EXISTS highscores ("
			"name  VARCHAR(10) NOT NULL,"
			"score INTEGER     NOT NULL"
		");"
	);
	
	run_query(conn, ""
		"CREATE TABLE IF NOT EXISTS user_settings ("
			"last_name       VARCHAR(10) NOT NULL,"    // name to fill game over with
			"fullscreen_mode INT         NOT NULL,"    // 0, 1, or 2
			"vsync_mode      INT         NOT NULL,"    // 0 or 1
			"volume_music    REAL        NOT NULL,"    // 0 - 1.0
			"volume_effects  REAL        NOT NULL"     // 0 - 1.0
		");"
	);

	sql(sqlite3_close(conn));
}

std::vector<HighscoreRecord> GetAllHighscores()
{
	std::vector<HighscoreRecord> records;

	const char* sql = ""
		"SELECT name, score, ROW_NUMBER () OVER (ORDER BY score DESC) AS score_order "
		"FROM highscores;";

	sqlite3* conn;
	sql(sqlite3_open("local.db", &conn));

	sqlite3_stmt* stmt;
	sql(sqlite3_prepare(conn, sql, -1, &stmt, nullptr));

	while (sql(sqlite3_step(stmt)) == SQLITE_ROW)
	{
		HighscoreRecord record;
		record.name  = (const char*)sqlite3_column_text(stmt, 0);
		record.score =              sqlite3_column_int (stmt, 1);
		record.order =              sqlite3_column_int (stmt, 2);

		records.push_back(record);
	}

	sql(sqlite3_finalize(stmt));
	sql(sqlite3_close(conn));

	return records;
}

void AddHighscore(const char* name, int score)
{
	run_single_query("INSERT INTO highscores (name, score) VALUES('%s', %d);", name, score);
}

UserSettings GetUserSettings()
{
	UserSettings record;

	const char* sql = "SELECT * FROM user_settings;";

	sqlite3* conn;
	sql(sqlite3_open("local.db", &conn));

	sqlite3_stmt* stmt;
	sql(sqlite3_prepare(conn, sql, -1, &stmt, nullptr));

	if (sql(sqlite3_step(stmt)) == SQLITE_ROW)
	{
		record.name           = (const char*)sqlite3_column_text  (stmt, 0);
		record.fullscreenMode =              sqlite3_column_int   (stmt, 1);
		record.vsyncMode      =              sqlite3_column_int   (stmt, 2);
		record.musicVol       =              sqlite3_column_double(stmt, 3);
		record.effectVol      =              sqlite3_column_double(stmt, 4);
	}

	else
	{
		// default settings

		run_query(conn, "INSERT INTO user_settings "
			"(last_name, fullscreen_mode, vsync_mode, volume_music, volume_effects) "
			"VALUES ('%s', %d, %d, %f, %f);",
			record.name.c_str(),
			record.fullscreenMode,
			record.vsyncMode,
			record.musicVol,
			record.effectVol
		);
	}

	sql(sqlite3_finalize(stmt));
	sql(sqlite3_close(conn));

	return record;
}

void UpdateUserSettings(const UserSettings& setting)
{
	run_single_query("UPDATE user_settings SET "
		"last_name       = '%s',"
		"fullscreen_mode = %d,"
		"vsync_mode      = %d,"
		"volume_music    = %f,"
		"volume_effects  = %f;",
		setting.name.c_str(),
		setting.fullscreenMode,
		setting.vsyncMode,
		setting.musicVol,
		setting.effectVol
	);
}
