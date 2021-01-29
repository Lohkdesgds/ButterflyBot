#include "guild_handler.h"


bool interpret_handle_modes(handle_modes& m, const std::string& str)
{
	if (str == "NONE") {
		m = handle_modes::NONE;
		return true;
	}
	if (str == "DELETE_SOURCE") {
		m = handle_modes::DELETE_SOURCE;
		return true;
	}
	if (str == "COPY_SOURCE") {
		m = handle_modes::COPY_SOURCE;
		return true;
	}
	if (str == "CUT_SOURCE") {
		m = handle_modes::CUT_SOURCE;
		return true;
	}
	if (str == "CUT_AND_REFERENCE_BACK_EMBED") {
		m = handle_modes::CUT_AND_REFERENCE_BACK_EMBED;
		return true;
	}
	return false;
}

std::string interpret_string_handle_modes(const handle_modes& m)
{
	switch (m) {
	case handle_modes::NONE:
		return "NONE";
	case handle_modes::DELETE_SOURCE:
		return "DELETE_SOURCE";
	case handle_modes::COPY_SOURCE:
		return "COPY_SOURCE";
	case handle_modes::CUT_SOURCE:
		return "CUT_SOURCE";
	case handle_modes::CUT_AND_REFERENCE_BACK_EMBED:
		return "CUT_AND_REFERENCE_BACK_EMBED";
	}
	return "?";
}

unsigned long long stdstoulla(std::string str) {
	while (str.length() > 0) if (!std::isdigit(str[0])) str.erase(str.begin()); else break;
	if (str.length() == 0) return 0;
	unsigned long long t{};
	if (sscanf_s(str.c_str(), "%llu", &t) == 1) {
		return t;
	}
	return 0;
}

std::string transform_weirdo(const std::string in)
{
	std::string out;
	for (auto& i : in) {
		char temp[5];
		sprintf_s(temp, "%c%X", '%', (unsigned char)(i + sizeof(char) / 2));
		out += temp;
	}
	return out;
}

LockGuardShared::LockGuardShared(std::shared_mutex& s, const lock_shared_mode mode)
	: mut(s)
{
	switch (mode) {
	case lock_shared_mode::SHARED:
		lock_shared();
		return;
	case lock_shared_mode::EXCLUSIVE:
		lock();
		return;
	}
}

LockGuardShared::~LockGuardShared()
{
	unlock();
}

void LockGuardShared::lock()
{
	if (locked) unlock();
	mut.lock_shared();
	shared = false;
	locked = true;
}

void LockGuardShared::lock_shared()
{	
	if (locked) unlock();
	mut.lock_shared();
	shared = true;
	locked = true;
}

void LockGuardShared::unlock()
{
	if (shared) {
		mut.unlock_shared();
	}
	else {
		mut.unlock();
	}
	shared = false;
	locked = false;
}


void chat_configuration::embed_configuration::from_json(const nlohmann::json& j)
{
	if (j.count("show_thumbnail") && !j["show_thumbnail"].is_null())												show_thumbnail = j["show_thumbnail"].get<bool>();
	if (j.count("thumbnail_override") && !j["thumbnail_override"].is_null())										thumbnail_override = j["thumbnail_override"].get<std::string>();
	if (j.count("show_user_information") && !j["show_user_information"].is_null())									show_user_information = j["show_user_information"].get<bool>();
	if (j.count("max_text_length") && !j["max_text_length"].is_null())												max_text_length = j["max_text_length"].get<unsigned>();
	if (j.count("automatically_markdown_text") && !j["automatically_markdown_text"].is_null())						automatically_markdown_text = j["automatically_markdown_text"].get<bool>();
	if (j.count("add_referece_link_to_message_copy") && !j["add_referece_link_to_message_copy"].is_null())			add_referece_link_to_message_copy = j["add_referece_link_to_message_copy"].get<bool>();
	if (j.count("add_referece_chat_to_message_copy") && !j["add_referece_chat_to_message_copy"].is_null())			add_referece_chat_to_message_copy = j["add_referece_chat_to_message_copy"].get<bool>();
	if (j.count("reference_link_back_text") && !j["reference_link_back_text"].is_null())							reference_link_back_text = j["reference_link_back_text"].get<std::string>();
	if (j.count("chat_to_send_embed") && !j["chat_to_send_embed"].is_null())										chat_to_send_embed = j["chat_to_send_embed"].get<unsigned long long>();
	if (j.count("priority") && !j["priority"].is_null())															priority = j["priority"].get<int>();

	if (j.count("auto_react_with") && !j["auto_react_with"].is_null()) {
		for (const auto& _field : j["auto_react_with"])
			auto_react_with.push_back(_field);
	}
}

nlohmann::json chat_configuration::embed_configuration::to_json() const
{
	nlohmann::json j;

	j["show_thumbnail"] = show_thumbnail;
	j["thumbnail_override"] = thumbnail_override;
	j["show_user_information"] = show_user_information;
	j["max_text_length"] = max_text_length;
	j["automatically_markdown_text"] = automatically_markdown_text;
	j["add_referece_link_to_message_copy"] = add_referece_link_to_message_copy;
	j["add_referece_chat_to_message_copy"] = add_referece_chat_to_message_copy;
	j["reference_link_back_text"] = reference_link_back_text;
	j["chat_to_send_embed"] = chat_to_send_embed;
	j["priority"] = priority;

	for (const auto& _field : auto_react_with)
		j["auto_react_with"].push_back(_field);

	return j;
}

void chat_configuration::embed_configuration::set_if_higher_priority(const embed_configuration& c)
{
	if (c.priority > priority) *this = c;
}

void chat_configuration::links_configuration::from_json(const nlohmann::json& j)
{
	if (j.count("handling") && !j["handling"].is_null())															handling = static_cast<handle_modes>(j["handling"].get<int>());
	if (j.count("silence_links") && !j["silence_links"].is_null())													silence_links = j["silence_links"].get<bool>();
	if (j.count("silence_codeblock") && !j["silence_codeblock"].is_null())											silence_codeblock = j["silence_codeblock"].get<bool>();
	if (j.count("embed_settings") && !j["embed_settings"].is_null())												embed_settings.from_json(j["embed_settings"]);
	if (j.count("has_to_match_this") && !j["has_to_match_this"].is_null())											has_to_match_this = j["has_to_match_this"].get<std::string>();
	if (j.count("use_regex") && !j["use_regex"].is_null())															use_regex = j["use_regex"].get<bool>();
	if (j.count("inverse_regex") && !j["inverse_regex"].is_null())													inverse_regex = j["inverse_regex"].get<bool>();
	if (j.count("chat_to_send") && !j["chat_to_send"].is_null())													chat_to_send = j["chat_to_send"].get<unsigned long long>();

	if (j.count("react_to_source") && !j["react_to_source"].is_null()) {
		for (const auto& _field : j["react_to_source"])
			react_to_source.push_back(_field);
	}

	if (j.count("react_to_copy") && !j["react_to_copy"].is_null()) {
		for (const auto& _field : j["react_to_copy"])
			react_to_copy.push_back(_field);
	}
}

nlohmann::json chat_configuration::links_configuration::to_json() const
{
	nlohmann::json j;
	j["handling"] = static_cast<int>(handling);
	j["silence_links"] = silence_links;
	j["silence_codeblock"] = silence_codeblock;
	j["embed_settings"] = embed_settings.to_json();
	j["has_to_match_this"] = has_to_match_this;
	j["use_regex"] = use_regex;
	j["inverse_regex"] = inverse_regex;
	j["chat_to_send"] = chat_to_send;

	for (const auto& _field : react_to_source)
		j["react_to_source"].push_back(_field);

	for (const auto& _field : react_to_copy)
		j["react_to_copy"].push_back(_field);

	return j;
}

chat_configuration::links_configuration::links_configuration(const nlohmann::json& j)
{
	from_json(j);
}

void chat_configuration::files_configuration::from_json(const nlohmann::json& j)
{
	if (j.count("handling") && !j["handling"].is_null())															handling = static_cast<handle_modes>(j["handling"].get<int>());
	if (j.count("embed_settings") && !j["embed_settings"].is_null())												embed_settings.from_json(j["embed_settings"]);
	if (j.count("has_to_match_this") && !j["has_to_match_this"].is_null())											has_to_match_this = j["has_to_match_this"].get<std::string>();
	if (j.count("use_regex") && !j["use_regex"].is_null())															use_regex = j["use_regex"].get<bool>();
	if (j.count("inverse_regex") && !j["inverse_regex"].is_null())													inverse_regex = j["inverse_regex"].get<bool>();
	if (j.count("minimum_file_size") && !j["minimum_file_size"].is_null())											minimum_file_size = j["minimum_file_size"].get<size_t>();
	if (j.count("maximum_file_size") && !j["maximum_file_size"].is_null())											maximum_file_size = j["maximum_file_size"].get<size_t>();
	if (j.count("chat_to_send") && !j["chat_to_send"].is_null())													chat_to_send = j["chat_to_send"].get<unsigned long long>();

	if (j.count("react_to_source") && !j["react_to_source"].is_null()) {
		for (const auto& _field : j["react_to_source"])
			react_to_source.push_back(_field);
	}

	if (j.count("react_to_copy") && !j["react_to_copy"].is_null()) {
		for (const auto& _field : j["react_to_copy"])
			react_to_copy.push_back(_field);
	}
}

nlohmann::json chat_configuration::files_configuration::to_json() const
{
	nlohmann::json j;
	j["handling"] = static_cast<int>(handling);
	j["embed_settings"] = embed_settings.to_json();
	j["has_to_match_this"] = has_to_match_this;
	j["use_regex"] = use_regex;
	j["inverse_regex"] = inverse_regex;
	j["minimum_file_size"] = minimum_file_size;
	j["maximum_file_size"] = maximum_file_size;
	j["chat_to_send"] = chat_to_send;

	for (const auto& _field : react_to_source)
		j["react_to_source"].push_back(_field);

	for (const auto& _field : react_to_copy)
		j["react_to_copy"].push_back(_field);

	return j;
}

chat_configuration::files_configuration::files_configuration(const nlohmann::json& j)
{
	from_json(j);
}

