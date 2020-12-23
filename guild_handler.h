#pragma once

#include <aegis.hpp>
#include "download/downloader.h"

#include <regex>
#include <unordered_map>

using namespace LSW::v5;

constexpr size_t maximum_file_size_global = 8e6;
constexpr size_t maximum_rules_vector_per_chat_per_type = 10;
const std::string default_command_start = "lsw/bb";
constexpr size_t safe_msg_limit = 1998;
const std::string version = "V3.0.0-rc7";

const auto default_color = 0xA321FF;
const size_t str_max_len_embed_default = 60;
const std::regex regex_link("http(?:s?)://([\\w_-]+(?:(?:\\.[\\w_-]+)+))([\\w.,@?^=%&:/~+#-]*[\\w@?^=%&/~+#-])?");

const unsigned long long mee_dev = 280450898885607425;
const unsigned long long bot_ids[2] = { 524361154294972416, /*beta:*/752301949528834068 };
const unsigned long long allowed_full_servers[] = {
	281976851142803456, // LSW OFFICIAL
	508808506569261100  // TESTS
};
const char recycle_emoji[] = u8"%E2%99%BB%EF%B8%8F";
const char delete_emoji[] = u8"%E2%9D%8C";



enum class handle_modes {NONE, DELETE_SOURCE, COPY_SOURCE, CUT_SOURCE, CUT_AND_REFERENCE_BACK_EMBED};
enum class lock_shared_mode {SHARED, EXCLUSIVE};

enum class help_type{ALL, GLOBAL, LINK, FILE, TEXT, MODES};


/// <summary>
/// Interpret an argument as handle_modes
/// </summary>
/// <param name="{handle_modes}">It will save_nolock the value here if valid.</param>
/// <param name="{std::string}">The string to interpret.</param>
/// <returns>{bool} True if success (match).</returns>
bool interpret_handle_modes(handle_modes&, const std::string&);
/// <summary>
/// Convert handle_modes to string
/// </summary>
/// <param name="{handle_modes}">Mode.</param>
/// <returns>{std::string} String.</returns>
std::string interpret_string_handle_modes(const handle_modes&);

unsigned long long stdstoulla(std::string);
std::string transform_weirdo(const std::string);

/*class delayedTasks {
	struct savedone {
		std::thread thr;
		bool done = false;
		std::chrono::seconds limit;
	};
	std::thread work;
	std::vector<savedone> thrs;
	std::mutex secure;
	bool die = false;

	void release_auto();
public:
	delayedTasks();
	~delayedTasks();
	void add_task(std::function<void(void)>, std::chrono::seconds);
	size_t size() const;
};

inline delayedTasks taskG;*/


class LockGuardShared {
	std::shared_mutex& mut;
	bool shared = false, locked = false;
public:
	LockGuardShared(std::shared_mutex&, const lock_shared_mode = lock_shared_mode::EXCLUSIVE);
	~LockGuardShared();

	// auto unlocks itself to lock in new model
	void lock();

	// auto unlocks itself to lock in new model
	void lock_shared();

	// unlock any or all
	void unlock();
};



struct chat_configuration {
	// common
	struct embed_configuration {
		bool show_thumbnail = false; // thumbnail on the right top side
		std::string thumbnail_override; // overrides the thumbnail with this
		bool show_user_information = true; // show the user data (who sent)
		unsigned max_text_length = str_max_len_embed_default; // max text length in a embed (can be up to 2048, 0 means no text)
		bool automatically_markdown_text = false; // text in embed ```md\nmarkdown-ed```?
		bool add_referece_link_to_message_copy = false; // add a ->see fully here<- like thing.
		bool add_referece_chat_to_message_copy = false; // add a <#CHANNELID> thing.
		std::string reference_link_back_text = "`-> source <-`";
		unsigned long long chat_to_send_embed = 0; // where should it send embed?
		int priority = 0; // if many triggers, higher priority sets embed stuff.

		std::vector<std::string> auto_react_with; // emojis to automatically react

		void from_json(const nlohmann::json&);
		nlohmann::json to_json() const;

		// if same, first does the job.
		void set_if_higher_priority(const embed_configuration&);
	};

	// triggers if link regex is triggered
	struct links_configuration {
		handle_modes handling = handle_modes::NONE; // if someone sends a link, do nothing, delete, copy and leave there, cut, or cut and reference back as embed?
		bool silence_links = false; // <link> or leave it link (with preview)? [silence = <>]
		bool silence_codeblock = false; // ```code block``` or `code block`
		embed_configuration embed_settings; // if handling == CUT_AND_REFERENCE_BACK_EMBED...
		std::string has_to_match_this; // if set, regex with this (DO TRY-CATCH!)
		bool use_regex = false; // simple find or full regex? true = regex.
		bool inverse_regex = false; // only works if regex
		unsigned long long chat_to_send = 0; // if everything match, where to copy? if none set and one CUT setting is set, it will handle as DELETE.
		std::vector<std::string> react_to_source; // what emojis to react to source (if still exists)
		std::vector<std::string> react_to_copy; // what emojis to react to copy (where it copies)

