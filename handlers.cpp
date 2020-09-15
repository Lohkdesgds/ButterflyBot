#include "handlers.h"



/*void handle_in_steps::add(std::function<void(void)> f)
{
	while (!m.try_lock()) std::this_thread::yield();
	list.emplace_back(std::move(f));
	m.unlock();
}

bool handle_in_steps::task()
{
	std::function<void(void)> ff;
	bool uu = false;
	m.lock();
	if (list.size()) {
		ff = list.front();
		list.erase(list.begin());
		uu = true;
	}
	m.unlock();
	if (uu && ff) {
		try {
			ff();
		}
		catch (aegis::error e) {
			std::cout << "AEGIS FATAL ERROR: " << e << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (std::exception e) {
			std::cout << "STD FATAL ERROR: " << e.what() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
		catch (...) {
			std::cout << "AEGIS FATAL ERROR" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			for (size_t p = 0; p < 4; p++) std::this_thread::yield();
		}
	}
	std::this_thread::yield();
	std::this_thread::sleep_for(std::chrono::milliseconds(750));
	std::this_thread::yield();
	return uu;
}*/

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

nlohmann::json chat_config::embed_link_gen(const bool show_link, const unsigned long long cid, const unsigned long long mid, const unsigned long long uid, const std::string uurl, const std::string ustr, const std::string desc, const std::string imgurl) const
{
	// message link: https://discordapp.com/channels/<group>/<channel>/<message>

	nlohmann::json j;
	j["color"] = default_color;
	j["author"]["name"] = ustr;
	j["author"]["icon_url"] = uurl;
	if (show_user_thumbnail) j["thumbnail"]["url"] = uurl;
	j["footer"]["text"] = std::to_string(uid);

	if (show_link) {
		const std::string url_m = "https://discordapp.com/channels/" + std::to_string(this_guild_orig) + "/" + std::to_string(cid) + "/" + std::to_string(mid);
		j["url"] = url_m;
		if (desc.length()) j["description"] = (show_link ? ("[`-> source <-`](" + url_m + ")") : "") + (cid ? "<#" + std::to_string(cid) + ">" : "") + (desc.length() ? ((dont_use_md ? "\n\n" : "\n```m\n") + desc + (dont_use_md ? "" : "```")) : "");
	}
	else if (desc.length()) {
		j["description"] = (cid ? "<#" + std::to_string(cid) + ">" : "") + (dont_use_md ? "\n\n" : "\n```m\n") + desc + (dont_use_md ? "" : "```");
	}
	else if (cid) {
		j["description"] = ("<#" + std::to_string(cid) + ">");
	}

	if (!imgurl.empty()) j["image"]["url"] = imgurl;

	return std::move(j);
}

nlohmann::json chat_config::embed_link_gen(const unsigned long long cid, const unsigned long long uid, const std::string uurl, const std::string ustr, const std::string desc, const std::string imgurl) const
{
	return embed_link_gen(false, 0, cid, uid, uurl, ustr, desc, imgurl);
}