void chat_configuration::texts_configuration::from_json(const nlohmann::json& j)
{
	if (j.count("handling") && !j["handling"].is_null())															handling = static_cast<handle_modes>(j["handling"].get<int>());
	if (j.count("silence_codeblock") && !j["silence_codeblock"].is_null())											silence_codeblock = j["silence_codeblock"].get<bool>();
	if (j.count("embed_settings") && !j["embed_settings"].is_null())												embed_settings.from_json(j["embed_settings"]);
	if (j.count("has_to_match_this") && !j["has_to_match_this"].is_null())											has_to_match_this = j["has_to_match_this"].get<std::string>();
	if (j.count("use_regex") && !j["use_regex"].is_null())															use_regex = j["use_regex"].get<bool>();
	if (j.count("inverse_regex") && !j["inverse_regex"].is_null())													inverse_regex = j["inverse_regex"].get<bool>();
	if (j.count("no_links_please") && !j["no_links_please"].is_null())												no_links_please = j["no_links_please"].get<bool>();
	if (j.count("no_files_please") && !j["no_files_please"].is_null())												no_files_please = j["no_files_please"].get<bool>();
	if (j.count("chat_to_send") && !j["chat_to_send"].is_null())													chat_to_send = j["chat_to_send"].get<unsigned long long>();

	if (j.count("react_to_source") && !j["react_to_source"].is_null()) {
		for (const auto& _field : j["react_to_source"])
			react_to_source.push_back(_field);
	}

	if (j.count("react_to_copy") && !j["react_to_copy"].is_null()) {
		for (const auto& _field : j["react_to_copy"])
			react_to_copy.push_back(_field);
	}
}

nlohmann::json chat_configuration::texts_configuration::to_json() const
{
	nlohmann::json j;
	j["handling"] = static_cast<int>(handling);
	j["silence_codeblock"] = silence_codeblock;
	j["embed_settings"] = embed_settings.to_json();
	j["has_to_match_this"] = has_to_match_this;
	j["use_regex"] = use_regex;
	j["inverse_regex"] = inverse_regex;
	j["no_links_please"] = no_links_please;
	j["no_files_please"] = no_files_please;
	j["chat_to_send"] = chat_to_send;

	for (const auto& _field : react_to_source)
		j["react_to_source"].push_back(_field);

	for (const auto& _field : react_to_copy)
		j["react_to_copy"].push_back(_field);

	return j;
}

chat_configuration::texts_configuration::texts_configuration(const nlohmann::json& j)
{
	from_json(j);
}

void chat_configuration::from_json(const nlohmann::json& j)
{
	if (j.count("link_configurations") && !j["link_configurations"].is_null()) {
		for (const auto& _field : j["link_configurations"])
			link_configurations.push_back(_field);
	}

	if (j.count("file_configurations") && !j["file_configurations"].is_null()) {
		for (const auto& _field : j["file_configurations"])
			file_configurations.push_back(_field);
	}

	if (j.count("text_configurations") && !j["text_configurations"].is_null()) {
		for (const auto& _field : j["text_configurations"])
			text_configurations.push_back(_field);
	}
}

nlohmann::json chat_configuration::to_json() const
{
	nlohmann::json j;

	for (const auto& _field : link_configurations)
		j["link_configurations"].push_back(_field.to_json());

	for (const auto& _field : file_configurations)
		j["file_configurations"].push_back(_field.to_json());

	for (const auto& _field : text_configurations)
		j["text_configurations"].push_back(_field.to_json());

	return j;
}

chat_configuration::chat_configuration(const chat_configuration& c)
{
	*this = c;
}

chat_configuration::chat_configuration(chat_configuration&& c)
{
	*this = std::move(c);
}

chat_configuration::chat_configuration(const nlohmann::json& j)
{
	from_json(j);
}

void chat_configuration::operator=(const chat_configuration& c)
{
	link_configurations = c.link_configurations;
	file_configurations = c.file_configurations;
	text_configurations = c.text_configurations;
}

void chat_configuration::operator=(chat_configuration&& c)
{
	link_configurations = std::move(c.link_configurations);
	file_configurations = std::move(c.file_configurations);
	text_configurations = std::move(c.text_configurations);
}

guild_data::guild_data(guild_data&& g) noexcept
{
	command_alias = std::move(g.command_alias);
	on_error_delete_source = std::move(g.on_error_delete_source);
	guild_id = std::move(g.guild_id);
	chats = std::move(g.chats);
}

guild_data::guild_data(const guild_data& g)
{
	command_alias = g.command_alias;
	on_error_delete_source = g.on_error_delete_source;
	guild_id = g.guild_id;
	chats = g.chats;
}

guild_data::guild_data(const nlohmann::json& j)
{
	from_json(j);
}

guild_data::guild_data(const unsigned long long id)
{
	guild_id = id;
	load_nolock();
}

guild_data::~guild_data()
{
	save_nolock();
}

void guild_data::operator=(guild_data&& g) noexcept
{
	command_alias = std::move(g.command_alias);
	on_error_delete_source = std::move(g.on_error_delete_source);
	guild_id = std::move(g.guild_id);
	chats = std::move(g.chats);
}

void guild_data::operator=(const guild_data& g)
{
	command_alias = g.command_alias;
	on_error_delete_source = g.on_error_delete_source;
	guild_id = g.guild_id;
	chats = g.chats;
}

void guild_data::from_json(const nlohmann::json& j)
{
	if (j.count("command_alias") && !j["command_alias"].is_null())													command_alias = j["command_alias"].get<std::string>();
	if (j.count("on_error_delete_source") && !j["on_error_delete_source"].is_null())								on_error_delete_source = j["on_error_delete_source"].get<bool>();

	if (j.count("chats") && !j["chats"].is_null()) {
		for (const auto& _field : j["chats"]) {
			std::pair<unsigned long long, nlohmann::json> xplic = _field.get<std::pair<unsigned long long, nlohmann::json>>();
			std::pair<unsigned long long, chat_configuration> don;
			don.first = xplic.first;
			don.second.from_json(xplic.second);
			chats.push_back(std::move(don));
		}
	}
}

nlohmann::json guild_data::to_json() const
{
	nlohmann::json j;

	j["command_alias"] = command_alias;
	j["on_error_delete_source"] = on_error_delete_source;

	for (const auto& _field : chats) {
		std::pair<unsigned long long, nlohmann::json> translate;
		translate.first = _field.first;
		translate.second = _field.second.to_json();
		j["chats"].push_back(translate);
	}

	return j;
}

chat_configuration& guild_data::get_chat(const unsigned long long id)
{
	{
		LockGuardShared luck(mut, lock_shared_mode::SHARED);

		for (auto& i : chats) {
			if (i.first == id) {
				return i.second;
			}
		}
	}
	LockGuardShared luck(mut, lock_shared_mode::EXCLUSIVE);
	{		
		std::pair<unsigned long long, chat_configuration> a;
		a.first = id;
		chats.push_back(a);
	}
	return chats.back().second;
}

bool guild_data::save_nolock()
{
	if (guild_id == 0) return false;
	CreateDirectoryA("servers", NULL);
	std::string pathh = "servers/" + std::to_string(guild_id) + "_data.bbv2";

	FILE* haha = nullptr;

	errno_t err = 0;
	if ((err = fopen_s(&haha, pathh.c_str(), "wb+")) == 0) {
		std::string cpy = to_json().dump(2);
		fwrite(cpy.c_str(), sizeof(char), cpy.length(), haha);
		fclose(haha);
		return true;
	}
	else {
		char buf[256];
		strerror_s(buf, err);
		std::cout << buf << std::endl;
	}
	return false;
}

void guild_data::load_nolock()
{
	if (guild_id == 0) return;
	std::string pathh = "servers/" + std::to_string(guild_id) + "_data.bbv2";

	FILE* haha = nullptr;
	if (fopen_s(&haha, pathh.c_str(), "rb") == 0) {
		std::string all_buf;
		char quickbuf[256];
		size_t u = 0;
		while (u = fread_s(quickbuf, 256, sizeof(char), 256, haha)) {
			for (size_t k = 0; k < u; k++) all_buf += quickbuf[k];
		}
		fclose(haha);

		if (all_buf.empty()) return;
		nlohmann::json j = nlohmann::json::parse(all_buf);

		from_json(j);
	}
}

std::string GuildHandle::_command_react(std::unordered_map<size_t, std::string>& processed, const size_t offset, std::vector<std::string>& react_to_source, std::vector<std::string>& react_to_copy)
{
	std::string res;
	if (processed[offset] == "source") {
		if (processed[1 + offset] == "add") {
			if (processed[2 + offset].empty()) {
				res = "Unknown emoji!";
			}
			else {
				std::string _filtered = processed[2 + offset];
				if (_filtered.size() > 2) {
					if (_filtered.front() == '<') {
						_filtered.erase(_filtered.begin());
						if (_filtered.back() == '>') _filtered.pop_back();
					}
					else {
						_filtered = transform_weirdo(_filtered);
					}
				}

				bool proceed = true;
				for (auto& i : react_to_source)
				{
					if (i == _filtered) {
						res = "Already in the list!";
						proceed = false;
						break;
					}
				}
				if (proceed) {
					react_to_source.push_back(_filtered);
					res = "Added " + _filtered + " to the list.";
				}
			}
		}
		else if (processed[1 + offset] == "remove") {
			if (processed[2 + offset].empty()) {
				res = "Unknown emoji!";
			}
			else {
				std::string _filtered = processed[2 + offset];
				if (_filtered.size() > 2) {
					if (_filtered.front() == '<') {
						_filtered.erase(_filtered.begin());
						if (_filtered.back() == '>') _filtered.pop_back();
					}
					else {
						_filtered = transform_weirdo(_filtered);
					}
				}

				bool proceed = true;
				for (size_t p = 0; p < react_to_source.size(); p++)
				{
					if (react_to_source[p] == _filtered)
					{
						react_to_source.erase(react_to_source.begin() + p);
						p--;
						proceed = false;
						res = "Removed successfully!";
					}
				}
				if (proceed) {
					res = "Couldn't find the emoji from list!";
				}
			}
		}
		else if (processed[1 + offset] == "remove_all") {
			react_to_source.clear();
		}
		else {
			res += "Reacting to source: ";
			for (auto& i : react_to_source) {
				res += i + ' ';
			}
		}
	}
	else if (processed[offset] == "destination") {
		if (processed[1 + offset] == "add") {
			if (processed[2 + offset].empty()) {
				res = "Unknown emoji!";
			}
			else {
				std::string _filtered = processed[2 + offset];
				if (_filtered.size() > 2) {
					if (_filtered.front() == '<') {
						_filtered.erase(_filtered.begin());
						if (_filtered.back() == '>') _filtered.pop_back();
					}
					else {
						_filtered = transform_weirdo(_filtered);
					}
				}

				bool proceed = true;
				for (auto& i : react_to_copy)
				{
					if (i == _filtered) {
						res = "Already in the list!";
						proceed = false;
						break;
					}
				}
				if (proceed) {
					react_to_copy.push_back(_filtered);
					res = "Added " + _filtered + " to the list.";
				}
			}
		}
		else if (processed[1 + offset] == "remove") {
			if (processed[2 + offset].empty()) {
				res = "Unknown emoji!";
			}
			else {
				std::string _filtered = processed[2 + offset];
				if (_filtered.size() > 2) {
					if (_filtered.front() == '<') {
						_filtered.erase(_filtered.begin());
						if (_filtered.back() == '>') _filtered.pop_back();
					}
					else {
						_filtered = transform_weirdo(_filtered);
					}
				}

				bool proceed = true;
				for (size_t p = 0; p < react_to_copy.size(); p++)
				{
					if (react_to_copy[p] == _filtered)
					{
						react_to_copy.erase(react_to_copy.begin() + p);
						p--;
						proceed = false;
						res = "Removed successfully!";
					}
				}
				if (proceed) {
					res = "Couldn't find the emoji from list!";
				}
			}
		}
		else if (processed[1 + offset] == "remove_all") {
			react_to_copy.clear();
		}
		else {
			res += "Reacting to source: ";
			for (auto& i : react_to_copy) {
				res += i + ' ';
			}
		}
	}
	else {
		res += "Reacting to source: ";
		for (auto& i : react_to_source) {
			res += i + ' ';
		}
		res += "\nReacting to copy/destination: ";
		for (auto& i : react_to_copy) {
			res += i + ' ';
		}
	}
	return res;
}