		void from_json(const nlohmann::json&);
		nlohmann::json to_json() const;

		links_configuration() = default;
		links_configuration(const nlohmann::json&);
	};

	// triggers if there's any ATTACHMENT)
	struct files_configuration {
		handle_modes handling = handle_modes::NONE; // if someone sends a file, do nothing, delete, copy and leave there, cut, or cut and reference back as embed?
		embed_configuration embed_settings; // if handling == CUT_AND_REFERENCE_BACK_EMBED...
		std::string has_to_match_this; // if set, regex with this [FILENAME] (DO TRY-CATCH!)
		bool use_regex = false; // simple find or full regex? true = regex.
		bool inverse_regex = false; // only works if regex
		size_t minimum_file_size = 0; // minimum file size filter (if max and min are the same, filters only a specific file size)
		size_t maximum_file_size = maximum_file_size_global; // maximum file size filter (if max and min are the same, filters only a specific file size)
		unsigned long long chat_to_send = 0; // if everything match, where to copy? if none set and one CUT setting is set, it will handle as DELETE.
		std::vector<std::string> react_to_source; // what emojis to react to source (if still exists)
		std::vector<std::string> react_to_copy; // what emojis to react to copy (where it copies)

		void from_json(const nlohmann::json&);
		nlohmann::json to_json() const;

		files_configuration() = default;
		files_configuration(const nlohmann::json&);
	};

	// triggers with ANY text (including links)
	struct texts_configuration {
		handle_modes handling = handle_modes::NONE; // if someone sends a file, do nothing, delete, copy and leave there, cut, or cut and reference back as embed?
		bool silence_codeblock = false; // ```code block``` or `code block`
		embed_configuration embed_settings; // if handling == CUT_AND_REFERENCE_BACK_EMBED...
		std::string has_to_match_this; // if set, regex with this (DO TRY-CATCH!)
		bool use_regex = false; // simple find or full regex? true = regex.
		bool inverse_regex = false; // only works if regex
		bool no_links_please = false; // cancel trigger if contains link?
		bool no_files_please = false; // cancel trigger if contains file?
		unsigned long long chat_to_send = 0; // if everything match, where to copy? if none set and one CUT setting is set, it will handle as DELETE.
		std::vector<std::string> react_to_source; // what emojis to react to source (if still exists)
		std::vector<std::string> react_to_copy; // what emojis to react to copy (where it copies)

		void from_json(const nlohmann::json&);
		nlohmann::json to_json() const;

		texts_configuration() = default;
		texts_configuration(const nlohmann::json&);
	};

	// THESE 3, RESPECT maximum_rules_vector_per_chat_per_type!
	std::vector<links_configuration> link_configurations; // there can be multiple triggers to same message that could actually copy stuff to many places.
	std::vector<files_configuration> file_configurations; // there can be multiple triggers to same message that could actually copy stuff to many places.
	std::vector<texts_configuration> text_configurations; // there can be multiple triggers to same message that could actually copy stuff to many places.
	std::shared_mutex secure;

	void from_json(const nlohmann::json&);
	nlohmann::json to_json() const;

	chat_configuration() {}
	chat_configuration(const chat_configuration&);
	chat_configuration(chat_configuration&&);
	chat_configuration(const nlohmann::json&);

	void operator=(const chat_configuration&);
	void operator=(chat_configuration&&);
};

struct guild_data { // see copy, move, json stuff, construct, if changed
	std::string command_alias;
	bool on_error_delete_source = false;
	long long guild_id = 0;
	std::vector<std::pair<unsigned long long, chat_configuration>> chats;

	std::shared_mutex mut;

	guild_data() = default;
	guild_data(guild_data&&) noexcept;
	guild_data(const guild_data&);
	guild_data(const nlohmann::json&);
	guild_data(const unsigned long long);
	~guild_data();

	void operator=(guild_data&&) noexcept;
	void operator=(const guild_data&);

	void from_json(const nlohmann::json&);
	nlohmann::json to_json() const;

	chat_configuration& get_chat(const unsigned long long);

	bool save_nolock();
	void load_nolock();
};


class GuildHandle {
	guild_data data;
	std::shared_ptr<aegis::core> core;
	std::shared_ptr<spdlog::logger> logg;

