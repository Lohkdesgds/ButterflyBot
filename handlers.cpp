#include "handlers.h"

/*custom_message_save::custom_message_save(aegis::gateway::objects::message& m)
{
	aegis::gateway::objects::message cpy = m;
	last_id = m.get_id();
	last_chat = m.get_channel_id();
	timestamp = m.timestamp;
	username = m.author.username;
	discriminator = m.author.discriminator;
	content = m.get_content();
	for (auto& i : m.reactions) {
		reactions.push_back({ i.emoji_.id != 0 ? ((i.emoji_.animated ? u8"<a:" : u8"<:") + i.emoji_.name + u8":" + std::to_string(i.emoji_.id) + u8">") : i.emoji_.name, i.count });
	}
	for (auto& i : m.embeds) {
		nlohmann::json jss;
		aegis::gateway::objects::to_json(jss, i);
		embeds_json.push_back(jss.dump());
	}
	for (auto& i : m.attachments) {
		attachments.push_back({ i.url, i.filename });
	}
}*/




bool slow_flush(aegis::create_message_t m, aegis::channel& ch, unsigned long long this_guild_orig) {
	std::this_thread::yield();
	if (m._content.empty()) return false;
	for (size_t tries = 0; tries < 7; tries++) {
		try {
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			ch.create_message(m).get();
			/*std::this_thread::sleep_for(std::chrono::milliseconds(800));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();*/
			return true;
		}
		catch (aegis::error e) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlush couldn't send message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (std::exception e) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlush couldn't send message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (...) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlush couldn't send message. Unknown error.");
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
	}
	return false;
}

bool slow_flush(std::string str, aegis::channel& ch, unsigned long long this_guild_orig) {
	return slow_flush(aegis::create_message_t().content(str), ch, this_guild_orig);
}

bool slow_flush_embed(nlohmann::json emb, aegis::channel& ch, unsigned long long this_guild_orig) {
	if (emb.empty()) return false;
	std::this_thread::yield();
	for (size_t tries = 0; tries < 7; tries++) {
		try {
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			ch.create_message_embed({}, emb).get();
			/*std::this_thread::sleep_for(std::chrono::milliseconds(800));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();*/
			return true;
		}
		catch (aegis::error e) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlushEmbed couldn't send message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (std::exception e) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlushEmbed couldn't send message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (...) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlushEmbed couldn't send message. Unknown error.");
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
	}
	return false;
}

bool slow_flush_embed(aegis::gateway::objects::embed emb, aegis::channel& ch, unsigned long long this_guild_orig) {
	std::this_thread::yield();
	for (size_t tries = 0; tries < 7; tries++) {
		try {
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			ch.create_message_embed({}, emb).get();
			/*std::this_thread::sleep_for(std::chrono::milliseconds(800));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();*/
			return true;
		}
		catch (aegis::error e) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlushEmbed couldn't send message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (std::exception e) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlushEmbed couldn't send message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (...) {
			logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " SlowFlushEmbed couldn't send message. Unknown error.");
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
	}
	return false;
}



aegis::channel* chat_config::get_channel_safe(unsigned long long ch, std::shared_ptr<aegis::core>& bot) const
{
	std::this_thread::yield();
	for (size_t tries = 0; tries < 10; tries++) {
		try {
			if (aegis::channel* found = bot->channel_create(ch); found) return found;
			else {
				logging.print("[", tries + 1, "/10] Guild #", this_guild_orig, " is having trouble while trying to find channel #", ch, ".");
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::seconds(2));
				std::this_thread::yield();
			}
		}
		catch (aegis::error e) {
			logging.print("[!] Got exception at Guild #", this_guild_orig, ": ", e);
			logging.print("[", tries + 1, "/10] Guild #", this_guild_orig, " trying to find channel #", ch, " again in 5 seconds.");
			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::seconds(5));
			std::this_thread::yield();
		}
	}
	logging.print("[!] Guild #", this_guild_orig, " couldn't find channel #", ch, ".");
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

	if (j.count("link_overwrite_contains") && !j["link_overwrite_contains"].is_null()) {
		for (const auto& _field : j["link_overwrite_contains"])
			link_overwrite_contains.push_back(_field);
	}
	if (j.count("files_overwrite_contains") && !j["files_overwrite_contains"].is_null()) {
		for (const auto& _field : j["files_overwrite_contains"])
			files_overwrite_contains.push_back(_field);
	}
}

nlohmann::json chat_config::export_config()
{
	nlohmann::json j;

	j["links"] = links_orig;
	j["files"] = files_orig;

	for (const auto& _field : link_overwrite_contains)
		j["link_overwrite_contains"].push_back(_field);
	for (const auto& _field : files_overwrite_contains)
		j["files_overwrite_contains"].push_back(_field);

	return j;
}