std::string GuildHandle::_command_embed(std::unordered_map<size_t, std::string>& processed, const size_t offset, chat_configuration::embed_configuration& embed)
{
	std::string res;

	if (processed[offset] == "show_thumbnail") {
		if (processed[1 + offset].empty()) {
			res = u8"Show_thumbnail value: " + std::string(embed.show_thumbnail ? "true" : "false");
		}
		else {
			embed.show_thumbnail = processed[1 + offset] == "true";
			res = u8"Show_thumbnail value set: " + std::string(embed.show_thumbnail ? "true" : "false");
		}
	}
	else if (processed[offset] == "thumbnail_override") {
		if (processed[1 + offset].empty()) {
			res = u8"Thumbnail_override value: " + embed.thumbnail_override;
		}
		else {
			embed.thumbnail_override = processed[1 + offset];
			res = u8"Thumbnail_override value set: " + embed.thumbnail_override;
		}
	}
	else if (processed[offset] == "show_user") {
		if (processed[1 + offset].empty()) {
			res = u8"Show_user value: " + std::string(embed.show_user_information ? "true" : "false");
		}
		else {
			embed.show_user_information = processed[1 + offset] == "true";
			res = u8"Show_user value set: " + std::string(embed.show_user_information ? "true" : "false");
		}
	}
	else if (processed[offset] == "max_length") {
		if (processed[1 + offset].empty()) {
			res = u8"Max_length value: " + std::to_string(embed.max_text_length);
		}
		else {
			embed.max_text_length = atoi(processed[1 + offset].c_str());
			res = u8"Max_length value set: " + std::to_string(embed.max_text_length);
		}
	}
	else if (processed[offset] == "markdown_text") {
		if (processed[1 + offset].empty()) {
			res = u8"Markdown_text value: " + std::string(embed.automatically_markdown_text ? "true" : "false");
		}
		else {
			embed.automatically_markdown_text = processed[1 + offset] == "true";
			res = u8"Markdown_text value set: " + std::string(embed.automatically_markdown_text ? "true" : "false");
		}
	}
	else if (processed[offset] == "reference_link") {
		if (processed[1 + offset].empty()) {
			res = u8"Reference_link value: " + std::string(embed.add_referece_link_to_message_copy ? "true" : "false");
		}
		else {
			embed.add_referece_link_to_message_copy = processed[1 + offset] == "true";
			res = u8"Reference_link value set: " + std::string(embed.add_referece_link_to_message_copy ? "true" : "false");
		}
	}
	else if (processed[offset] == "reference_chat") {
		if (processed[1 + offset].empty()) {
			res = u8"Reference_chat value: " + std::string(embed.add_referece_chat_to_message_copy ? "true" : "false");
		}
		else {
			embed.add_referece_chat_to_message_copy = processed[1 + offset] == "true";
			res = u8"Reference_chat value set: " + std::string(embed.add_referece_chat_to_message_copy ? "true" : "false");
		}
	}
	else if (processed[offset] == "reference_text") {
		if (processed[1 + offset].empty()) {
			res = u8"Reference_text value: " + embed.reference_link_back_text;
		}
		else {
			embed.reference_link_back_text.clear();
			for (size_t p = 1 + offset; !processed[p].empty(); p++) embed.reference_link_back_text += processed[p] + ' ';
			if (embed.reference_link_back_text.length()) embed.reference_link_back_text.pop_back();
			res = u8"Reference_text value set: " + embed.reference_link_back_text;
		}
	}
	else if (processed[offset] == "send_to") {
		if (processed[1 + offset].empty()) {
			res = u8"Send_to value: <#" + std::to_string(embed.chat_to_send_embed) + "> (ID#" + std::to_string(embed.chat_to_send_embed) + ")";
		}
		else {
			embed.chat_to_send_embed = stdstoulla(processed[1 + offset].c_str());
			res = u8"Send_to value set: <#" + std::to_string(embed.chat_to_send_embed) + "> (ID#"+ std::to_string(embed.chat_to_send_embed) + ")";
		}
	}
	else if (processed[offset] == "priority") {
		if (processed[1 + offset].empty()) {
			res = u8"Priority value: " + std::to_string(embed.priority);
		}
		else {
			embed.priority = atoi(processed[1 + offset].c_str());
			res = u8"Priority value set: " + std::to_string(embed.priority);
		}
	}
	else if (processed[offset] == "react") {
		if (processed[1 + offset] == "add") {
			if (processed[2 + offset].empty()) {
				res = "Unknown emoji!";
			}
			else {
				std::string _filtered = processed[2 + offset];
				if (_filtered.size() > 2) {
					if (_filtered.front() == '<') {
						_filtered.erase(_filtered.begin());
						if (_filtered.back() == '>') _filtered.pop_back();
					}
					else {
						_filtered = transform_weirdo(_filtered);
					}
				}

				bool proceed = true;
				for (auto& i : embed.auto_react_with)
				{
					if (i == _filtered) {
						res = "Already in the list!";
						proceed = false;
						break;
					}
				}
				if (proceed) {
					embed.auto_react_with.push_back(_filtered);
					res = "Added " + _filtered + " to the list.";
				}
			}
		}
		else if (processed[1 + offset] == "remove") {
			if (processed[2 + offset].empty()) {
				res = "Unknown emoji!";
			}
			else {
				std::string _filtered = processed[2 + offset];
				if (_filtered.size() > 2) {
					if (_filtered.front() == '<') {
						_filtered.erase(_filtered.begin());
						if (_filtered.back() == '>') _filtered.pop_back();
					}
					else {
						_filtered = transform_weirdo(_filtered);
					}
				}

				bool proceed = true;
				for (size_t p = 0; p < embed.auto_react_with.size(); p++)
				{
					if (embed.auto_react_with[p] == _filtered)
					{
						embed.auto_react_with.erase(embed.auto_react_with.begin() + p);
						p--;
						proceed = false;
						res = "Removed successfully!";
					}
				}
				if (proceed) {
					res = "Couldn't find the emoji from list!";
				}
			}
		}
		else if (processed[1 + offset] == "remove_all") {
			embed.auto_react_with.clear();
		}
		else {
			res += "Reacting to source: ";
			for (auto& i : embed.auto_react_with) {
				res += i + ' ';
			}
		}
	}
	else {
		res = u8"All data within embed:\n" + embed.to_json().dump(2);
	}

	return res;
}

