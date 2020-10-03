#pragma once

#include "shared.h"

#include <aegis.hpp>
#include <thread>
#include <regex>

#define MAX_FILE_SIZE 8e6

const std::string main_cmd = "lsw/bb";
const unsigned long long mee_dev = 280450898885607425; // myself, for debugging and help
const unsigned long long bot_ids[2] = { 524361154294972416, /*beta:*/752301949528834068 };
const std::string version = "V2.0.0.0a";

const char recycle_emoji[] = u8"%E2%99%BB%EF%B8%8F";
const char delete_emoji[] = u8"%E2%9D%8C";

using namespace LSW::v5;



const auto default_color = 0xA321FF;
const size_t str_max_len_embed = 60;
const std::regex regex_link("http(?:s?)://([\\w_-]+(?:(?:\\.[\\w_-]+)+))([\\w.,@?^=%&:/~+#-]*[\\w@?^=%&/~+#-])?");


inline const std::string transformWeirdo(const std::string in)
{
	std::string out;
	for (auto& i : in) {
		char temp[5];
		sprintf_s(temp, "%c%X", '%', (unsigned char)(i + sizeof(char) / 2));
		out += temp;
	}
	return out;
}



/*class handle_in_steps {
	std::vector<std::function<void(void)>> list;
	std::mutex m;
public:
	void add(std::function<void(void)>);
	bool task();
};*/



class chat_config {
	aegis::channel* get_channel_safe(unsigned long long, std::shared_ptr<aegis::core>&) const;

	unsigned long long this_guild_orig = 0;

	// show ref link, channel id, message id, user id, user avatar url, user name, text, image preview
	nlohmann::json embed_link_gen(const bool, const unsigned long long, const unsigned long long, const unsigned long long, const std::string, const std::string, const std::string = "", const std::string = "") const;
	// if you want just the embed with no linking and such, call this. Args: user_id, user_url, user_name, desc = "", imgurl = ""
	nlohmann::json embed_link_gen(const unsigned long long, const unsigned long long, const std::string, const std::string, const std::string = "", const std::string = "") const;

	void handle_poll(slowflush_end&) const;
	std::string _fix_content(const std::string&) const;
public:
	bool link_ref_back = false;
	bool link_ref_show_ref = true;
	unsigned long long links_orig = 0;
	std::vector<std::pair<std::string, unsigned long long>> link_overwrite_contains;

	bool files_ref_back = false;
	bool files_ref_show_ref = true;
	unsigned long long files_orig = 0;
	std::vector<std::pair<std::string, unsigned long long>> files_overwrite_contains;

	bool nlf_ref_back = false; // nlf == not link (or) file
	bool nlf_ref_show_ref = true;
	unsigned long long nlf_orig = 0;
	std::vector<std::pair<std::string, unsigned long long>> nlf_overwrite_contains;
	mutable std::mutex mute;

	std::vector<std::string> poll_emoji_overwrite;
	std::string overwrite_thumbnail; // if any, overwrite.
	bool case_sensitive = false;
	bool use_regex = false;
	bool show_user_thumbnail = true;
	bool dont_use_md = false;
	bool autopoll = false; // automatically react to the latest message.
	unsigned long long embed_fallback = 0; // if set, all messages will be embed and files will be thrown here. if 0, cancel

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

	//std::thread hs_thr;
	//handle_in_steps hs;

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
	void handle(aegis::channel&, const unsigned long long, aegis::gateway::objects::emoji);
};