void chat_config::handle_poll(slowflush_end& e) const
{
	if (!e.good() || !autopoll) return;

	auto fuu = e.create_reaction("%F0%9F%91%8D");
	while (!fuu.available()) std::this_thread::sleep_for(std::chrono::milliseconds(50));
	auto fuu2 = e.create_reaction("%F0%9F%91%8E");
	while (!fuu2.available()) std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

std::string chat_config::_fix_content(const std::string& str) const
{
	std::string cpy = str;
	/*for (auto& i : str) {
		if (i == '`' || i == '@') cpy += '\\';
		cpy += i;
	}*/
	std::smatch m;
	std::string wrk;
	while (std::regex_search(cpy, m, regex_link)) {
		wrk += cpy.substr(0, m.position(0)) + "<" + m[0].str() + ">";
		cpy = (cpy.length() > (m[0].str().length() + m.position(0)) ? (cpy.substr(m[0].str().length() + m.position(0))) : "");
	}
	if (!cpy.empty()) wrk += cpy;
	wrk = wrk.substr(0, safe_msg_limit);
	return std::move(wrk);
}




chat_config::chat_config(const chat_config& cc)
{
	*this = cc;
}

void chat_config::load(nlohmann::json j, unsigned long long guild)
{
	this_guild_orig = guild;

	if (j.count("show_user_thumbnail") && !j["show_user_thumbnail"].is_null())
		show_user_thumbnail = j["show_user_thumbnail"].get<bool>();

	if (j.count("dont_use_md") && !j["dont_use_md"].is_null())
		dont_use_md = j["dont_use_md"].get<bool>();

	if (j.count("autopoll") && !j["autopoll"].is_null())
		autopoll = j["autopoll"].get<bool>();

	if (j.count("embed_fallback") && !j["embed_fallback"].is_null())
		embed_fallback = j["embed_fallback"].get<unsigned long long>();


	if (j.count("links") && !j["links"].is_null())
		links_orig = j["links"].get<unsigned long long>();

	if (j.count("files") && !j["files"].is_null())
		files_orig = j["files"].get<unsigned long long>();

	if (j.count("not_link_or_file") && !j["not_link_or_file"].is_null())
		nlf_orig = j["not_link_or_file"].get<unsigned long long>();


	if (j.count("links_refback") && !j["links_refback"].is_null())
		link_ref_back = j["links_refback"].get<bool>();

	if (j.count("files_refback") && !j["files_refback"].is_null())
		files_ref_back = j["files_refback"].get<bool>();

	if (j.count("not_link_or_file_refback") && !j["not_link_or_file_refback"].is_null())
		nlf_ref_back = j["not_link_or_file_refback"].get<bool>();


	if (j.count("links_refback_show") && !j["links_refback_show"].is_null())
		link_ref_show_ref = j["links_refback_show"].get<bool>();

	if (j.count("files_refback_show") && !j["files_refback_show"].is_null())
		files_ref_show_ref = j["files_refback_show"].get<bool>();

	if (j.count("not_link_or_file_refback_show") && !j["not_link_or_file_refback_show"].is_null())
		nlf_ref_show_ref = j["not_link_or_file_refback_show"].get<bool>();


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
			nlf_overwrite_contains.push_back(_field);
	}
}

nlohmann::json chat_config::export_config()
{
	nlohmann::json j;

	j["show_user_thumbnail"] = show_user_thumbnail;
	j["dont_use_md"] = dont_use_md;
	j["autopoll"] = autopoll;
	j["embed_fallback"] = embed_fallback;

	j["links"] = links_orig;
	j["files"] = files_orig;
	j["not_link_or_file"] = nlf_orig;

	j["links_refback"] = link_ref_back;
	j["files_refback"] = files_ref_back;
	j["not_link_or_file_refback"] = nlf_ref_back;

	j["links_refback_show"] = link_ref_show_ref;
	j["files_refback_show"] = files_ref_show_ref;
	j["not_link_or_file_refback_show"] = nlf_ref_show_ref;

	for (const auto& _field : link_overwrite_contains)
		j["link_overwrite_contains"].push_back(_field);
	for (const auto& _field : files_overwrite_contains)
		j["files_overwrite_contains"].push_back(_field);
	for (const auto& _field : nlf_overwrite_contains)
		j["not_link_or_file_overwrite_contains"].push_back(_field);

	return j;
}

chat_config& chat_config::operator=(const chat_config& cc)
{
	this_guild_orig = cc.this_guild_orig;

	show_user_thumbnail = cc.show_user_thumbnail;
	dont_use_md = cc.dont_use_md;
	autopoll = cc.autopoll;
	embed_fallback = cc.embed_fallback;

	links_orig = cc.links_orig;
	files_orig = cc.files_orig;
	nlf_orig = cc.nlf_orig;

	link_ref_back = cc.link_ref_back;
	files_ref_back = cc.files_ref_back;
	nlf_ref_back = cc.nlf_ref_back;

	link_ref_show_ref = cc.link_ref_show_ref;
	files_ref_show_ref = cc.files_ref_show_ref;
	nlf_ref_show_ref = cc.nlf_ref_show_ref;

	for (auto& i : cc.link_overwrite_contains) link_overwrite_contains.push_back(i);
	for (auto& i : cc.files_overwrite_contains) files_overwrite_contains.push_back(i);
	for (auto& i : cc.nlf_overwrite_contains) nlf_overwrite_contains.push_back(i);

	return *this;
}