void GuildHandle::command(const std::string& arguments, aegis::channel& ch, aegis::gateway::objects::message& from)
{
	//ch.create_message("Got this as arguments: '" + arguments + "'");

	std::unordered_map<size_t, std::string> processed;
	{
		std::string buf;
		size_t argp = 0;
		for (auto& i : arguments) {
			if (i != ' ') buf += i;
			else if (!buf.empty()) processed.emplace(argp++, std::move(buf));
		}
		if (!buf.empty()) processed.emplace(argp++, std::move(buf));
	}

	/*auto autodie = [](aegis::gateway::objects::message m) {
		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::seconds(20));
		m.delete_message();
	};*/

	const size_t len = processed.size();
	help_type htype = help_type::ALL;
	bool bypass_limit = false;

	for (auto& i : allowed_full_servers) {
		bypass_limit |= (ch.get_guild_id() == i);
	}
	
	//if (len > 1) 
	{
		//from.delete_message();

		auto& here = data.get_chat(ch.get_id());
		LockGuardShared luck(here.secure, lock_shared_mode::EXCLUSIVE);


		if (     processed[0] == "reset") {
			if (processed[1] == "CONFIRM") {
				bool found = false;
				for (auto i = data.chats.begin(); i < data.chats.end(); i++) {
					if (i->first == ch.get_id()) {
						data.chats.erase(i);
						found = true;
						break;
					}
				}

				if (found) {
					ch.create_message(
						u8"Reset chat #" + std::to_string(ch.get_id()) + " done."
					); // ).then<void>(autodie);
					data.save_nolock();
				}
				else {
					ch.create_message(
						u8"Couldn't find any config for chat #" + std::to_string(ch.get_id()) + "."
					); // ).then<void>(autodie);
				}
				return;
			}
			else {
				ch.create_message(
					u8"Are you sure? Call `... reset CONFIRM` if you really want to delete this chat whole config (forever)."
				); // ).then<void>(autodie);
				return;
			}
		}

		else if (processed[0] == "alias") {
			if (processed[1].empty()) {
				ch.create_message(
					u8"Alias: " + data.command_alias
				); // ).then<void>(autodie);
				return;
			}
			else {
				data.command_alias = processed[1];
				ch.create_message(
					u8"Alias set to: " + data.command_alias
				); // ).then<void>(autodie);
				data.save_nolock();
				return;
			}
		}

		else if (processed[0] == "error_delete_source") {
			if (processed[1].empty()) {
				ch.create_message(
					u8"Error causes delete source? " + std::string(data.on_error_delete_source ? "true" : "false")
				); // ).then<void>(autodie);
				return;
			}
			else {
				data.on_error_delete_source = processed[1] == "true";
				ch.create_message(
					u8"Error causes delete source now set to: " + std::string(data.on_error_delete_source ? "true" : "false")
				); // ).then<void>(autodie);
				data.save_nolock();
				return;
			}
		}

		else if (processed[0] == "status") {

			LockGuardShared luck(data.mut, lock_shared_mode::SHARED);

			const bool detailed = processed[1] == "full";

			std::string resume;
			size_t total_links = 0;
			size_t total_files = 0;
			size_t total_texts = 0;
			size_t this_links = here.link_configurations.size();
			size_t this_files = here.file_configurations.size();
			size_t this_texts = here.text_configurations.size();

			std::string detail_links;
			std::string detail_files;
			std::string detail_texts;
			std::string detail_chats;

			for (auto& i : data.chats) {
				total_links += i.second.link_configurations.size();
				total_files += i.second.file_configurations.size();
				total_texts += i.second.text_configurations.size();

				if (detailed) {
					detail_chats += std::to_string(i.first) + ";";
					if (i.second.link_configurations.size()) detail_links += std::to_string(i.first) + ";";
					if (i.second.file_configurations.size()) detail_files += std::to_string(i.first) + ";";
					if (i.second.text_configurations.size()) detail_texts += std::to_string(i.first) + ";";
				}
			}

			if (detailed) {
				if (detail_chats.length()) detail_chats.pop_back();
				if (detail_links.length()) detail_links.pop_back();
				if (detail_files.length()) detail_files.pop_back();
				if (detail_texts.length()) detail_texts.pop_back();
			}

			resume += "# This is the data saved for this server and some more information:\n";
			resume += "- Number of chats configured (all server, indexed): " + std::to_string(data.chats.size()) + "\n";
			if (detailed) resume += "* Chats:\n{" + detail_chats + "}\n";
			resume += "- Number of link-related configured (all server):   " + std::to_string(total_links) + "\n";
			if (detailed) resume += "* Chats:\n{" + detail_links + "}\n";
			resume += "- Number of file-related configured (all server):   " + std::to_string(total_files) + "\n";
			if (detailed) resume += "* Chats:\n{" + detail_files + "}\n";
			resume += "- Number of text-related configured (all server):   " + std::to_string(total_texts) + "\n";
			if (detailed) resume += "* Chats:\n{" + detail_texts + "}\n";
			resume += "- Number of link-related configured (this chat):    " + std::to_string(this_links) + "\n";
			resume += "- Number of file-related configured (this chat):    " + std::to_string(this_files) + "\n";
			resume += "- Number of text-related configured (this chat):    " + std::to_string(this_texts) + "\n";
			resume += "- Memory usage:                                     " + std::to_string(double(aegis::utility::getCurrentRSS()) / (1u << 20)) + " MB\n";
			resume += "- Peak memory usage:                                " + std::to_string(double(aegis::utility::getPeakRSS()) / (1u << 20)) + " MB\n";
			resume += "- Uptime:                                           " + core->uptime_str() + "\n";
			resume += "- Version:                                          " + version + "\n";

			for (size_t off = 0; resume.length();) {
				std::string to_send = resume.substr(0, 1950);
				if (resume.length() > 1950) resume = resume.substr(1950);
				else resume.clear();
				ch.create_message(
					"```md\n" + to_send + "```"
				); // ).then<void>(autodie);
			}

			return;
		}

		else if (processed[0] == "help") {
			if (processed[1] == "link") 
			{
				htype = help_type::LINK;
			}
			else if (processed[1] == "file") 
			{
				htype = help_type::FILE;
			}
			else if (processed[1] == "text") 
			{
				htype = help_type::TEXT;
			}
			else if (processed[1] == "modes") 
			{
				htype = help_type::MODES;
			}
			else if (processed[1] == "global") 
			{
				htype = help_type::GLOBAL;
			}
			else {
				ch.create_message(
					u8"Full help: just call the bot command\n"
					u8"Only specific help available:\n"
					u8"```css\nhelp global````Shows the \"Global/Chat\" help part only`\n"
					u8"```css\nhelp link````Shows the \"Link\" help part only`\n"
					u8"```css\nhelp file````Shows the \"File\" help part only`\n"
					u8"```css\nhelp text````Shows the \"Text\" help part only`\n"
					u8"```css\nhelp modes````Shows the first generic part and the MODES for handling only`\n"
				); // ).then<void>(autodie);
				return;
			}
		}

		// > > > > > > > > > > > > > > > > > > = LINK = < < < < < < < < < < < < < < < < < < //
		else if (processed[0] == "link")
		{
			size_t offset = 0;
			size_t which = 0;

			if (processed[1] == "select") {
				if (len >= 3) {
					which = atoll(processed[2].c_str());
					offset += 2;
				}
				else {
					ch.create_message(
						u8"There are " + std::to_string(here.link_configurations.size()) + " link settings available."
					); // ).then<void>(autodie);
					return;
				}
			}

			if (!bypass_limit && which >= maximum_rules_vector_per_chat_per_type) {
				ch.create_message(
					u8"Too many settings or number too big. The limit is " + std::to_string(maximum_rules_vector_per_chat_per_type) + " configurations per type per chat."
				); // ).then<void>(autodie);
				return;
			}

			{ 
				// check existance, or create next, or fail
				if (which == here.link_configurations.size()) {
					here.link_configurations.push_back({});
				}
				else if (which > here.link_configurations.size()) {
					ch.create_message(
						u8"Number too high, please select one that already exists or the exact next one (#" + std::to_string(here.link_configurations.size()) + ")."
					); // ).then<void>(autodie);
					return;
				}

				auto& actual = here.link_configurations[which];

				// "switch"
				if (processed[1 + offset] == "delete") {
					if (processed[2 + offset] == "CONFIRM") {
						here.link_configurations.erase(here.link_configurations.begin() + which);
						ch.create_message(
							u8"Deleted config #" + std::to_string(which) + " and moved the next ones one back."
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
					else {
						ch.create_message(
							u8"Are you sure? Call `... delete CONFIRM` if you really want to delete this config (forever)."
						); // ).then<void>(autodie);
						return;
					}
				}
				else if (processed[1 + offset] == "handling") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Handling value: " + interpret_string_handle_modes(actual.handling)
						); // ).then<void>(autodie);
						return;
					}
					else {			
						handle_modes _m{};
						if (interpret_handle_modes(_m, processed[2 + offset])) {
							actual.handling = _m;
							ch.create_message(
								u8"Handling value set: " + interpret_string_handle_modes(actual.handling)
							); // ).then<void>(autodie);
							data.save_nolock();
							return;
						}
						else {
							ch.create_message(
								u8"Cannot interpret value. Please check help."
							); // ).then<void>(autodie);
							return;
						}
					}
				}
				else if (processed[1 + offset] == "silence_links") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Silence_links value: " + std::string(actual.silence_links ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.silence_links = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Silence_links value set: " + std::string(actual.silence_links ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "silence_codeblock") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Silence_codeblock value: " + std::string(actual.silence_codeblock ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.silence_codeblock = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Silence_codeblock value set: " + std::string(actual.silence_codeblock ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "match") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Match value: " + actual.has_to_match_this
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.has_to_match_this = processed[2 + offset];
						ch.create_message(
							u8"Match value set: " + actual.has_to_match_this
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "match_regex") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Match_regex value: " + std::string(actual.use_regex ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.use_regex = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Match_regex value set: " + std::string(actual.use_regex ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "inverse_regex") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Inverse_regex value: " + std::string(actual.inverse_regex ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.inverse_regex = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Inverse_regex value set: " + std::string(actual.inverse_regex ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "send_to") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Send_to value: <#" + std::to_string(actual.chat_to_send) + "> (ID#" + std::to_string(actual.chat_to_send) + ")"
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.chat_to_send = stdstoulla(processed[2 + offset]);
						ch.create_message(
							u8"Send_to value set: <#" + std::to_string(actual.chat_to_send) + "> (ID#" + std::to_string(actual.chat_to_send) + ")"
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "react") {
					std::string resume = _command_react(processed, offset + 2, actual.react_to_source, actual.react_to_copy);					

					for (size_t off = 0; resume.length();) {
						std::string to_send = resume.substr(0, 1950);
						if (resume.length() > 1950) resume = resume.substr(1950);
						else resume.clear();
						ch.create_message(
							to_send
						); // ).then<void>(autodie);
					}
					data.save_nolock();
					return;
				}
				else if (processed[1 + offset] == "embed") {
					std::string resume = _command_embed(processed, offset + 2, actual.embed_settings);					

					for (size_t off = 0; resume.length();) {
						std::string to_send = resume.substr(0, 1950);
						if (resume.length() > 1950) resume = resume.substr(1950);
						else resume.clear();
						ch.create_message(
							to_send
						); // ).then<void>(autodie);
					}
					data.save_nolock();
					return;
				}
				else {
					if (which >= here.link_configurations.size())
					{
						ch.create_message(
							u8"Invalid number. Please check if this one really exists."
						); // ).then<void>(autodie);
						return;
					}
					else
					{
						std::string resume = u8"All data within #" + std::to_string(which) + ":\n" + here.link_configurations[which].to_json().dump(2);

						for (size_t off = 0; resume.length();) {
							std::string to_send = resume.substr(0, 1950);
							if (resume.length() > 1950) resume = resume.substr(1950);
							else resume.clear();
							ch.create_message(
								to_send
							); // ).then<void>(autodie);
						}
						return;
					}
				}
			}
		}

		// > > > > > > > > > > > > > > > > > > = FILE = < < < < < < < < < < < < < < < < < < //
		else if (processed[0] == "file")
		{
			size_t offset = 0;
			size_t which = 0;

			if (processed[1] == "select") {
				if (len >= 3) {
					which = atoll(processed[2].c_str());
					offset += 2;
				}
				else {
					ch.create_message(
						u8"There are " + std::to_string(here.file_configurations.size()) + " file settings available."
					); // ).then<void>(autodie);
				}
			}

			if (!bypass_limit && which >= maximum_rules_vector_per_chat_per_type) {
				ch.create_message(
					u8"Too many settings or number too big. The limit is " + std::to_string(maximum_rules_vector_per_chat_per_type) + " configurations per type per chat."
				); // ).then<void>(autodie);
				return;
			}

			{
				// check existance, or create next, or fail
				if (which == here.file_configurations.size()) {
					here.file_configurations.push_back({});
				}
				else if (which > here.file_configurations.size()) {
					ch.create_message(
						u8"Number too high, please select one that already exists or the exact next one (#" + std::to_string(here.file_configurations.size()) + ")."
					); // ).then<void>(autodie);
					return;
				}

				auto& actual = here.file_configurations[which];

				// "switch"
				if (processed[1 + offset] == "delete") {
					if (processed[2 + offset] == "CONFIRM") {
						here.file_configurations.erase(here.file_configurations.begin() + which);
						ch.create_message(
							u8"Deleted config #" + std::to_string(which) + " and moved the next ones one back."
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
					else {
						ch.create_message(
							u8"Are you sure? Call `... delete CONFIRM` if you really want to delete this config (forever)."
						); // ).then<void>(autodie);
						return;
					}
				}
				else if (processed[1 + offset] == "handling") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Handling value: " + interpret_string_handle_modes(actual.handling)
						); // ).then<void>(autodie);
						return;
					}
					else {
						handle_modes _m{};
						if (interpret_handle_modes(_m, processed[2 + offset])) {
							actual.handling = _m;
							ch.create_message(
								u8"Handling value set: " + interpret_string_handle_modes(actual.handling)
							); // ).then<void>(autodie);
							data.save_nolock();
							return;
						}
						else {
							ch.create_message(
								u8"Cannot interpret value. Please check help."
							); // ).then<void>(autodie);
							return;
						}
					}
				}
				else if (processed[1 + offset] == "match") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Match value: " + actual.has_to_match_this
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.has_to_match_this = processed[2 + offset];
						ch.create_message(
							u8"Match value set: " + actual.has_to_match_this
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "match_regex") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Match_regex value: " + std::string(actual.use_regex ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.use_regex = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Match_regex value set: " + std::string(actual.use_regex ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "inverse_regex") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Inverse_regex value: " + std::string(actual.inverse_regex ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.inverse_regex = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Inverse_regex value set: " + std::string(actual.inverse_regex ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "minimum_file_size") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Minimum_file_size value: " + std::to_string(actual.minimum_file_size) + " byte(s)"
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.minimum_file_size = atoi(processed[2 + offset].c_str());
						if (actual.minimum_file_size > actual.maximum_file_size) actual.maximum_file_size = actual.minimum_file_size;
						ch.create_message(
							u8"Minimum_file_size value set: " + std::to_string(actual.minimum_file_size) + " byte(s)"
						); // ).then<void>(autodie);
						return;
					}
				}
				else if (processed[1 + offset] == "maximum_file_size") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Maximum_file_size value: " + std::to_string(actual.maximum_file_size) + " byte(s)"
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.maximum_file_size = atoi(processed[2 + offset].c_str());
						if (actual.minimum_file_size < actual.maximum_file_size) actual.minimum_file_size = actual.maximum_file_size;
						ch.create_message(
							u8"Maximum_file_size value set: " + std::to_string(actual.maximum_file_size) + " byte(s)"
						); // ).then<void>(autodie);
						return;
					}
				}
				else if (processed[1 + offset] == "send_to") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Send_to value: <#" + std::to_string(actual.chat_to_send) + "> (ID#" + std::to_string(actual.chat_to_send) + ")"
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.chat_to_send = stdstoulla(processed[2 + offset]);
						ch.create_message(
							u8"Send_to value set: <#" + std::to_string(actual.chat_to_send) + "> (ID#" + std::to_string(actual.chat_to_send) + ")"
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "react") {
					std::string resume = _command_react(processed, offset + 2, actual.react_to_source, actual.react_to_copy);

					for (size_t off = 0; resume.length();) {
						std::string to_send = resume.substr(0, 1950);
						if (resume.length() > 1950) resume = resume.substr(1950);
						else resume.clear();
						ch.create_message(
							to_send
						); // ).then<void>(autodie);
					}
					data.save_nolock();
					return;
				}
				else if (processed[1 + offset] == "embed") {
					std::string resume = _command_embed(processed, offset + 2, actual.embed_settings);

					for (size_t off = 0; resume.length();) {
						std::string to_send = resume.substr(0, 1950);
						if (resume.length() > 1950) resume = resume.substr(1950);
						else resume.clear();
						ch.create_message(
							to_send
						); // ).then<void>(autodie);
					}
					data.save_nolock();
					return;
				}
				else {
					if (which >= here.file_configurations.size())
					{
						ch.create_message(
							u8"Invalid number. Please check if this one really exists."
						); // ).then<void>(autodie);
						return;
					}
					else
					{
						std::string resume = u8"All data within #" + std::to_string(which) + ":\n" + here.file_configurations[which].to_json().dump(2);

						for (size_t off = 0; resume.length();) {
							std::string to_send = resume.substr(0, 1950);
							if (resume.length() > 1950) resume = resume.substr(1950);
							else resume.clear();
							ch.create_message(
								to_send
							); // ).then<void>(autodie);
						}

						return;
					}
				}
			}
		}

		// > > > > > > > > > > > > > > > > > > = TEXT = < < < < < < < < < < < < < < < < < < //
		else if (processed[0] == "text")
		{
			size_t offset = 0;
			size_t which = 0;

			if (processed[1] == "select") {
				if (len >= 3) {
					which = atoll(processed[2].c_str());
					offset += 2;
				}
				else {
					ch.create_message(
						u8"There are " + std::to_string(here.text_configurations.size()) + " text settings available."
					); // ).then<void>(autodie);
				}
			}

			if (!bypass_limit && which >= maximum_rules_vector_per_chat_per_type) {
				ch.create_message(
					u8"Too many settings or number too big. The limit is " + std::to_string(maximum_rules_vector_per_chat_per_type) + " configurations per type per chat."
				); // ).then<void>(autodie);
				return;
			}

			{
				// check existance, or create next, or fail
				if (which == here.text_configurations.size()) {
					here.text_configurations.push_back({});
				}
				else if (which > here.text_configurations.size()) {
					ch.create_message(
						u8"Number too high, please select one that already exists or the exact next one (#" + std::to_string(here.text_configurations.size()) + ")."
					); // ).then<void>(autodie);
					return;
				}

				auto& actual = here.text_configurations[which];

				// "switch"
				if (processed[1 + offset] == "delete") {
					if (processed[2 + offset] == "CONFIRM") {
						here.text_configurations.erase(here.text_configurations.begin() + which);
						ch.create_message(
							u8"Deleted config #" + std::to_string(which) + " and moved the next ones one back."
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
					else {
						ch.create_message(
							u8"Are you sure? Call `... delete CONFIRM` if you really want to delete this config (forever)."
						); // ).then<void>(autodie);
						return;
					}
				}
				else if (processed[1 + offset] == "handling") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Handling value: " + interpret_string_handle_modes(actual.handling)
						); // ).then<void>(autodie);
						return;
					}
					else {
						handle_modes _m{};
						if (interpret_handle_modes(_m, processed[2 + offset])) {
							actual.handling = _m;
							ch.create_message(
								u8"Handling value set: " + interpret_string_handle_modes(actual.handling)
							); // ).then<void>(autodie);
							data.save_nolock();
							return;
						}
						else {
							ch.create_message(
								u8"Cannot interpret value. Please check help."
							); // ).then<void>(autodie);
							return;
						}
					}
				}
				else if (processed[1 + offset] == "ignore_links") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Ignore_links value: " + std::string(actual.no_links_please ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.no_links_please = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Ignore_links value set: " + std::string(actual.no_links_please ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "ignore_files") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Ignore_files value: " + std::string(actual.no_files_please ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.no_files_please = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Ignore_files value set: " + std::string(actual.no_files_please ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "silence_codeblock") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Silence_codeblock value: " + std::string(actual.silence_codeblock ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.silence_codeblock = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Silence_codeblock value set: " + std::string(actual.silence_codeblock ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "match") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Match value: " + actual.has_to_match_this
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.has_to_match_this = processed[2 + offset];
						ch.create_message(
							u8"Match value set: " + actual.has_to_match_this
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "match_regex") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Match_regex value: " + std::string(actual.use_regex ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.use_regex = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Match_regex value set: " + std::string(actual.use_regex ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "inverse_regex") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Inverse_regex value: " + std::string(actual.inverse_regex ? "true" : "false")
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.inverse_regex = (processed[2 + offset] == "true");
						ch.create_message(
							u8"Inverse_regex value set: " + std::string(actual.inverse_regex ? "true" : "false")
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "send_to") {
					if (processed[2 + offset].empty()) {
						ch.create_message(
							u8"Send_to value: <#" + std::to_string(actual.chat_to_send) + "> (ID#" + std::to_string(actual.chat_to_send) + ")"
						); // ).then<void>(autodie);
						return;
					}
					else {
						actual.chat_to_send = stdstoulla(processed[2 + offset]);
						ch.create_message(
							u8"Send_to value set: <#" + std::to_string(actual.chat_to_send) + "> (ID#" + std::to_string(actual.chat_to_send) + ")"
						); // ).then<void>(autodie);
						data.save_nolock();
						return;
					}
				}
				else if (processed[1 + offset] == "react") {
					std::string resume = _command_react(processed, offset + 2, actual.react_to_source, actual.react_to_copy);

					for (size_t off = 0; resume.length();) {
						std::string to_send = resume.substr(0, 1950);
						if (resume.length() > 1950) resume = resume.substr(1950);
						else resume.clear();
						ch.create_message(
							to_send
						); // ).then<void>(autodie);
					}
					data.save_nolock();
					return;
				}
				else if (processed[1 + offset] == "embed") {
					std::string resume = _command_embed(processed, offset + 2, actual.embed_settings);

					for (size_t off = 0; resume.length();) {
						std::string to_send = resume.substr(0, 1950);
						if (resume.length() > 1950) resume = resume.substr(1950);
						else resume.clear();
						ch.create_message(
							to_send
						); // ).then<void>(autodie);
					}
					data.save_nolock();
					return;
				}
				else {
					if (which >= here.text_configurations.size())
					{
						ch.create_message(
							u8"Invalid number. Please check if this one really exists."
						); // ).then<void>(autodie);
						return;
					}
					else
					{
						std::string resume = u8"All data within #" + std::to_string(which) + ":\n" + here.text_configurations[which].to_json().dump(2);

						for (size_t off = 0; resume.length();) {
							std::string to_send = resume.substr(0, 1950);
							if (resume.length() > 1950) resume = resume.substr(1950);
							else resume.clear();
							ch.create_message(
								to_send
							); // ).then<void>(autodie);
						}
						return;
					}
				}
			}
		}
	}

	if (htype == help_type::ALL || htype == help_type::MODES) {
		ch.create_message(
			u8"**Help (command ...):**\n"
			u8"```md\n"
			u8"# About [select <order>]\n"
			u8"- You create a new config everytime you don't specify this.\n"
			u8"- The first one is #0, and you can edit it with 'select 0'. If you have multiple ones, select the one you want.\n"
			u8"# General\n"
			u8"- If you just select, it will show the JSON of what you selected.\n"
			u8"- You can call any config to have its value (if you don't mention a value to set).\n"
			u8"```\n"
			u8"**Handling modes (USE CAPS)**\n"
			u8"```css\nNONE````Do nothing`\n"
			u8"```css\nDELETE_SOURCE````Erase the source only`\n"
			u8"```css\nCOPY_SORCE````Copy source and don't touch it`\n"
			u8"```css\nCUT_SOURCE````Cut and paste the source somewhere else`\n"
			u8"```css\nCUT_AND_REFERENCE_BACK_EMBED````Cut, paste somewhere ese and reference back on source's chat.`\n"
		); // ).then<void>(autodie);
	}

	if (htype == help_type::ALL || htype == help_type::GLOBAL) {
		ch.create_message_embed({}, {
			{ "title", "***GLOBAL/CHAT***" },
			{ "color", default_color },
			{ "description",
				u8"```css\nGlobal```"
				u8"`help ...                                               `\n"
				u8"`     global                                            `\n"
				u8"`     link                                              `\n"
				u8"`     file                                              `\n"
				u8"`     text                                              `\n"
				u8"`     modes                                             `\n"
				u8"`status <full>                                          `\n"
				u8"`alias <string>                                         `\n"
				u8"`error_delete_source <true|false>                       `\n"
				u8"```css\nChat```"
				u8"`reset <CONFIRM>                                        `\n"
			}
		}); // ).then<void>(autodie);
	}

	if (htype == help_type::ALL || htype == help_type::LINK) {
		ch.create_message_embed({}, {
			{ "title", "***LINK***" },
			{ "color", default_color },
			{ "description",
				u8"`link [select <order>] ...                              `\n"
				u8"`                      delete <CONFIRM>                 `\n"
				u8"`                      handling <MODE>                  `\n"
				u8"`                      silence_links <true|false>       `\n"
				u8"`                      silence_codeblock <true|false>   `\n"
				u8"`                      match <string>                   `\n"
				u8"`                      match_regex <true|false>         `\n"
				u8"`                      inverse_regex <true|false>       `\n"
				u8"`                      send_to <id>                     `\n"
				u8"`                      react ...                        `\n"
				u8"`                            source ...                 `\n"
				u8"`                                   add <emoji>         `\n"
				u8"`                                   remove <emoji>      `\n"
				u8"`                                   remove_all          `\n"
				u8"`                            destination ...            `\n"
				u8"`                                        add <emoji>    `\n"
				u8"`                                        remove <emoji> `\n"
				u8"`                                        remove_all     `\n"
				u8"`                      embed ...                        `\n"
				u8"`                            show_thumbnail <true|false>`\n"
				u8"`                            thumbnail_override <url>   `\n"
				u8"`                            show_user <true|false>     `\n"
				u8"`                            max_length <unsigned>      `\n"
				u8"`                            markdown_text <true|false> `\n"
				u8"`                            reference_link <true|false>`\n"
				u8"`                            reference_chat <true|false>`\n"
				u8"`                            reference_text <string>    `\n"
				u8"`                            send_to <id>               `\n"
				u8"`                            priority <int>             `\n"
				u8"`                            react ...                  `\n"
				u8"`                                  add <emoji>          `\n"
				u8"`                                  remove <emoji>       `\n"
				u8"`                                  remove_all           `\n"
			}
		}); // ).then<void>(autodie);
	}

	if (htype == help_type::ALL || htype == help_type::FILE) {
		ch.create_message_embed({}, {
			{ "title", "***FILE***" },
			{ "color", default_color },
			{ "description",
				u8"`file [select <order>] ...                              `\n"
				u8"`                      delete <CONFIRM>                 `\n"
				u8"`                      handling <MODE>                  `\n"
				u8"`                      match <string>                   `\n"
				u8"`                      match_regex <true|false>         `\n"
				u8"`                      inverse_regex <true|false>       `\n"
				u8"`                      minimum_file_size <unsigned>     `\n"
				u8"`                      maximum_file_size <unsigned>     `\n"
				u8"`                      send_to <id>                     `\n"
				u8"`                      react ...                        `\n"
				u8"`                            source ...                 `\n"
				u8"`                                   add <emoji>         `\n"
				u8"`                                   remove <emoji>      `\n"
				u8"`                                   remove_all          `\n"
				u8"`                            destination ...            `\n"
				u8"`                                        add <emoji>    `\n"
				u8"`                                        remove <emoji> `\n"
				u8"`                                        remove_all     `\n"
				u8"`                      embed ...                        `\n"
				u8"`                            show_thumbnail <true|false>`\n"
				u8"`                            thumbnail_override <url>   `\n"
				u8"`                            show_user <true|false>     `\n"
				u8"`                            max_length <unsigned>      `\n"
				u8"`                            markdown_text <true|false> `\n"
				u8"`                            reference_link <true|false>`\n"
				u8"`                            reference_chat <true|false>`\n"
				u8"`                            reference_text <string>    `\n"
				u8"`                            send_to <id>               `\n"
				u8"`                            priority <int>             `\n"
				u8"`                            react ...                  `\n"
				u8"`                                  add <emoji>          `\n"
				u8"`                                  remove <emoji>       `\n"
				u8"`                                  remove_all           `\n"
			}
		}); // ).then<void>(autodie);
	}

	if (htype == help_type::ALL || htype == help_type::TEXT) {
		ch.create_message_embed({}, {
			{ "title", "***TEXT***" },
			{ "color", default_color },
			{ "description",
				u8"`text [select <order>] ...                              `\n"
				u8"`                      delete <CONFIRM>                 `\n"
				u8"`                      ignore_links <true|false>        `\n"
				u8"`                      ignore_files <true|false>        `\n"
				u8"`                      handling <MODE>                  `\n"
				u8"`                      silence_codeblock <true|false>   `\n"
				u8"`                      match <string>                   `\n"
				u8"`                      match_regex <true|false>         `\n"
				u8"`                      inverse_regex <true|false>       `\n"
				u8"`                      send_to <id>                     `\n"
				u8"`                      react ...                        `\n"
				u8"`                            source ...                 `\n"
				u8"`                                   add <emoji>         `\n"
				u8"`                                   remove <emoji>      `\n"
				u8"`                                   remove_all          `\n"
				u8"`                            destination ...            `\n"
				u8"`                                        add <emoji>    `\n"
				u8"`                                        remove <emoji> `\n"
				u8"`                                        remove_all     `\n"
				u8"`                      embed ...                        `\n"
				u8"`                            show_thumbnail <true|false>`\n"
				u8"`                            thumbnail_override <url>   `\n"
				u8"`                            show_user <true|false>     `\n"
				u8"`                            max_length <unsigned>      `\n"
				u8"`                            markdown_text <true|false> `\n"
				u8"`                            reference_link <true|false>`\n"
				u8"`                            reference_chat <true|false>`\n"
				u8"`                            reference_text <string>    `\n"
				u8"`                            send_to <id>               `\n"
				u8"`                            priority <int>             `\n"
				u8"`                            react ...                  `\n"
				u8"`                                  add <emoji>          `\n"
				u8"`                                  remove <emoji>       `\n"
				u8"`                                  remove_all           `\n"
			}
		}); // ).then<void>(autodie);
	}
}

