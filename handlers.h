#pragma once

#include "shared.h"

#include <aegis.hpp>
#include <thread>

#define MAX_FILE_SIZE 8e6

const std::string main_cmd = "lsw/bb";
const unsigned long long mee_dev = 280450898885607425; // myself, for debugging and help

using namespace LSW::v5;




class chat_config {
	aegis::channel* get_channel_safe(unsigned long long, std::shared_ptr<aegis::core>&) const;

	unsigned long long this_guild_orig = 0;
public:
	unsigned long long links_orig = 0;
	std::vector<std::pair<std::string, unsigned long long>> link_overwrite_contains;
	unsigned long long files_orig = 0;
	std::vector<std::pair<std::string, unsigned long long>> files_overwrite_contains;
	mutable std::mutex mute;

	chat_config() = default;
	chat_config(const chat_config&);

	void load(nlohmann::json, unsigned long long);
	nlohmann::json export_config();

	chat_config& operator=(const chat_config&);


	// this should be called from pendency thread handle
	void handle_message(std::shared_ptr<aegis::core>, aegis::gateway::objects::message) const;
};

struct guild_data {
	std::string command_alias;
	std::vector<unsigned long long> admin_tags;
	std::map<unsigned long long, chat_config> chats;

	std::mutex mut;

	guild_data() = default;
	guild_data(const guild_data&);

	guild_data& operator=(const guild_data&);
};

class GuildHandle {
	guild_data data;
	std::shared_ptr<aegis::core> core;
	std::shared_ptr<spdlog::logger> logg;
	unsigned long long guildid = 0;

	//std::vector<aegis::gateway::objects::message> pendencies;
	//std::thread pendency_handler;
	//std::mutex pendency_mutex;
	bool keep_run = false;

	//void handle_pendencies();

	void command(std::vector<std::string>, aegis::channel&);

	bool has_admin_rights(aegis::guild&, aegis::user&);

	nlohmann::json gen_json();

	void save_settings();
	void load_settings();
public:
	GuildHandle(std::shared_ptr<aegis::core>, aegis::snowflake);
	//GuildHandle(const GuildHandle&);

	~GuildHandle();

	void end();

	GuildHandle& start();

	//GuildHandle& operator=(const GuildHandle&);

	// is this this guild id?
	bool amI(aegis::snowflake);

	void handle(aegis::channel&, aegis::gateway::objects::message);
};