void chat_config::handle_message(std::shared_ptr<aegis::core> core, aegis::gateway::objects::message msg) const
{
	// copy
	std::shared_ptr<spdlog::logger> logg = core->log;
	//logg->info("Handling message...");

	auto this_guild = this_guild_orig;
	auto links = links_orig;
	auto files = files_orig;
	auto nonee = nlf_orig;
	auto content = msg.get_content();// .substr(0, safe_msg_limit);

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

	// if !dont_use_md, remove ``` (because those will break the block)
	if (!dont_use_md) {
		for (size_t p = content.find("```"); p != std::string::npos; p = content.find("```")) {
			content = content.substr(0, p) + (content.length() > p + 3 ? content.substr(p + 3) : "");
		}
	}

	content = _fix_content(content);


	// >> LINK
	if (std::regex_search(content, regex_link)) {
		//logg->info("Got link. Handling...");

		std::lock_guard<std::mutex> luck(mute);
		for (auto& i : link_overwrite_contains) {
			if (content.find(i.first) != std::string::npos) {
				chat_link = i.second;
				break;
			}
		}
		if (!chat_link) {
			chat_link = links;
		}
	}

	// >> FILE
	if (msg.attachments.size() > 0) {
		//logg->info("Got file. Handling...");

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

	// >> TEXT
	if (!chat_link && !chat_file) {
		//logg->info("Got not link nor file. Handling...");

		for (auto& i : nlf_overwrite_contains) {
			if (content.find(i.first) != std::string::npos) {
				chat_none = i.second;
				break;
			}
		}
		if (!chat_none) {
			chat_none = nonee;
		}
	}




	// HANDLE LINK
	if (chat_link > 1) { // has link, share link in links section (might have file)				<< LINKS
		//logg->info("Moving/Copying link...");

		aegis::channel* ch = get_channel_safe(chat_link, core);
		bool failure = true;
		
		if (ch) {
			slowflush_end thismsg;

			if (embed_fallback) {
				thismsg = slow_flush_embed(
					embed_link_gen(
						chat_link,
						msg.author.id,
						"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
						msg.author.username + "#" + msg.author.discriminator,
						content
					),
					*ch,
					this_guild,
					logg
				);

				failure = !thismsg;
				if (!link_ref_back) handle_poll(thismsg);
			}
			else {
				std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + content;
				to_send = to_send.substr(0, safe_msg_limit);

				thismsg = slow_flush(to_send, *ch, this_guild, logg);

				failure = !thismsg;
				if (!link_ref_back) handle_poll(thismsg);
			}

			if (link_ref_back && fallback && !failure && chat_file <= 1) { // chat_file <= 1 means it will be copied by file already
				auto ennnn = slow_flush_embed(
					embed_link_gen(
						link_ref_show_ref,
						chat_link,
						thismsg.get_id(),
						msg.author.id,
						"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
						msg.author.username + "#" + msg.author.discriminator,
						content.length() > str_max_len_embed ? content.substr(0, str_max_len_embed - 3) + "..." : content
					),
					*fallback,
					this_guild,
					logg
				);

				handle_poll(ennnn);
			}
		}
		if (failure && fallback) {
			slow_flush("Failed to move your message.", *fallback, this_guild, logg);
		}
	}

	// HANDLE FILE
	if (chat_file > 1) { // has file, share file in files section (might have link)				<< FILES
		//logg->info("Moving/Copying file...");

		aegis::channel* ch = get_channel_safe(embed_fallback ? embed_fallback : chat_file, core);
		aegis::channel* realch = embed_fallback ? get_channel_safe(chat_file, core) : ch;
		int failure = 1;

		if (ch && realch) {
			std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + (chat_link ? "" : (" " + content));
			to_send = to_send.substr(0, safe_msg_limit);

			failure = false;

			auto& i = msg.attachments.front();
			Downloader down;
			down.getASync(i.url.c_str());

			//logg->info("[!] Guild #{} is downloading {}...", this_guild, i.filename);

			while (!down.ended()) {
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}


			if (down.read().size() >= MAX_FILE_SIZE) {
				logg->info("[!] Guild #{} can't copy download {}. (size: {} byte(s))", this_guild, i.filename, down.read().size());
				failure = 2;
			}
			else {
				logg->info("[!] Guild #{} downloaded {}. (size: {} byte(s))", this_guild, i.filename, down.read().size());
				aegis::rest::aegis_file fp;
				fp.name = i.filename;
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				//std::cout << "CPY>";
				for (auto& k : down.read()) fp.data.push_back(k);
				//std::cout << "DONE>";

				if (embed_fallback) {								// EVERYTHING EMBED!
					//std::cout << "EMBFB>";

					slowflush_end source = slow_flush(fp, *ch, this_guild, logg); //send to embed fallback chat
					//std::cout << "S1N>";
					slowflush_end thismsg;

					thismsg = slow_flush_embed( // send to real file chat with embed
						embed_link_gen(
							chat_file,
							msg.author.id,
							"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
							msg.author.username + "#" + msg.author.discriminator,
							content,
							source.attachments.size() ? source.attachments[0].url : ""
						),
						*realch,
						this_guild,
						logg
					);

					//std::cout << "S2E>";

					failure = !thismsg;
					if (!files_ref_back) handle_poll(thismsg); // if not ref_back, poll right there.

					if (files_ref_back && fallback && !failure) {
						//std::cout << "BACK>";
						auto ennnn = slow_flush_embed( // ref_back
							embed_link_gen(
								files_ref_show_ref,
								chat_file,
								thismsg.get_id(),
								msg.author.id,
								"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
								msg.author.username + "#" + msg.author.discriminator,
								content.length() > str_max_len_embed ? content.substr(0, str_max_len_embed - 3) + "..." : content,
								source.attachments.size() ? source.attachments[0].url : ""
							),
							*fallback,
							this_guild,
							logg
						);
						//std::cout << "END.\n";

						handle_poll(ennnn);
					}

				}
				else {												// EVERYTHING NOT EXACTLY EMBED
					//std::cout << "NONFB>";
					if (slow_flush(to_send, *ch, this_guild, logg)) { // send to chat
						//std::cout << "S1N>";

						if (auto source = slow_flush(fp, *ch, this_guild, logg); source) { // send big file to chat
							//std::cout << "S2N>";
							failure = !source;
							if (!files_ref_back) handle_poll(source); // no ref? poll there

							if (files_ref_back && fallback && !failure) { // ref, ref.
								//std::cout << "BACK>";
								auto ennnn = slow_flush_embed(
									embed_link_gen(
										files_ref_show_ref,
										chat_file,
										source.get_id(),
										msg.author.id,
										"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
										msg.author.username + "#" + msg.author.discriminator,
										content.length() > str_max_len_embed ? content.substr(0, str_max_len_embed - 3) + "..." : content,
										source.attachments.size() ? source.attachments[0].url : ""
									),
									*fallback,
									this_guild,
									logg
								);
								//std::cout << "END.\n";

								handle_poll(ennnn);
							}
						}
					}
				}
			}
		}

		if (failure && fallback) {
			slow_flush("Failed to move your file." + std::string(failure == 2 ? " File too big." : ""), *fallback, this_guild, logg);
		}
	}

	// HANDLE NEITHER BUT TEXT
	if (chat_link == 0 && chat_file == 0 && chat_none > 1) { // not link and no file, DEFAULT chat_none				<< NONES || just_delete_source
		//logg->info("Moving/Copying text...");

		if (content.length()) {
			aegis::channel* ch = get_channel_safe(chat_none, core);
			bool failure = true;

			if (ch) {
				slowflush_end thismsg;

				if (embed_fallback) {									// EVERYTHING EMBED!
					thismsg = slow_flush_embed(
						embed_link_gen(
							chat_none,
							msg.author.id,
							"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
							msg.author.username + "#" + msg.author.discriminator,
							content
						),
						*ch,
						this_guild,
						logg
					);

					failure = !thismsg;
					if (!link_ref_back) handle_poll(thismsg);
				}
				else {													// NOT EVERYTHING EMBED
					std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + content;
					to_send = to_send.substr(0, safe_msg_limit);

					thismsg = slow_flush(to_send, *ch, this_guild, logg);

					failure = !thismsg;
					if (!link_ref_back) handle_poll(thismsg);
				}



				if (link_ref_back && fallback && !failure) {			// COPY TO CHAT AS EMBED?
					auto ennnn = slow_flush_embed(
						embed_link_gen(
							link_ref_show_ref,
							chat_none,
							thismsg.get_id(),
							msg.author.id,
							"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
							msg.author.username + "#" + msg.author.discriminator,
							content.length() > str_max_len_embed ? content.substr(0, str_max_len_embed - 3) + "..." : content
						),
						*fallback,
						this_guild,
						logg
					);

					handle_poll(ennnn);
				}
				/*
				std::string to_send = "`" + msg.author.username + "#" + msg.author.discriminator + ":` " + content;
				if (to_send.length() > safe_msg_limit) to_send = to_send.substr(0, safe_msg_limit);

				if (auto thismsg = slow_flush(to_send, *ch, this_guild, logg); thismsg) {
					failure = false;

					if (nlf_ref_back && fallback) {
						auto ennnn = slow_flush_embed(
							embed_link_gen(
								nlf_ref_show_ref,
								chat_none,
								thismsg.get_id(),
								msg.author.id,
								"https://cdn.discordapp.com/avatars/" + std::to_string(msg.author.id) + "/" + msg.author.avatar + ".png?size=256",
								msg.author.username + "#" + msg.author.discriminator,
								content.length() > str_max_len_embed ? content.substr(0, str_max_len_embed - 3) + "..." : content
							),
							*fallback,
							this_guild,
							logg
						);

						handle_poll(ennnn);
					}
				}*/
			}
			if (failure && fallback) {
				slow_flush("Failed to move your message.", *fallback, this_guild, logg);
			}
		}
	}


	// CLEANUP
	if (chat_link || chat_file || chat_none) {
		//logg->info("Cleaning up original message...");

		std::this_thread::yield();
		for (size_t tries = 0; tries < 7; tries++) {
			try {
				if (fallback) {
					auto mmm = fallback->delete_message(msg.get_id());
					while (!mmm.available()) {
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						std::this_thread::yield();
					}
					tries = 7;
					//logg->info("Cleaned up.");
					break;
				}
				else logg->error("[!] Guild #{} can't delete source message.", this_guild);
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

	//logg->info("Handled message.");
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

	std::function<void(void)> common_help = [&] {
		
		const std::string msg_pack[] = {
			"**Commands:**\n"
			"```md\n"
			"# Global:\n"
			"- debug - shows all config (in json)\n"
			"- debughere - shows all config (in json) in this chat\n"
			"- alias <command_alias> - add an alias, like ~\n"
			"- admintag add/remove <id> - allow or not a tag to run these commands\n```",

			"```md\n"
			"# Local:\n"
			"- showuserthumbnail - enables/disables user big thumbnail\n"
			"- dontusemd - enables/disables the `code block` inside embeds\n"
			"- autopoll - enables/disables auto thumbsup/down reaction\n"
			"- embedall <id> - enables embed to every message (make sure links permission is set; these commands won't be embed). <id> is the chat where it will send raw files. (* = clear)\n"
			"- delallconfig - removes ALL configuration in this chat\n"
			"- link <id> - set default link redirect (* = clear)\n"
			"- file <id> - set default file redirect (* = clear)\n```",

			"```md\n"
			"- text <id> - set default text redirect (if not file nor link; * = clear)\n"
			"- link redirback - after cut (if cut), create an embed-ref? (switch)\n"
			"- file redirback - after cut (if cut), create an embed-ref? (switch)\n"
			"- text redirback - after cut (if cut), create an embed-ref? (switch)\n"
			"- link redirbacklink - adds a link to the message (redirback) (switch)\n"
			"- file redirbacklink - adds a link to the message (redirback) (switch)\n"
			"- text redirbacklink - adds a link to the message (redirback) (switch)\n"
			"- specific link <contains> <id> - if link has <contains>, redirect to <id> (if <id> is 0, it will only delete the source)\n"
			"- specific file <contains> <id> - if file has <contains>, redirect to <id> (if <id> is 0, it will only delete the source)\n"
			"- specific text <contains> <id> - if text has <contains>, redirect to <id> (not file/link only) (if <id> is 0, it will only delete the source)\n"
			"- specific remove * * - removes ALL specific rules (yes, two * *, one means any entry, second means any in those entries)\n"
			"- specific remove link <contains> - remove a rule set earlier (pro tip: * clear everything)\n"
			"- specific remove file <contains> - remove a rule set earlier (pro tip: * clear everything)\n"
			"- specific remove text <contains> - remove a rule set earlier (pro tip: * clear everything)\n```"
		};

		for (auto& aaa : msg_pack) {
			slow_flush(aaa, buck, guildid, logg);
		}
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
		else if (yo == "debughere") {
			std::string uh;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				uh = friendo.export_config().dump(2);
			}
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
		else if (yo == "autopoll") {

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				friendo.autopoll = !friendo.autopoll;
				sending = std::string("Autopoll ") + (friendo.autopoll ? "ENABLED" : "DISABLED");
				save = true;
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
		else if (yo == "dontusemd") {

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				friendo.dont_use_md = !friendo.dont_use_md;
				sending = std::string("Automatic block of text is now ") + (!friendo.dont_use_md ? "ENABLED" : "DISABLED");
				save = true;
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
		else if (yo == "showuserthumbnail") {

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				friendo.show_user_thumbnail = !friendo.show_user_thumbnail;
				sending = std::string("User thumbnail in embed is now ") + (friendo.show_user_thumbnail ? "ENABLED" : "DISABLED");
				save = true;
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

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
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				data.command_alias = res;
			}
			slow_flush("Has set " + res + " as command alias successfully.", buck, guildid, logg);
			save = true;

			well_done = true;
		}
		else if (yo == "embedall") {
			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				if (res == "*") {
					friendo.embed_fallback = 0;
					sending = "Removed embed fallback successfully.";
					save = true;
				}
				else {
					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;

					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {
						friendo.embed_fallback = yaoo;
						sending = "Has set <#" + std::to_string(yaoo) + "> as embed fallback successfully.";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
				}
			}
			if (sending.length() > 0) slow_flush(sending, buck, guildid, logg);

			well_done = true;
		}
		else if (yo == "link") { // 																			<< lsw/bb link <id>

			std::string sending;
			{
				std::lock_guard<std::mutex> luck_me(data.mut);
				auto ch_id = buck.get_id();
				auto& friendo = data.chats[ch_id];
				std::lock_guard<std::mutex> luck(friendo.mute);

				if (res == "*") {
					friendo.links_orig = 0;
					sending = "Removed default link configuration for this chat.";
					save = true;
				}
				else if (res == "redirback") {
					friendo.link_ref_back = !friendo.link_ref_back;
					sending = std::string("Ref back set to ") + (friendo.link_ref_back ? "TRUE" : "FALSE");
					save = true;
				}
				else if (res == "redirbacklink") {
					friendo.link_ref_show_ref = !friendo.link_ref_show_ref;
					sending = std::string("Link on ref back set to ") + (friendo.link_ref_show_ref ? "TRUE" : "FALSE");
					save = true;
				}
				else {
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


				if (res == "*") {
					friendo.nlf_orig = 0;
					sending = "Removed default text configuration for this chat.";
					save = true;
				}
				else if (res == "redirback") {
					friendo.nlf_ref_back = !friendo.nlf_ref_back;
					sending = std::string("Ref back set to ") + (friendo.nlf_ref_back ? "TRUE" : "FALSE");
					save = true;
				}
				else if (res == "redirbacklink") {
					friendo.nlf_ref_show_ref = !friendo.nlf_ref_show_ref;
					sending = std::string("Link on ref back set to ") + (friendo.nlf_ref_show_ref ? "TRUE" : "FALSE");
					save = true;
				}
				else {
					while (res.length() > 0) {
						if (!std::isdigit(res[0])) { // not a number
							res.erase(res.begin());
						}
						else break;
					}

					unsigned long long yaoo;

					if (sscanf_s(res.c_str(), "%llu", &yaoo) == 1) {
						friendo.nlf_orig = yaoo ? yaoo : 1;
						if (yaoo) sending = "Has set <#" + std::to_string(yaoo) + "> as global text fallback successfully.";
						else sending = "Has set to just delete as global text fallback successfully.";
						save = true;
					}
					else {
						sending = "Failed to get the ID from your command.";
					}
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


				if (res == "*") {
					friendo.files_orig = 0;
					sending = "Removed default files configuration for this chat.";
					save = true;
				}
				else if (res == "redirback") {
					friendo.files_ref_back = !friendo.files_ref_back;
					sending = std::string("Ref back set to ") + (friendo.files_ref_back ? "TRUE" : "FALSE");
					save = true;
				}
				else if (res == "redirbacklink") {
					friendo.files_ref_show_ref = !friendo.files_ref_show_ref;
					sending = std::string("Link on ref back set to ") + (friendo.files_ref_show_ref ? "TRUE" : "FALSE");
					save = true;
				}
				else {
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

						for (auto& h : friendo.nlf_overwrite_contains) {
							if (h.first == yo3) {
								h.second = yaoo ? yaoo : 1;
								already_got = true;
								break;
							}
						}

						if (!already_got) friendo.nlf_overwrite_contains.push_back({ yo3, yaoo ? yaoo : 1 });

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
						friendo.nlf_overwrite_contains.clear();

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
							friendo.nlf_overwrite_contains.clear();

							sending = "Cleaned up ALL TEXT rulesets of this chat.";
							save = true;
						}
						else { // contains

							bool found = false;

							for (size_t p = 0; p < friendo.nlf_overwrite_contains.size(); p++) {
								if (friendo.nlf_overwrite_contains[p].first == res) {
									friendo.nlf_overwrite_contains.erase(friendo.nlf_overwrite_contains.begin() + p);
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
				//std::cout << translate.second.dump(2) << std::endl;
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
	/*if (keep_run) {
		keep_run = false;
		if (hs_thr.joinable()) hs_thr.join();
		save_settings();
	}*/
	save_settings();
}

GuildHandle& GuildHandle::start()
{
	/*if (!keep_run) {
		load_settings();

		keep_run = true;
		hs_thr = std::thread([&] {while (keep_run) { if (hs.task()) { logg->info("Tasked once."); } else { std::this_thread::sleep_for(std::chrono::milliseconds(250)); std::this_thread::yield(); } }});
	}*/
	load_settings();
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
	try {
		size_t cc = 0;
		aegis::channel* src_p = nullptr;

		for (src_p = nullptr; !src_p; src_p = core->channel_create(src.get_id())) {
			if (++cc > 20) {
				logg->critical("FATAL ERROR HANDLING INPUT. CAN'T FIND CHANNEL AFTER 10 TRIES");
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				std::this_thread::yield();
			}
		}

		auto get_content = m.get_content();
		auto get_guild_id = m.get_guild_id();
		auto get_user_id = m.author.id;
		auto get_id = m.get_id();

		if (get_content.find(main_cmd) == 0 || ((data.command_alias.length() > 0) && get_content.find(data.command_alias) == 0)) {

			auto userr = core->user_create(get_user_id);
			auto guildd = core->guild_create(get_guild_id, &core->get_shard_by_guild(get_guild_id));

			if (!has_admin_rights(*guildd, *userr)) { // adm/owner/me
				slow_flush("You have no permission.", *src_p, guildid, logg);

				auto mmm = src_p->delete_message(get_id);
				while (!mmm.available()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

				return;
			}
			size_t start = get_content.find(main_cmd) == 0 ? main_cmd.length() : data.command_alias.size();

			std::vector<std::string> arguments;

			arguments.push_back(main_cmd);

			const std::string& slice = get_content;
			{
				std::string buf;
				for (size_t p = start; p < slice.length(); p++) {
					auto& i = slice[p];
					if (i != ' ') buf += slice[p];
					else if (!buf.empty()) arguments.push_back(std::move(buf));
				}
				if (!buf.empty()) arguments.push_back(std::move(buf));
			}
			command(arguments, *src_p);
		}
		else {
			data.chats[src_p->get_id()].handle_message(core, m);
		}
	}
	catch (...) {
		logg->critical("FATAL ERROR HANDLING INPUT.");
	}
}

void GuildHandle::handle(aegis::channel& ch, const unsigned long long mid, aegis::gateway::objects::emoji e)
{
	try {
		auto msgp = ch.get_message(mid);
		while (!msgp.available()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::this_thread::yield();
		}
		auto msg = msgp.get();

		//["footer"]["text"]

		if (msg.embeds.size() && [&] {for (auto& i : bot_ids) if (msg.author.id == i) return true; return false; }()) { // message generated from bot && has embed

			nlohmann::json j = msg.embeds[0];
			unsigned long long user_c = 0;

			if (j.count("footer") && !j["footer"].is_null()) {
				auto inn = j["footer"];
				if (inn.count("text") && !inn["text"].is_null()) {
					std::string scn = inn["text"].get<std::string>();
					if (!sscanf_s(scn.c_str(), "%llu", &user_c)) user_c = 0;
				}
			}
			
			std::string e_end = e.id ? ((e.animated ? "a:" : "") + e.name + ":" + std::to_string(e.id)) : (transformWeirdo(e.name));

			if (user_c == e.user && user_c) {
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
						e.id ? ((e.animated ? "a:" : "") +  e.name + ":" + std::to_string(e.id)) : (transformWeirdo(e.name))
					);
				}
			}
		}		
	}
	catch (...) {
		logg->critical("FATAL ERROR HANDLING INPUT.");
	}
}