std::string GuildHandle::filter(std::string s, const bool c, const bool l)
{
	if (c) {
		for (auto& i : s) if (i == '`') i = '\'';
	}
	if (l) {
		std::smatch m;
		std::string wrk;
		while (std::regex_search(s, m, regex_link)) {
			if (size_t pos = m.position(0) + m[0].str().length(); m.position(0) > 0 && s[m.position(0) - 1] == '<' && pos < s.length() && s[pos] == '>') {
				wrk += s.substr(0, m.position(0)) + m[0].str();
			}
			else wrk += s.substr(0, m.position(0)) + "<" + m[0].str() + ">";
			s = (s.length() > (m[0].str().length() + m.position(0)) ? (s.substr(m[0].str().length() + m.position(0))) : "");
		}
		if (!s.empty()) wrk += s;
		wrk = wrk.substr(0, 2000);
		s = std::move(wrk);
	}
	return std::move(s);
}

Tools::Future<aegis::gateway::objects::message> GuildHandle::copy_message_to(bool& good, const unsigned long long chid, const aegis::gateway::objects::message& m, const bool code_clean, const bool link_silence)
{
	return copy_message_to(good, chid, m.get_content(), m.author.username + "#" + m.author.discriminator, code_clean, link_silence);
}
Tools::Future<aegis::gateway::objects::message> GuildHandle::copy_message_to(bool& good, const unsigned long long chid, const std::string content, const std::string author, const bool code_clean, const bool link_silence)
{
	if (auto* ch = core->find_channel(chid); ch) {
		std::string to_send = "`" + author + ":` " + filter(content, code_clean, link_silence);
		to_send = to_send.substr(0, safe_msg_limit);
		good = true;
		return ch->create_message(to_send);
	}
	good = false;
	return Tools::fake_future(aegis::gateway::objects::message{});
}