chat_config& chat_config::operator=(const chat_config& cc)
{
	this_guild_orig = cc.this_guild_orig;
	links_orig = cc.links_orig;
	files_orig = cc.files_orig;
	for (auto& i : cc.link_overwrite_contains) link_overwrite_contains.push_back(i);
	for (auto& i : cc.files_overwrite_contains) files_overwrite_contains.push_back(i);

	return *this;
}

void chat_config::handle_message(std::shared_ptr<aegis::core> core, aegis::gateway::objects::message msg) const
{
	// copy
	auto this_guild = this_guild_orig;
	auto links = links_orig;
	auto files = files_orig;

	aegis::channel* fallback = get_channel_safe(msg.get_channel_id(), core);

	unsigned long long
		chat_link = 0,
		chat_file = 0;
		
	{
		std::lock_guard<std::mutex> luck(mute);
		for (auto& i : link_overwrite_contains) {
			if (msg.get_content().find(i.first) != std::string::npos) {
				chat_link = i.second;
				break;
			}
		}
		if (!chat_link && msg.get_content().find("http") != std::string::npos) {
			chat_link = links;
		}
	}
	// if chat_link has value, send there.


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
	// if chat_file has value, there is file. Allow only one tho. Better.


	if (chat_link) { // has link, share link in links section (might have file)
		aegis::channel* ch = get_channel_safe(chat_link, core);
		bool failure = true;
		
		if (ch) {
			std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + msg.get_content();
			if (to_send.length() > 2000) to_send = to_send.substr(0, 2000);

			if (slow_flush(to_send, *ch, this_guild)) failure = false;
		}
		if (failure && fallback) {
			slow_flush("Failed to move your message.", *fallback, this_guild);
		}
	}
	if (chat_file) { // has file, share file in files section (might have link)
		aegis::channel* ch = get_channel_safe(chat_file, core);
		int failure = 1;

		if (ch) {
			std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + (chat_link ? "" : (" " + msg.get_content()));
			if (to_send.length() > 2000) to_send = to_send.substr(0, 2000);

			failure = false;

			auto& i = msg.attachments.front();
			Downloader down;
			down.getASync(i.url.c_str());

			logging.print("[!] Guild #", this_guild_orig, " is downloading ", i.filename, "...");

			while (!down.ended()) {
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

			logging.print("[!] Guild #", this_guild_orig, " downloaded ", i.filename, ". (size: ", down.read().size(), " byte(s))");

			if (down.read().size() >= MAX_FILE_SIZE) {
				failure = 2;
			}
			else {
				if (slow_flush(to_send, *ch, this_guild)) {
					aegis::create_message_t msg2;
					aegis::rest::aegis_file fp;
					fp.name = i.filename;
					for (auto& k : down.read()) fp.data.push_back(k);
					msg2.file(fp);

					if (slow_flush(msg2, *ch, this_guild)) failure = 0;
				}
			}
		}

		if (failure && fallback) {
			slow_flush("Failed to move your file." + std::string(failure == 2 ? " File too big." : ""), *fallback, this_guild);
		}
	}
	/*if (!chat_link && !chat_file) { // if there's no file and link...
	  // maybe later a autocopy?
	}*/

	if (chat_link || chat_file) {
		std::this_thread::yield();
		for (size_t tries = 0; tries < 7; tries++) {
			try {
				msg.delete_message(); // had link or file, so delete.
			}
			catch (aegis::error e) {
				logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " delete_message couldn't delete message. Got error: ", e);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			}
			catch (std::exception e) {
				logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " delete_message couldn't delete message. Got error: ", e); 
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				for (size_t p = 0; p < 4; p++) std::this_thread::yield();
			}
			catch (...) {
				logging.print("[Local][", tries + 1, "/7] Guild #", this_guild_orig, " delete_message couldn't delete message. Unknown error.");
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

/*void GuildHandle::handle_pendencies()
{
	while (keep_run) {
		
		std::this_thread::yield();

		if (pendencies.size() == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			std::this_thread::yield();
			continue;
		}

		aegis::gateway::objects::message cpy;
		bool good = false;
		{
			if (pendency_mutex.try_lock()) {
				cpy = pendencies.front();
				pendencies.erase(pendencies.begin());
				good = true;
				pendency_mutex.unlock();
			}
		}

		if (!good) {
			logging.print("[!] Guild #", guildid, " failed to task pendencies once.");
			std::this_thread::yield();
			continue;
		}

		try {
			data.chats[cpy.get_channel_id()].handle_message(core, cpy);
		}
		catch (aegis::error e) {
			logging.print("[!] Guild #", guildid, " couldn't handle message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (std::exception e) {
			logging.print("[!] Guild #", guildid, " couldn't handle message. Got error: ", e);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (...) {
			logging.print("[!] Guild #", guildid, " couldn't handle message. Unknown error.");
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
	}
}*/

void GuildHandle::command(std::vector<std::string> args, aegis::channel& buck)
{
	const size_t len = args.size();

	bool save = false;
	bool well_done = false;

	auto common_help = [&] {
		std::string msg =
			"**Commands:**\n\n"
			"Global:\n"
			"- debug - shows all config (in json)\n"
			"- alias <command alias> - add an alias, like ~\n"
			"- admintag add/remove <id> - allow or not a tag to run these commands\n"
			"Local:\n"
			"- link <id> - set default link redirect\n"
			"- file <id> - set default file redirect\n"
			"- specific link <contains> <id> - set if link has <contains> redirect to <id>\n"
			"- specific file <contains> <id> - set if file has <contains> in its name redirect to <id>\n"
			"- specific remove \\* \\* - removes ALL specific rules (yes, two \\* \\*, one means any entry, second means any in those entries)\n"
			"- specific remove link <contains> - remove a setting set earlier (pro tip: * clear everything)\n"
			"- specific remove file <contains> - remove a setting set earlier (pro tip: * clear everything)\n";

		slow_flush(msg, buck, guildid);
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
					slow_flush("```\n" + slicu + "```", buck, guildid);
					slicu.erase();
				}
			}
			if (slicu.size() > 0) {
				slow_flush("```\n" + slicu + "```", buck, guildid);
				slicu.erase();
			}

			well_done = true;
		}
	}
		break;
	case 3: // <cmd> { alias, link, file } <id>
	{
		auto& yo = args[1];
		auto& res = args[2];

		if (yo == "alias") { // GLOBAL																			<< lsw/bb alias <id>
			data.mut.lock();
			data.command_alias = res;
			data.mut.unlock();
			slow_flush("Has set " + res + " as command alias successfully.", buck, guildid);
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
					friendo.links_orig = yaoo;
					sending = "Has set <#" + std::to_string(yaoo) + "> as global link fallback successfully.";
					save = true;
				}
				else {
					sending = "Failed to get the ID from your command.";
				}
			}			
			if (sending.length() > 0) slow_flush(sending, buck, guildid);

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
					friendo.files_orig = yaoo;
					sending = "Has set <#" + std::to_string(yaoo) + "> as global file fallback successfully.";
					save = true;
				}
				else {
					sending = "Failed to get the ID from your command.";
				}
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid);

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
				if (sending.length() > 0) slow_flush(sending, buck, guildid);

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
				if (sending.length() > 0) slow_flush(sending, buck, guildid);

				well_done = true;
			}
		}
	}
		break;
	case 5: // <cmd> { specific [link|file <contains> <id>] | [specific remove link|file <contains>] } 
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
								h.second = yaoo;
								already_got = true;
								break;
							}
						}

						if (!already_got) friendo.link_overwrite_contains.push_back({ yo3, yaoo });

						sending = "Added/updated ruleset (link) \"" + yo3 + "\" -> <#" + std::to_string(yaoo) + "> successfully";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid);

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
								h.second = yaoo;
								already_got = true;
								break;
							}
						}

						if (!already_got) friendo.files_overwrite_contains.push_back({ yo3, yaoo });

						sending = "Added/updated ruleset (file) \"" + yo3 + "\" -> <#" + std::to_string(yaoo) + "> successfully";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
				if (sending.length() > 0) slow_flush(sending, buck, guildid);

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

						sending = "Cleaned up ALL rulesets of this chat.";
						save = true;
					}
					if (sending.length() > 0) slow_flush(sending, buck, guildid);

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
					if (sending.length() > 0) slow_flush(sending, buck, guildid);

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
					if (sending.length() > 0) slow_flush(sending, buck, guildid);

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

		logging.print("[!] Guild #", guildid, " saved config successfully.");
	}
	else {
		logging.print("[!] Guild #", guildid, " failed to save config. Aborting save.");
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

		logging.print("[!] Guild #", guildid, " loaded config successfully.");
	}
	else {
		logging.print("[!] Guild #", guildid, " failed to load config. (new?)");
	}
}

GuildHandle::GuildHandle(std::shared_ptr<aegis::core> bot, aegis::snowflake guild)
{
	core = bot;
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
			slow_flush("You have no permission.", src, guildid);
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
