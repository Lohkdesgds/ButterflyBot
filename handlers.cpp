#include "handlers.h"

aegis::channel* chat_config::get_channel_safe(unsigned long long ch, std::shared_ptr<aegis::core>& bot) const
{
	std::shared_ptr<spdlog::logger> logg = bot->log;
	std::this_thread::yield();
	for (size_t tries = 0; tries < 10; tries++) {
		try {
			if (aegis::channel* found = bot->channel_create(ch); found) return found;
			else {
				logg->error("[!][{}/10] Guild #{} is having trouble while trying to find channel #{}.", tries + 1, this_guild_orig, ch);
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::seconds(2));
				std::this_thread::yield();
			}
		}
		catch (aegis::error e) {
			logg->error("[!] Exception at Guild #{}: {}.", this_guild_orig, e);
			logg->warn("[!][{}/10] Guild #{} will try to find channel #{} again in 5 second.", tries + 1, this_guild_orig, ch);

			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::seconds(5));
			std::this_thread::yield();
		}
		catch (std::exception e) {
			logg->error("[!] Exception at Guild #{}: {}.", this_guild_orig, e.what());
			logg->warn("[!][{}/10] Guild #{} will try to find channel #{} again in 5 second.", tries + 1, this_guild_orig, ch);

			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::seconds(5));
			std::this_thread::yield();
		}
	}
	logg->error("[!] Guild #{} couldn't find channel #{}.", this_guild_orig, ch);
	std::this_thread::yield();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::this_thread::yield();
	return nullptr;
}




chat_config::chat_config(const chat_config& cc)
{
	*this = cc;
}

void chat_config::load(nlohmann::json j, unsigned long long guild)
{
	this_guild_orig = guild;

	if (j.count("links") && !j["links"].is_null())
		links_orig = j["links"].get<unsigned long long>();

	if (j.count("files") && !j["files"].is_null())
		files_orig = j["files"].get<unsigned long long>();

	if (j.count("not_link_or_file") && !j["not_link_or_file"].is_null())
		not_link_or_file_orig = j["not_link_or_file"].get<unsigned long long>();

	if (j.count("link_overwrite_contains") && !j["link_overwrite_contains"].is_null()) {
		for (const auto& _field : j["link_overwrite_contains"])
			link_overwrite_contains.push_back(_field);
	}
	if (j.count("files_overwrite_contains") && !j["files_overwrite_contains"].is_null()) {
		for (const auto& _field : j["files_overwrite_contains"])
			files_overwrite_contains.push_back(_field);
	}
	if (j.count("not_link_or_file_overwrite_contains") && !j["not_link_or_file_overwrite_contains"].is_null()) {
		for (const auto& _field : j["not_link_or_file_overwrite_contains"])
			not_link_or_file_overwrite_contains.push_back(_field);
	}
}

nlohmann::json chat_config::export_config()
{
	nlohmann::json j;

	j["links"] = links_orig;
	j["files"] = files_orig;
	j["not_link_or_file"] = not_link_or_file_orig;

	for (const auto& _field : link_overwrite_contains)
		j["link_overwrite_contains"].push_back(_field);
	for (const auto& _field : files_overwrite_contains)
		j["files_overwrite_contains"].push_back(_field);
	for (const auto& _field : not_link_or_file_overwrite_contains)
		j["not_link_or_file_overwrite_contains"].push_back(_field);

	return j;
}

chat_config& chat_config::operator=(const chat_config& cc)
{
	this_guild_orig = cc.this_guild_orig;
	links_orig = cc.links_orig;
	not_link_or_file_orig = cc.not_link_or_file_orig;
	for (auto& i : cc.link_overwrite_contains) link_overwrite_contains.push_back(i);
	for (auto& i : cc.files_overwrite_contains) files_overwrite_contains.push_back(i);
	for (auto& i : cc.not_link_or_file_overwrite_contains) not_link_or_file_overwrite_contains.push_back(i);

	return *this;
}