	/// <summary>
	/// Assist function for command
	/// </summary>
	/// <param name="{std::unordered_map}">Arguments.</param>
	/// <param name="{size_t}">Offset (this being one after 'react' word).</param>
	/// <param name="{std::vector}">SOURCE Emojis.</param>
	/// <param name="{std::vector}">COPY/DEST Emojis.</param>
	/// <returns>{std::string} Resulting message.</returns>
	std::string _command_react(std::unordered_map<size_t, std::string>&, const size_t, std::vector<std::string>&, std::vector<std::string>&);/// <summary>
	/// Assist function for command
	/// </summary>
	/// <param name="{std::unordered_map}">Arguments.</param>
	/// <param name="{size_t}">Offset (this being one after 'react' word).</param>
	/// <param name="{embed_configuration}">The EMBED.</param>
	/// <returns>{std::string} Resulting message.</returns>
	std::string _command_embed(std::unordered_map<size_t, std::string>&, const size_t, chat_configuration::embed_configuration&);

	/// <summary>
	/// If command, this
	/// </summary>
	/// <param name="{std::string}">The content (withou the trigger itself).</param>
	/// <param name="{aegis::channel}">The channel where it came from.</param>
	/// <param name="{aegis::gateway::objects::message}">The triggered message.</param>
	void command(const std::string&, aegis::channel&, aegis::gateway::objects::message&);

	/// <summary>
	/// Filters markdown and links.
	/// </summary>
	/// <param name="std::string">Input.</param>
	/// <param name="bool">Filter `? (they will be replaced with ').</param>
	/// <param name="bool">Filter link preview?</param>
	/// <returns>{std::string} Filtered string.</returns>
	std::string filter(std::string, const bool, const bool);

	/// <summary>
	/// Generate new message.
	/// </summary>
	/// <param name="bool">Well done?</param>
	/// <param name="ULL">Channel ID.</param>
	/// <param name="message">The message to copy.</param>
	/// <param name="bool">(filter call) Filter `? (they will be replaced with ').</param>
	/// <param name="bool">(filter call) Filter link preview?</param>
	/// <returns>The message copied.</returns>
	Tools::Future<aegis::gateway::objects::message> copy_message_to(bool&, const unsigned long long, const aegis::gateway::objects::message&, const bool, const bool);

	/// <summary>
	/// Generate new message.
	/// </summary>
	/// <param name="bool">Well done?</param>
	/// <param name="ULL">Channel ID.</param>
	/// <param name="std::string">Message CONTENT.</param>
	/// <param name="std::string">Author FULL STRING (Name#DISC).</param>
	/// <param name="bool">(filter call) Filter `? (they will be replaced with ').</param>
	/// <param name="bool">(filter call) Filter link preview?</param>
	/// <returns>The message copied.</returns>
	Tools::Future<aegis::gateway::objects::message> copy_message_to(bool&, const unsigned long long, const std::string, const std::string, const bool, const bool);

	/// <summary>
	/// Generate new message.
	/// </summary>
	/// <param name="bool">Well done?</param>
	/// <param name="ULL">Channel ID.</param>
	/// <param name="std::string">FILENAME.</param>
	/// <param name="std::string">DATA.</param>
	/// <returns>The message with attachment.</returns>
	Tools::Future<aegis::gateway::objects::message> copy_file_to(bool&, const unsigned long long, const std::string&, const std::string&);

	/// <summary>
	/// <para>Generate new message.</para>
	/// <para>AUTOMATICALLY REACTS TO IT BASED ON EMBED_CONFIGURATION!</para>
	/// </summary>
	/// <param name="bool">Well done?</param>
	/// <param name="message">Message source (FROM EVENT).</param>
	/// <param name="embed_configuration">Embed configuration.</param>
	/// <param name="ULL">If set to link back (via embed config), what channel ID to link to? (where the copy message has been sent).</param>
	/// <param name="ULL">If set to link back (via embed config), what message ID to link to? (where the copy message has been sent).</param>
	/// <returns>Embed message.</returns>
	Tools::Future<aegis::gateway::objects::message> copy_generate_embed_auto_everything(bool&, const aegis::gateway::objects::message&, const chat_configuration::embed_configuration&, const unsigned long long, const unsigned long long, const std::string/*, const bool = false*/);

	/*void load_file();
	void save_file();*/
public:
	GuildHandle(std::shared_ptr<aegis::core>, aegis::snowflake);
	~GuildHandle();

	bool operator==(const aegis::snowflake&);

	void handle(aegis::channel&, aegis::gateway::objects::message);
	void handle(aegis::channel&, const unsigned long long, aegis::gateway::objects::emoji);
};