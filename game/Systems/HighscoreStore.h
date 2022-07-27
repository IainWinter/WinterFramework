#pragma once

#include "app/System.h"
#include "Events.h"
#include "HighscoreRecord.h"
#include "sql/sqlite3.h"

struct System_HighscoreStore : System<System_HighscoreStore>
{
	void Init()
	{
		CreateDB();
	}

	void OnAttach()
	{
		Attach<event_SubmitHighscore>();
		Attach<event_RequestHighscores>();
	}

	void on(event_SubmitHighscore& e)
	{
		AddScoreToDB(e.name.c_str(), e.score);
	}

	void on(event_RequestHighscores& e)
	{
		for (HighscoreRecord& record : GetAllRecords())
		{
			SendToRoot(event_RespondHighscore{ record });
		}
	}

private:

	void CreateDB()
	{
		const char* sql = ""
			"CREATE TABLE IF NOT EXISTS highscores ("
				"name  VARCHAR(10) NOT NULL,"
				"score INTEGER     NOT NULL"
			");";

		sqlite3* conn;
		sqlite3_stmt* stmt;
		sqlite3_open("local.db", &conn);
		sqlite3_prepare(conn, sql, -1, &stmt, nullptr);
		int result = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		sqlite3_close(conn);

		if (result != SQLITE_DONE)
		{
			printf("Failed to create highscores table\n");
		}
	}

	void AddScoreToDB(const char* name, int score)
	{
		// name is limited to 5 chars

		char sql[1000];
		memset(sql, 0, 1000);
		sprintf(sql, "INSERT INTO highscores (name, score) VALUES('%s', %d);", name, score);

		sqlite3* conn;
		sqlite3_stmt* stmt;
		sqlite3_open("local.db", &conn);
		sqlite3_prepare(conn, sql, -1, &stmt, nullptr);
		int result = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		sqlite3_close(conn);

		if (result != SQLITE_DONE)
		{
			printf("Failed to write score to db\n");
		}
	}

	std::vector<HighscoreRecord> GetAllRecords()
	{
		std::vector<HighscoreRecord> records;

		const char* sql = "SELECT name, score, ROW_NUMBER () OVER (ORDER BY score DESC) AS score_order FROM highscores;";

		sqlite3* conn;
		sqlite3_stmt* stmt;
		sqlite3_open("local.db", &conn);
		int r = sqlite3_prepare(conn, sql, -1, &stmt, nullptr);

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			HighscoreRecord record;
			record.name  = (const char*)sqlite3_column_text(stmt, 0);
			record.score = sqlite3_column_int(stmt, 1);
			record.order = sqlite3_column_int(stmt, 2);

			records.push_back(record);
		}

		sqlite3_finalize(stmt);
		sqlite3_close(conn);

		return records;
	}
};