void chat_config::handle_message(std::shared_ptr<aegis::core> core, aegis::gateway::objects::message msg) const
{
	// copy
	std::shared_ptr<spdlog::logger> logg = core->log;

	auto this_guild = this_guild_orig;
	auto links = links_orig;
	auto files = files_orig;
	auto nonee = not_link_or_file_orig;

	aegis::channel* fallback = fallback = get_channel_safe(msg.get_channel_id(), core);
	for (size_t p = 0; p < 7 && !fallback; p++) {
		fallback = get_channel_safe(msg.get_channel_id(), core);
		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		std::this_thread::yield();
	}

	unsigned long long
		chat_link = 0,
		chat_file = 0,
		chat_none = 0;

	/*
	IF value is 0			=> NOT FOUND
	ELSE IF value is 1		=> ERASE ONLY
	ELSE					=> COPY
	*/

		
	// >> LINK
	{
		if (std::regex_search(msg.get_content(), regex_link)) {

			std::lock_guard<std::mutex> luck(mute);
			for (auto& i : link_overwrite_contains) {
				if (msg.get_content().find(i.first) != std::string::npos) {
					chat_link = i.second;
					break;
				}
			}
			if (!chat_link) {
				chat_link = links;
			}
		}
	}

	// >> FILE
	if (msg.attachments.size() > 0) {
		auto& at = msg.attachments[0];
		std::lock_guard<std::mutex> luck(mute);
		for (auto& i : files_overwrite_contains) {
			if (at.filename.rfind(i.first) + i.first.length() == at.filename.length()) {
				chat_file = i.second;
				break;
			}
		}
		if (!chat_file) {
			for (auto& i : files_overwrite_contains) {
				if (at.filename.find(i.first) != std::string::npos) {
					chat_file = i.second;
					break;
				}
			}
		}
		if (!chat_file) {
			chat_file = files;
		}
	}

	if (!chat_link && !chat_file) {
		for (auto& i : not_link_or_file_overwrite_contains) {
			if (msg.get_content().find(i.first) != std::string::npos) {
				chat_none = i.second;
				break;
			}
		}
		if (!chat_none) {
			chat_none = nonee;
		}
	}





	if (chat_link > 1) { // has link, share link in links section (might have file)				<< LINKS
		aegis::channel* ch = get_channel_safe(chat_link, core);
		bool failure = true;
		
		if (ch) {
			std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + msg.get_content();
			if (to_send.length() > 2000) to_send = to_send.substr(0, 2000);

			if (slow_flush(to_send, *ch, this_guild, logg)) failure = false;
		}
		if (failure && fallback) {
			slow_flush("Failed to move your message.", *fallback, this_guild, logg);
		}
	}

	if (chat_file > 1) { // has file, share file in files section (might have link)				<< FILES
		aegis::channel* ch = get_channel_safe(chat_file, core);
		int failure = 1;

		if (ch) {
			std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + (chat_link ? "" : (" " + msg.get_content()));
			if (to_send.length() > 2000) to_send = to_send.substr(0, 2000);

			failure = false;

			auto& i = msg.attachments.front();
			Downloader down;
			down.getASync(i.url.c_str());

			logg->info("[!] Guild #{} is downloading {}...", this_guild, i.filename);

			while (!down.ended()) {
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

			logg->info("[!] Guild #{} downloaded {}. (size: {} byte(s))", this_guild, i.filename, down.read().size());

			if (down.read().size() >= MAX_FILE_SIZE) {
				failure = 2;
			}
			else {
				if (slow_flush(to_send, *ch, this_guild, logg)) {
					aegis::rest::aegis_file fp;
					fp.name = i.filename;
					for (auto& k : down.read()) fp.data.push_back(k);
					if (slow_flush(fp, *ch, this_guild, logg)) failure = 0;
				}
			}
		}

		if (failure && fallback) {
			slow_flush("Failed to move your file." + std::string(failure == 2 ? " File too big." : ""), *fallback, this_guild, logg);
		}
	}

	if (chat_link == 0 && chat_file == 0 && chat_none > 1) { // not link and no file, DEFAULT chat_none				<< NONES || just_delete_source

		if (msg.get_content().length()) {
			aegis::channel* ch = get_channel_safe(chat_none, core);
			bool failure = true;

			if (ch) {
				std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + msg.get_content();
				if (to_send.length() > 2000) to_send = to_send.substr(0, 2000);

				if (slow_flush(to_send, *ch, this_guild, logg)) failure = false;
			}
			if (failure && fallback) {
				slow_flush("Failed to move your message.", *fallback, this_guild, logg);
			}
		}
	}



	if (chat_link || chat_file || chat_none) {
		std::this_thread::yield();
		for (size_t tries = 0; tries < 7; tries++) {
			try {
				if (fallback) fallback->delete_message(msg.get_id());
				else logg->error("[!] Guild #{} can't delete source message.", this_guild);
				//else logging.print("[Local] Guild #", this_guild_orig, " can't delete source message.");
				//msg.delete_message(); // had link or file, so delete.
			}
			catch (aegis::error e) {
				logg->error("[!][{}/7] Guild #{} delete_message couldn't delete message. Got error: {}.", tries + 1, this_guild, e);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			}
			catch (std::exception e) {
				logg->error("[!][{}/7] Guild #{} delete_message couldn't delete message. Got error: {}.", tries + 1, this_guild, e.what());
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			}
			catch (...) {
				logg->error("[!][{}/7] Guild #{} delete_message couldn't delete message. Got unknown error.", tries + 1, this_guild);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			}
		}
	}
}


guild_data::guild_data(const guild_data& data)
{
	*this = data;
}

guild_data& guild_data::operator=(const guild_data& data)
{
	command_alias = data.command_alias;
	for (auto& i : data.admin_tags) admin_tags.push_back(i);
	for (auto& i : data.chats) chats[i.first] = i.second;
	return *this;
}

void GuildHandle::command(std::vector<std::string> args, aegis::channel& buck)
{
	const size_t len = args.size();

	bool save = false;
	bool well_done = false;

	auto common_help = [&] {
		std::string msg =
			"**Commands:**\n\n"
			"```md\n"
			"# Global:\n"
			"- debug - shows all config (in json)\n"
			"- alias <command_alias> - add an alias, like ~\n"
			"- admintag add/remove <id> - allow or not a tag to run these commands\n"
			"# Local:\n"
			"- delallconfig - removes ALL configuration in this chat\n"
			"- link <id> - set default link redirect\n"
			"- file <id> - set default file redirect\n"
			"- text <id> - set default text redirect (if not file nor link)\n"
			"- specific link <contains> <id> - if link has <contains>, redirect to <id> (if <id> is 0, it will only delete the source)\n"
			"- specific file <contains> <id> - if file has <contains>, redirect to <id> (if <id> is 0, it will only delete the source)\n"
			"- specific text <contains> <id> - if text has <contains>, redirect to <id> (not file/link only) (if <id> is 0, it will only delete the source)\n"
			"- specific remove * * - removes ALL specific rules (yes, two * *, one means any entry, second means any in those entries)\n"
			"- specific remove link <contains> - remove a rule set earlier (pro tip: * clear everything)\n"
			"- specific remove file <contains> - remove a rule set earlier (pro tip: * clear everything)\n```";

		slow_flush(msg, buck, guildid, logg);
	};

	switch (len) {
	case 2:
	{
		auto& yo = args[1];

		if (yo == "debug") {
			std::string uh = gen_json().dump(2);
			std::string slicu;

			for (size_t p = 0; p < uh.size(); p++) {
				slicu += uh[p];
				if (slicu.size() == 1900) {
					slow_flush("```\n" + slicu + "```", buck, guildid, logg);
					slicu.erase();
				}
			}
			if (slicu.size() > 0) {
				slow_flush("```\n" + slicu + "```", buck, guildid, logg);
				slicu.erase();
			}

			well_done = true;
		}
		else if (yo == "delallconfig") {
			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();

				
				for (auto it = data.chats.begin(); it != data.chats.end(); it++) {
					if ((*it).first == ch_id) {
						data.chats.erase(it);
						break;
					}
				}

				/*auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				friendo.link_overwrite_contains.clear();
				friendo.files_overwrite_contains.clear();
				friendo.not_link_or_file_overwrite_contains.clear();
				friendo.links_orig = 0;
				friendo.files_orig = 0;
				friendo.not_link_or_file_orig = 0;*/

				sending = "Cleaned up ALL rulesets and default rules of this chat.";
				save = true;
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
	}
		break;
	case 3: // <cmd> { alias, link, file, text } <id>
	{
		auto& yo = args[1];
		auto& res = args[2];

		if (yo == "alias") { // GLOBAL																			<< lsw/bb alias <id>
			data.mut.lock();
			data.command_alias = res;
			data.mut.unlock();
			slow_flush("Has set " + res + " as command alias successfully.", buck, guildid, logg);
			save = true;

			well_done = true;
		}
		else if (yo == "link") { // 																			<< lsw/bb link <id>

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				while (res.length() > 0) {
					if (!std::isdigit(res[0])) { // not a number
						res.erase(res.begin());
					}
					else break;
				}

				unsigned long long yaoo;
				
				if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {
					friendo.links_orig = yaoo ? yaoo : 1;
					if (yaoo) sending = "Has set <#" + std::to_string(yaoo) + "> as global link fallback successfully.";
					else sending = "Has set to just delete as global link fallback successfully.";
					save = true;
				}
				else {
					sending = "Failed to get the ID from your command.";
				}
			}			
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
		else if (yo == "text") { // 																			<< lsw/bb text <id>

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				while (res.length() > 0) {
					if (!std::isdigit(res[0])) { // not a number
						res.erase(res.begin());
					}
					else break;
				}

				unsigned long long yaoo;
				
				if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {
					friendo.not_link_or_file_orig = yaoo ? yaoo : 1;
					if (yaoo) sending = "Has set <#" + std::to_string(yaoo) + "> as global text fallback successfully.";
					else sending = "Has set to just delete as global text fallback successfully.";
					save = true;
				}
				else {
					sending = "Failed to get the ID from your command.";
				}
			}			
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
		else if (yo == "file") { // 																			<< lsw/bb file <id>

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				while (res.length() > 0) {
					if (!std::isdigit(res[0])) { // not a number
						res.erase(res.begin());
					}
					else break;
				}

				unsigned long long yaoo;
				
				if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {
					friendo.files_orig = yaoo ? yaoo : 1;
					if (yaoo) sending = "Has set <#" + std::to_string(yaoo) + "> as global file fallback successfully.";
					else sending = "Has set to just delete as global file fallback successfully.";
					save = true;
				}
				else {
					sending = "Failed to get the ID from your command.";
				}
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
	}
		break;
	case 4: // <cmd> { admintag add|remove } <id>
	{
		auto& yo = args[1];
		auto& yo2 = args[2];
		auto& res = args[3];

		if (yo == "admintag") { // GLOBAL																			<< lsw/bb alias <id>
			
			if (yo2 == "add") {

				std::string sending;
				{
					std::lock_guard<std::mutex> luck_me(data.mut);

					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;
					
					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {

						bool already_there = false;

						for (auto& i : data.admin_tags) {
							already_there = (i == yaoo);
							if (already_there) break;
						}

						if (!already_there) {
							data.admin_tags.push_back(yaoo);
							sending = "Added <@&" + std::to_string(yaoo) + "> as admin.";
							save = true;
						}
						else {
							sending = "<@&" + std::to_string(yaoo) + "> already set as admin.";
						}
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

				well_done = true;
			}
			else if (yo2 == "remove") {

				std::string sending;
				{
					std::lock_guard<std::mutex> luck_me(data.mut);

					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;
					
					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {

						bool found_there = false;

						for (size_t p = 0; p < data.admin_tags.size(); p++) {
							if (data.admin_tags[p] == yaoo) {
								data.admin_tags.erase(data.admin_tags.begin() + p);
								found_there = true;
								break;
							}
						}

						if (found_there) {
							data.admin_tags.push_back(yaoo);
							sending = "Removed <@&" + std::to_string(yaoo) + "> from admin list.";
							save = true;
						}
						else {
							sending = "<@&" + std::to_string(yaoo) + "> was not an admin.";
						}
					}
					else {
						sending = "Failed to get the ID from your command.", buck, guildid;
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

				well_done = true;
			}
		}
	}
		break;
	case 5: // <cmd> { specific [link|file|text <contains> <id>] | [specific remove link|file|text <contains>] } 
	{
		auto& yo = args[1];
		auto& yo2 = args[2];
		auto& yo3 = args[3];
		auto& res = args[4];

		if (yo == "specific") {
			if (yo2 == "link" && yo3 != "*") {

				std::string sending;
				{
					std::lock_guard<std::mutex> luck_me(data.mut);
					auto ch_id = buck.get_id();
					auto& friendo = data.chats[ch_id];
					std::lock_guard<std::mutex> luck(friendo.mute);

					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;
					
					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {

						bool already_got = false;

						for (auto& h : friendo.link_overwrite_contains) {
							if (h.first == yo3) {
								h.second = yaoo ? yaoo : 1;
								already_got = true;
								break;
							}
						}

						if (!already_got) friendo.link_overwrite_contains.push_back({ yo3, yaoo ? yaoo : 1 });

						if (yaoo) sending = "Added/updated ruleset (link) \"" + yo3 + "\" -> <#" + std::to_string(yaoo) + "> successfully";
						else sending = "Added/updated ruleset (link) \"" + yo3 + "\" -> DELETE successfully";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

				well_done = true;
			}
			else if (yo2 == "text" && yo3 != "*") {

				std::string sending;
				{
					std::lock_guard<std::mutex> luck_me(data.mut);
					auto ch_id = buck.get_id();
					auto& friendo = data.chats[ch_id];
					std::lock_guard<std::mutex> luck(friendo.mute);

					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;
					
					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {

						bool already_got = false;

						for (auto& h : friendo.not_link_or_file_overwrite_contains) {
							if (h.first == yo3) {
								h.second = yaoo ? yaoo : 1;
								already_got = true;
								break;
							}
						}

						if (!already_got) friendo.not_link_or_file_overwrite_contains.push_back({ yo3, yaoo ? yaoo : 1 });

						if (yaoo) sending = "Added/updated ruleset (text) \"" + yo3 + "\" -> <#" + std::to_string(yaoo) + "> successfully";
						else sending = "Added/updated ruleset (text) \"" + yo3 + "\" -> DELETE successfully";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

				well_done = true;
			}
			else if (yo2 == "file" && yo3 != "*") {

				std::string sending;
				{
					std::lock_guard<std::mutex> luck_me(data.mut);
					auto ch_id = buck.get_id();
					auto& friendo = data.chats[ch_id];
					std::lock_guard<std::mutex> luck(friendo.mute);

					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;
					
					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {

						bool already_got = false;

						for (auto& h : friendo.files_overwrite_contains) {
							if (h.first == yo3) {
								h.second = yaoo ? yaoo : 1;
								already_got = true;
								break;
							}
						}

						if (!already_got) friendo.files_overwrite_contains.push_back({ yo3, yaoo ? yaoo : 1 });

						if (yaoo) sending = "Added/updated ruleset (file) \"" + yo3 + "\" -> <#" + std::to_string(yaoo) + "> successfully";
						else sending = "Added/updated ruleset (file) \"" + yo3 + "\" -> DELETE successfully";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

				well_done = true;
			}
			else if (yo2 == "remove") { // remember * = all

				if (yo3 == "*" && res == "*") {

					std::string sending;
					{
						std::lock_guard<std::mutex> luck_me(data.mut);
						auto ch_id = buck.get_id();
						auto& friendo = data.chats[ch_id];
						std::lock_guard<std::mutex> luck(friendo.mute);

						friendo.link_overwrite_contains.clear();
						friendo.files_overwrite_contains.clear();
						friendo.not_link_or_file_overwrite_contains.clear();

						sending = "Cleaned up ALL rulesets of this chat.";
						save = true;
					}
					if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

					well_done = true;
				}
				else if (yo3 == "link") {

					std::string sending;
					{
						std::lock_guard<std::mutex> luck_me(data.mut);
						auto ch_id = buck.get_id();
						auto& friendo = data.chats[ch_id];
						std::lock_guard<std::mutex> luck(friendo.mute);

						if (res == "*") {
							friendo.link_overwrite_contains.clear();

							sending = "Cleaned up ALL LINK rulesets of this chat.";
							save = true;
						}
						else { // contains

							bool found = false;

							for (size_t p = 0; p < friendo.link_overwrite_contains.size(); p++) {
								if (friendo.link_overwrite_contains[p].first == res) {
									friendo.link_overwrite_contains.erase(friendo.link_overwrite_contains.begin() + p);
									found = true;
									break;
								}
							}

							if (found) {
								sending = "Removed ruleset (link) with key \"" + res + "\" successfully.";
								save = true;
							}
							else {
								sending = "Can't find any ruleset (link) with key \"" + res + "\".";
							}
						}
					}
					if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

					well_done = true;
				}
				else if (yo3 == "text") {

					std::string sending;
					{
						std::lock_guard<std::mutex> luck_me(data.mut);
						auto ch_id = buck.get_id();
						auto& friendo = data.chats[ch_id];
						std::lock_guard<std::mutex> luck(friendo.mute);

						if (res == "*") {
							friendo.not_link_or_file_overwrite_contains.clear();

							sending = "Cleaned up ALL TEXT rulesets of this chat.";
							save = true;
						}
						else { // contains

							bool found = false;

							for (size_t p = 0; p < friendo.not_link_or_file_overwrite_contains.size(); p++) {
								if (friendo.not_link_or_file_overwrite_contains[p].first == res) {
									friendo.not_link_or_file_overwrite_contains.erase(friendo.not_link_or_file_overwrite_contains.begin() + p);
									found = true;
									break;
								}
							}

							if (found) {
								sending = "Removed ruleset (text) with key \"" + res + "\" successfully.";
								save = true;
							}
							else {
								sending = "Can't find any ruleset (text) with key \"" + res + "\".";
							}
						}
					}
					if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

					well_done = true;
				}
				else if (yo3 == "file") {

					std::string sending;
					{
						std::lock_guard<std::mutex> luck_me(data.mut);
						auto ch_id = buck.get_id();
						auto& friendo = data.chats[ch_id];
						std::lock_guard<std::mutex> luck(friendo.mute);

						if (res == "*") {
							friendo.files_overwrite_contains.clear();

							sending = "Cleaned up ALL FILE rulesets of this chat.";
							save = true;
						}
						else { // contains

							bool found = false;

							for (size_t p = 0; p < friendo.files_overwrite_contains.size(); p++) {
								if (friendo.files_overwrite_contains[p].first == res) {
									friendo.files_overwrite_contains.erase(friendo.files_overwrite_contains.begin() + p);
									found = true;
									break;
								}
							}

							if (found) {
								sending = "Removed ruleset (file) with key \"" + res + "\" successfully.";
								save = true;
							}
							else {
								sending = "Can't find any ruleset (file) with key \"" + res + "\".";
							}
						}
					}
					if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

					well_done = true;
				}
				
			}
		}
	}
		break;
	}

	if (!well_done) {
		common_help();
	}
	if (save) save_settings();
}

bool GuildHandle::has_admin_rights(aegis::guild& guild, aegis::user& user)
{
	std::lock_guard<std::mutex> luck_me(data.mut);

	if (user.get_id() == guild.get_owner() || user.get_id() == mee_dev) return true;

	auto tags = user.get_guild_info(guildid).roles;

	//std::lock_guard<std::mutex> keep(besur);

	for (auto& i : tags) {
		for (auto& j : data.admin_tags) {
			if (j == i) {
				return true;
			}
		}
	}
	return false;

}

nlohmann::json GuildHandle::gen_json()
{
	nlohmann::json sav;
	sav["command_alias"] = data.command_alias;

	for (const auto& _field : data.admin_tags)
		sav["admin_tags"].push_back(_field);

	for (auto& i : data.chats)
		sav["chats_config"].push_back(std::pair<unsigned long long, nlohmann::json>(i.first, i.second.export_config()));

	return sav;
}

/*
std::string command_alias;
std::vector<unsigned long long> admin_tags;
std::map<unsigned long long, chat_config> chats;
*/

void GuildHandle::save_settings()
{
	nlohmann::json sav = gen_json();

	CreateDirectoryA("servers", NULL);

	std::string pathh = "servers/" + std::to_string(guildid) + "_data.bbv1";
	std::shared_ptr<FILE> fpp;
	FILE* haha = nullptr;

	if (fopen_s(&haha, pathh.c_str(), "wb") == 0) {
		std::shared_ptr<FILE> fpp = std::shared_ptr<FILE>(haha, [](FILE* f) {fclose(f); });
		auto cpy = sav.dump();
		fwrite(cpy.c_str(), sizeof(char), cpy.length(), fpp.get());

		logg->info("[!] Guild #{} saved config successfully.", guildid);
	}
	else {
		logg->error("[!] Guild #{} failed to save config.", guildid);
	}
}

void GuildHandle::load_settings()
{
	std::string pathh = "servers/" + std::to_string(guildid) + "_data.bbv1";

	FILE* haha = nullptr;
	if (fopen_s(&haha, pathh.c_str(), "rb") == 0) {
		std::shared_ptr<FILE> fpp = std::shared_ptr<FILE>(haha, [](FILE* f) {fclose(f); });
		std::string all_buf;
		char quickbuf[256];
		size_t u = 0;
		while (u = fread_s(quickbuf, 256, sizeof(char), 256, fpp.get())) {
			for (size_t k = 0; k < u; k++) all_buf += quickbuf[k];
		}

		nlohmann::json j = nlohmann::json::parse(all_buf);


		if (j.count("command_alias") && !j["command_alias"].is_null())
			data.command_alias = j["command_alias"].get<std::string>();
		if (j.count("admin_tags") && !j["admin_tags"].is_null())
			data.command_alias = j["admin_tags"].get<std::string>();

		if (j.count("admin_tags") && !j["admin_tags"].is_null()) {
			for (const auto& _field : j["admin_tags"])
				data.admin_tags.push_back(j["admin_tags"].get<unsigned long long>());
		}

		if (j.count("chats_config") && !j["chats_config"].is_null()) {
			for (const auto& _field : j["chats_config"]) {
				auto translate = _field.get<std::pair<unsigned long long, nlohmann::json>>();
				chat_config chat;
				chat.load(translate.second, guildid);
				data.chats[translate.first] = chat;
			}
		}

		logg->info("[!] Guild #{} loaded config successfully.", guildid);
	}
	else {
		logg->warn("[!] Guild #{} failed to load config. (new?)", guildid);
	}
}

GuildHandle::GuildHandle(std::shared_ptr<aegis::core> bot, aegis::snowflake guild)
{
	core = bot;
	logg = bot->log;
	guildid = guild;

	//pendency_keep_run = true;
	//pendency_handler = std::thread([&] {handle_pendencies(); });
}

/*GuildHandle::GuildHandle(const GuildHandle& gh)
{
	*this = gh;
}*/

GuildHandle::~GuildHandle()
{
	end();
}

void GuildHandle::end()
{
	if (keep_run) {
		keep_run = false;
		//if (pendency_handler.joinable()) pendency_handler.join();
		save_settings();
	}
}

GuildHandle& GuildHandle::start()
{
	if (!keep_run) {
		load_settings();

		keep_run = true;
		//pendency_handler = std::thread([&] {handle_pendencies(); });
	}
	return *this;
}

/*GuildHandle& GuildHandle::operator=(const GuildHandle& gh)
{
	data = gh.data;
	core = gh.core;
	guildid = gh.guildid;

	return *this;
}*/

bool GuildHandle::amI(aegis::snowflake id)
{
	return guildid == id;
}

void GuildHandle::handle(aegis::channel& src, aegis::gateway::objects::message m)
{
	if (m.get_content().find(main_cmd) == 0 || ((data.command_alias.length() > 0) && m.get_content().find(data.command_alias) == 0)) {

		if (!has_admin_rights(m.get_guild(), m.get_user())) { // adm/owner/me
			slow_flush("You have no permission.", src, guildid, logg);
			m.delete_message();
			return; 
		}
		size_t start = m.get_content().find(main_cmd) == 0 ? main_cmd.length() : data.command_alias.size();

		std::vector<std::string> arguments;

		arguments.push_back(main_cmd);

		const std::string& slice = m.get_content();
		{
			std::string buf;
			for (size_t p = start; p < slice.length(); p++) {
				auto& i = slice[p];
				if (i != ' ') buf += slice[p];
				else if (!buf.empty()) arguments.push_back(std::move(buf));
			}
			if (!buf.empty()) arguments.push_back(std::move(buf));
		}
		command(arguments, src);
	}
	else {
		//std::lock_guard<std::mutex> luck(pendency_mutex);
		//pendencies.push_back(m);
		data.chats[src.get_id()].handle_message(core, m);
	}
}
