#pragma once

#include "Log.h"
#include "HighscoreRecord.h"
#include "UserSettings.h"
#include <vector>

void CreateDBAndTablesIfNeeded();

std::vector<HighscoreRecord> GetAllHighscores();
void AddHighscore(const char* name, int score);

UserSettings GetUserSettings();
void UpdateUserSettings(const UserSettings& setting);