Tools::Future<aegis::gateway::objects::message> GuildHandle::copy_file_to(bool& good, const unsigned long long chid, const std::string& filename, const std::string& data)
{
	if (auto* ch = core->find_channel(chid); ch) {
		aegis::rest::aegis_file fp;
		fp.name = filename;
		for (auto& k : data) fp.data.push_back(k);
		aegis::create_message_t m;
		m.file(fp);
		return ch->create_message(m);
	}
	good = false;
	return Tools::fake_future(aegis::gateway::objects::message{});
}

Tools::Future<aegis::gateway::objects::message> GuildHandle::copy_generate_embed_auto_everything(bool& good, const aegis::gateway::objects::message& m, const chat_configuration::embed_configuration& cc, const unsigned long long cid, const unsigned long long mid, const std::string imgurl/*, const bool nodesc*/)
{
	if (auto* ch = core->find_channel(cc.chat_to_send_embed); ch) {

		/*if (!ch->perms().can_embed()) {
			good = false;
			return Tools::fake_future(aegis::gateway::objects::message{});
		}*/

		good = true;

		nlohmann::json j;
		std::string pre_msg;

		// fixed
		j["color"] = default_color;
		j["footer"]["text"] = std::to_string(m.author.id);

		// not fixed
		if (cc.show_user_information) {
			j["author"]["name"] = m.author.username + "#" + m.author.discriminator;
			j["author"]["icon_url"] = "https://cdn.discordapp.com/avatars/" + std::to_string(m.author.id) + "/" + m.author.avatar + ".png?size=256";
		}
		if (cc.show_thumbnail) {
			if (!cc.thumbnail_override.empty()) j["thumbnail"]["url"] = cc.thumbnail_override;
			else j["thumbnail"]["url"] = "https://cdn.discordapp.com/avatars/" + std::to_string(m.author.id) + "/" + m.author.avatar + ".png?size=256";
		}
		if (cc.add_referece_link_to_message_copy) {
			std::string link = "https://discordapp.com/channels/" + std::to_string(m.get_guild_id()) + "/" + std::to_string(cid) + "/" + std::to_string(mid);
			j["url"] = link;
			pre_msg += "[" + cc.reference_link_back_text + "](" + link + ") ";
		}
		if (cc.add_referece_chat_to_message_copy) {
			pre_msg += ("<#" + std::to_string(cid) + ">");
		}

		//if (!nodesc)
		{
			std::string _res = filter(m.get_content().substr(0, cc.max_text_length), cc.automatically_markdown_text, false);
			j["description"] = 
				(
					pre_msg + (pre_msg.length() ? "\n" : "") +
					(cc.automatically_markdown_text ? (_res.length() ? "```md\n" : "") : "") + _res + (cc.automatically_markdown_text ? (_res.length() ? "```" : "") : "")
				).substr(0, 2048);
		}
		
		if (!imgurl.empty()) j["image"]["url"] = imgurl;

		return ch->create_message_embed({}, j).then<aegis::gateway::objects::message>([=](aegis::gateway::objects::message m) {
			for (auto& j : cc.auto_react_with) {
				m.create_reaction(j).get();
			}
			return m;
		});
	}
	good = false;
	return Tools::fake_future(aegis::gateway::objects::message{});
}
/*
void GuildHandle::load_file()
{
	data.load();
}

void GuildHandle::save_file()
{
	if (!data.save_nolock()) {
		std::cout << "FAILED TO SAVE GUILD #" + std::to_string(data.guild_id) + " TO FILE!" << std::endl;
	}
}*/

GuildHandle::GuildHandle(std::shared_ptr<aegis::core> core, aegis::snowflake id)
	: data(id.get())
{
	this->core = core;
	this->logg = core->log;
	//load_file();
	//logg->info("Guild #{} launched.", data.guild_id);
}

GuildHandle::~GuildHandle()
{
	//save_file();
	//if (logg.use_count()) logg->info("Guild #{} disconnected.", data.guild_id);
}

bool GuildHandle::operator==(const aegis::snowflake& f)
{
	return data.guild_id == f;
}

void GuildHandle::handle(aegis::channel& ch, aegis::gateway::objects::message m)
{
	if (m.get_content().find(default_command_start) == 0) {
		if (!(m.author.id == ch.get_guild().get_owner() || m.author.id == mee_dev)) {
			ch.create_message("Only the owner can run commands.");
			return;
		}
		std::string sub = m.get_content().substr(default_command_start.length()); // cut command
		while (sub.length() > 0 && sub[0] == ' ') sub.erase(sub.begin()); // cleanup space
		command(sub, ch, m);
	}
	else if (!data.command_alias.empty() && m.get_content().find(data.command_alias) == 0) {
		if (!(m.author.id == ch.get_guild().get_owner() || m.author.id == mee_dev)) {
			ch.create_message("Only the owner can run commands.");
			return;
		}
		std::string sub = m.get_content().substr(data.command_alias.length()); // cut command
		while (sub.length() > 0 && sub[0] == ' ') sub.erase(sub.begin()); // cleanup space
		command(sub, ch, m);
	}
	else { // filter things.

		std::string content_lower = m.get_content();
		for (auto& i : content_lower) i = std::tolower(i);

		bool has_link = std::regex_search(content_lower, regex_link);
		bool has_file = m.attachments.size();
		bool has_text = !m.get_content().empty();
		bool delete_message_source = false;

		// FROM HERE, CRITICAL 
		chat_configuration& here = data.get_chat(ch.get_id());
		LockGuardShared luck(data.mut, lock_shared_mode::SHARED);
		LockGuardShared luck2(here.secure, lock_shared_mode::SHARED);

		/*chat_configuration::embed_configuration config; // use: set_if_higher_priority
		unsigned long long config_channel_id = 0;
		unsigned long long config_message_id = 0;
		bool has_config_been_set = false; // if not, ignore config variable.*/
		int embed_set_highest_value = 0;
		std::function<void(void)> embed_set_to_do;
		const aegis::gateway::objects::message mcpy = m; // fixed content copy


		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
		//                                                                   LINK MANAGING                                                                   //
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //


		if (has_link) {
			for (auto& cc : here.link_configurations) {
				bool match = false;
				if (cc.use_regex) {
					try {
						std::regex rex(cc.has_to_match_this);
						match = std::regex_search(content_lower, rex);
						if (cc.inverse_regex) match = !match;
					}
					catch (const std::regex_error& e) {
						if (data.on_error_delete_source) m.delete_message();
						ch.create_message("[LINK] Failed doing regex here with format = '" + cc.has_to_match_this + "'. Ignored REGEX. Bot gave up.")
							.then<void>([](aegis::gateway::objects::message mm) {
								std::this_thread::sleep_for(std::chrono::seconds(20));
								mm.delete_message();
							});
						return;
					}
				}
				else {
					match = content_lower.find(cc.has_to_match_this) != std::string::npos;
				}

				if (match) {

					switch (cc.handling) {
					case handle_modes::NONE:
					{
						for (auto& j : cc.react_to_source) m.create_reaction(j).get();
					}
						break;

					case handle_modes::DELETE_SOURCE:
					{
						delete_message_source = true;
					}
						break;

					case handle_modes::CUT_SOURCE:
					{
						delete_message_source = true;
					}
					// CUT IS A COPY WITH DELETE!

					case handle_modes::COPY_SOURCE:
					{
						for (auto& j : cc.react_to_source) m.create_reaction(j).get();

						bool gottem = false;
						auto fut = copy_message_to(gottem, cc.chat_to_send, m, cc.silence_codeblock, cc.silence_links);
						if (gottem) {
							fut.then<void>([=](aegis::gateway::objects::message m) {
								for (auto& j : cc.react_to_copy) {
									m.create_reaction(j).get();
								}
							});
						}
						else {
							if (data.on_error_delete_source) m.delete_message();
							ch.create_message("[LINK] Cannot handle message. Copying last message failed. Bot gave up. Please check settings like chat to send.")
								.then<void>([](aegis::gateway::objects::message mm) {
									std::this_thread::sleep_for(std::chrono::seconds(20));
									mm.delete_message();
								});
							return;
						}
					}
						break;

					case handle_modes::CUT_AND_REFERENCE_BACK_EMBED: // same as copy, but has copy_generate_embed_auto_everything block there
					{
						delete_message_source = true;

						bool gottem = false;

						auto fut = copy_message_to(gottem, cc.chat_to_send, m, cc.silence_codeblock, cc.silence_links);
						if (gottem) {
							auto m3 = fut.get();
							for (auto& j : cc.react_to_copy) {
								m3.create_reaction(j).get();
							}

							if (!embed_set_to_do || cc.embed_settings.priority > embed_set_highest_value) {
								embed_set_to_do = [&, mcpy, m3, embs = cc.embed_settings] {
									bool gottem2 = false;
									if (copy_generate_embed_auto_everything(gottem2, mcpy, embs, m3.get_channel_id(), m3.get_id(), ""); !gottem2) {
										if (data.on_error_delete_source) m.delete_message();
										ch.create_message("[LINK] Cannot create EMBED message. Please fix permissions or dig down into this. Bot gave up.")
											.then<void>([](aegis::gateway::objects::message mm) {
											std::this_thread::sleep_for(std::chrono::seconds(20));
											mm.delete_message();
										});
										return;
									}
								};
								embed_set_highest_value = cc.embed_settings.priority;
							}
							
						}
						else {
							if (data.on_error_delete_source) m.delete_message();
							ch.create_message("[LINK] Cannot handle message. Copying last message failed. Bot gave up. Please check settings like chat to send.")
								.then<void>([](aegis::gateway::objects::message mm) {
									std::this_thread::sleep_for(std::chrono::seconds(20));
									mm.delete_message();
								});
							return;
						}
					}
						break;
					}
				}
			}
		}


		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
		//                                                                   FILE MANAGING                                                                   //
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //


		if (has_file) {

			std::string content_copy_sure = (has_link ? "" : m.get_content());

			for (auto& each_file : m.attachments) {

				std::string filename_lower = each_file.filename;
				for (auto& i : filename_lower) i = std::tolower(i);

				for (auto& cc : here.file_configurations) {
					bool match = false;
					if (cc.use_regex) {
						try {
							std::regex rex(cc.has_to_match_this);
							match = std::regex_search(filename_lower, rex);
							if (cc.inverse_regex) match = !match;
						}
						catch (const std::regex_error& e) {
							if (data.on_error_delete_source) m.delete_message();
							ch.create_message("[FILE] Failed doing regex here with format = '" + cc.has_to_match_this + "'. Ignored REGEX. Bot gave up.")
								.then<void>([](aegis::gateway::objects::message mm) {
									std::this_thread::sleep_for(std::chrono::seconds(20));
									mm.delete_message();
								});
							return;
						}
					}
					else {
						match = filename_lower.find(cc.has_to_match_this) != std::string::npos;
					}

					if (match) {

						switch (cc.handling) {
						case handle_modes::NONE:
						{
							for (auto& j : cc.react_to_source) m.create_reaction(j).get();
						}
						break;

						case handle_modes::DELETE_SOURCE:
						{
							delete_message_source = true;
						}
						break;

						case handle_modes::CUT_SOURCE:
						{
							delete_message_source = true;
						}
						// CUT IS A COPY WITH DELETE!

						case handle_modes::COPY_SOURCE:
						{
							Interface::Downloader down;
							if (!down.get(each_file.url)) {
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot download file '" + filename_lower + "' from '<" + each_file.url + ">'. Ignored DOWNLOAD. Bot gave up.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								break;
							}
							// downloaded file.

							if (down.read().length() < cc.maximum_file_size || down.read().length() > cc.minimum_file_size) // filter
								break;

							if (down.read().length() > maximum_file_size_global) {
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot handle message. File is too big, I can only handle up to 8 MB (Discord limit). Bot gave up.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								return;
							}

							bool gottem = false;
							auto fut = copy_message_to(gottem, cc.chat_to_send, content_copy_sure, m.author.username + "#" + m.author.discriminator, true, false);
							if (!gottem) { // no reactions here, so just test
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot handle message. Copying last message (username) failed. Bot gave up. Please check settings like chat to send.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								return;
							}

							fut.get(); // sync, better.

							auto fut2 = copy_file_to(gottem, cc.chat_to_send, filename_lower.length() ? filename_lower : "unknown", down.read());
							if (gottem) {								
								fut2.then<void>([=](aegis::gateway::objects::message m) {
									for (auto& j : cc.react_to_copy) {
										m.create_reaction(j).get();
									}
								});
							}
							else {
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot handle message. Copying last message (file) failed. Bot gave up. Please check settings like chat to send.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								return;
							}

						}
						break;

						case handle_modes::CUT_AND_REFERENCE_BACK_EMBED: // same as copy, but has copy_generate_embed_auto_everything block there
						{							
							Interface::Downloader down;
							if (!down.get(each_file.url)) {
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot download file '" + filename_lower + "' from '<" + each_file.url + ">'. Ignored DOWNLOAD. Bot gave up.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								break;
							}
							// downloaded file.

							if (down.read().length() > cc.maximum_file_size || down.read().length() < cc.minimum_file_size) // filter
								break;

							delete_message_source = true;

							if (down.read().length() > maximum_file_size_global) {
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot handle message. File is too big, I can only handle up to 8 MB (Discord limit). Bot gave up.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								return;
							}

							bool gottem = false;
							auto fut = copy_message_to(gottem, cc.chat_to_send, content_copy_sure, m.author.username + "#" + m.author.discriminator, true, false);
							if (!gottem) { // no reactions here, so just test
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot handle message. Copying last message (username) failed. Bot gave up. Please check settings like chat to send.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								return;
							}

							fut.get(); // sync, better.

							auto fut2 = copy_file_to(gottem, cc.chat_to_send, filename_lower.length() ? filename_lower : "unknown", down.read());
							if (gottem) {
								auto m3 = fut2.get();
								for (auto& j : cc.react_to_copy) {
									m3.create_reaction(j).get();
								}
								// asdasdasd

								if (!embed_set_to_do || cc.embed_settings.priority > embed_set_highest_value) {
									embed_set_to_do = [&, mcpy, m3, embs = cc.embed_settings]{
										bool gottem2 = false;
										if (copy_generate_embed_auto_everything(gottem2, mcpy, embs, m3.get_channel_id(), m3.get_id(), m3.attachments[0].url); !gottem2) {
											if (data.on_error_delete_source) m.delete_message();
											ch.create_message("[FILE] Cannot create EMBED message. Please fix permissions or dig down into this. Bot gave up.")
												.then<void>([](aegis::gateway::objects::message mm) {
												std::this_thread::sleep_for(std::chrono::seconds(20));
												mm.delete_message();
													});
											return;
										}
									};
									embed_set_highest_value = cc.embed_settings.priority;
								}
							}
							else {
								if (data.on_error_delete_source) m.delete_message();
								ch.create_message("[FILE] Cannot handle message. Copying last message (file) failed. Bot gave up. Please check settings like chat to send.")
									.then<void>([](aegis::gateway::objects::message mm) {
										std::this_thread::sleep_for(std::chrono::seconds(20));
										mm.delete_message();
									});
								return;
							}
						}
						break;
						}
					}
				}
			}
		}


		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
		//                                                                   TEXT MANAGING                                                                   //
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

		
		if (has_text) {
			for (auto& cc : here.text_configurations) {
				bool match = false;

				if (cc.no_links_please && has_link) continue; // skip if ignore links
				if (cc.no_files_please && has_file) continue; // skip if ignore files
				
				if (cc.use_regex) {
					try {
						std::regex rex(cc.has_to_match_this);
						match = std::regex_search(content_lower, rex);
						if (cc.inverse_regex) match = !match;
					}
					catch (const std::regex_error& e) {
						if (data.on_error_delete_source) m.delete_message();
						ch.create_message("[TEXT] Failed doing regex here with format = '" + cc.has_to_match_this + "'. Ignored REGEX. Bot gave up.")
							.then<void>([](aegis::gateway::objects::message mm) {
								std::this_thread::sleep_for(std::chrono::seconds(20));
								mm.delete_message();
							});
						return;
					}
				}
				else {
					match = content_lower.find(cc.has_to_match_this) != std::string::npos;
				}

				if (match) {

					switch (cc.handling) {
					case handle_modes::NONE:
					{
						for (auto& j : cc.react_to_source) m.create_reaction(j).get();
					}
						break;

					case handle_modes::DELETE_SOURCE:
					{
						delete_message_source = true;
					}
						break;

					case handle_modes::CUT_SOURCE:
					{
						delete_message_source = true;
					}
					// CUT IS A COPY WITH DELETE!

					case handle_modes::COPY_SOURCE:
					{
						for (auto& j : cc.react_to_source) m.create_reaction(j);

						bool gottem = false;
						auto fut = copy_message_to(gottem, cc.chat_to_send, m, cc.silence_codeblock, false);
						if (gottem) {
							fut.then<void>([=](aegis::gateway::objects::message m) {
								for (auto& j : cc.react_to_copy) {
									m.create_reaction(j).get();
								}
							});
						}
						else {
							if (data.on_error_delete_source) m.delete_message();
							ch.create_message("[TEXT] Cannot handle message. Copying last message failed. Bot gave up. Please check settings like chat to send.")
								.then<void>([](aegis::gateway::objects::message mm) {
									std::this_thread::sleep_for(std::chrono::seconds(20));
									mm.delete_message();
								});
							return;
						}
					}
						break;

					case handle_modes::CUT_AND_REFERENCE_BACK_EMBED: // same as copy, but has copy_generate_embed_auto_everything block there
					{
						delete_message_source = true;

						bool gottem = false;

						auto fut = copy_message_to(gottem, cc.chat_to_send, m, cc.silence_codeblock, false);
						if (gottem) {
							auto m3 = fut.get();
							for (auto& j : cc.react_to_copy) {
								m3.create_reaction(j).get();
							}

							if (!embed_set_to_do || cc.embed_settings.priority > embed_set_highest_value) {
								embed_set_to_do = [&, mcpy, m3, embs = cc.embed_settings]{
									bool gottem2 = false;
									if (copy_generate_embed_auto_everything(gottem2, mcpy, embs, m3.get_channel_id(), m3.get_id(), ""); !gottem2) {
										if (data.on_error_delete_source) m.delete_message();
										ch.create_message("[TEXT] Cannot create EMBED message. Please fix permissions or dig down into this. Bot gave up.")
											.then<void>([](aegis::gateway::objects::message mm) {
												std::this_thread::sleep_for(std::chrono::seconds(20));
												mm.delete_message();
											});
										return;
									}
								};
								embed_set_highest_value = cc.embed_settings.priority;
							}							
						}
						else {
							if (data.on_error_delete_source) m.delete_message();
							ch.create_message("[TEXT] Cannot handle message. Copying last message failed. Bot gave up. Please check settings like chat to send.")
								.then<void>([](aegis::gateway::objects::message mm) {
									std::this_thread::sleep_for(std::chrono::seconds(20));
									mm.delete_message();
								});
							return;
						}
					}
						break;
					}
				}
			}
		}

		if (embed_set_to_do) embed_set_to_do();

		if (delete_message_source) m.delete_message();
	}
}

void GuildHandle::handle(aegis::channel& ch, const unsigned long long mid, aegis::gateway::objects::emoji e)
{
	try {
		auto msgp = ch.get_message(mid);
		while (!msgp.get_ready()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(4));
			std::this_thread::yield();
		}
		auto msg = msgp.get();

		//["footer"]["text"]

		if (msg.embeds.size() && [&] {for (auto& i : bot_ids) if (msg.author.id == i) return true; return false; }()) { // message generated from bot && has embed

			nlohmann::json j = msg.embeds[0];
			unsigned long long user_c = 0;

			if (j.count("footer") && !j["footer"].is_null()) {
				const auto& inn = j["footer"];
				if (inn.count("text") && !inn["text"].is_null()) {
					std::string scn = inn["text"].get<std::string>();
					if (!sscanf_s(scn.c_str(), "%llu", &user_c)) user_c = 0;
				}
			}

			std::string e_end = e.id ? ((e.animated ? "a:" : "") + e.name + ":" + std::to_string(e.id)) : (transform_weirdo(e.name));

			aegis::gateway::objects::message msg = ch.get_message(mid).get();

			if (user_c) {
				if (user_c == e.user) {
					if (!e.id && e_end == delete_emoji) {
						//logg->info("Emoji: got DELETE MESSAGE.");
						ch.delete_message(msg.get_id());
					}
					else if (!e.id && e_end == recycle_emoji) {
						//logg->info("Emoji: got DEL REACTIONS.");
						ch.delete_all_reactions(msg.get_id());
					}
					else {
						//logg->info("Emoji: got ADD.");
						ch.create_reaction(msg.get_id(),
							e.id ? ((e.animated ? "a:" : "") + e.name + ":" + std::to_string(e.id)) : (transform_weirdo(e.name))
						);
					}
				}
				else {
					bool skip_ = false;
					if (msg.reactions.size()) {
						for (auto& i : msg.reactions) {
							if (i.emoji_.id == e.id && i.emoji_.name == e.name && i.emoji_.animated == e.animated && (i.count > 1 || i.me)) {
								skip_ = true;
								break;
							}
						}
					}
					if (!skip_) ch.delete_user_reaction(msg.get_id(), e.id ? ((e.animated ? "a:" : "") + e.name + ":" + std::to_string(e.id)) : (transform_weirdo(e.name)), e.user);
				}
			}
		}
	}
	catch (...) {
		logg->critical("FATAL ERROR HANDLING INPUT.");
	}
}
