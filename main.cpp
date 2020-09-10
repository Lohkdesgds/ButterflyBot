#include "shared.h"

void nolock(std::function<void(void)> f, std::string wher = "") {
	std::string err_line = "[FATAL-ERROR] NoLock failed" + (wher.length() > 0 ? (" at " + wher) : "") + ":";
	try {
		f();
	}
	catch (aegis::error e) {
		std::cout << err_line << e << std::endl;
	}
	catch (std::exception e) {
		std::cout << err_line << e.what() << std::endl;
	}
	catch (...) {
		std::cout << err_line << std::endl;
	}
}

int main()
{
	std::vector<std::shared_ptr<GuildHandle>> guilds;
	std::mutex guilds_m;
	bool ignore_all_ending_lmao = false;

	std::shared_ptr<aegis::core> thebot = std::shared_ptr<aegis::core>(new aegis::core(aegis::create_bot_t().log_level(spdlog::level::trace).token(token)), [](aegis::core* c) {
		c->shutdown();
		delete c;
		});

	std::shared_ptr<spdlog::logger> logg = thebot->log;
	logg->info("[!] Bot has started.");


	// join in guild
	thebot->set_on_guild_create([&](aegis::gateway::events::guild_create obj) {
		if (ignore_all_ending_lmao) return;
		nolock([&] {

			logg->info("[!] Joined/Connected Guild #{} ({}) from {}", obj.guild.id, obj.guild.name, obj.guild.region);

			guilds_m.lock();
			guilds.emplace_back(std::make_shared<GuildHandle>(thebot, obj.guild.id));
			auto cpy = guilds.back();
			guilds_m.unlock();
			cpy->start();

			}, "set_on_guild_create");
		});
	// leave a guild
	thebot->set_on_guild_delete([&](aegis::gateway::events::guild_delete obj) {
		if (ignore_all_ending_lmao) return;
		nolock([&] {
			logg->info("[!] Left Guild #{}", obj.guild_id);

			std::lock_guard<std::mutex> luck(guilds_m);
			for (size_t p = 0; p < guilds.size(); p++) {
				if (guilds[p]->amI(obj.guild_id)) {
					guilds.erase(guilds.begin() + p);
					return;
				}
			}
			}, "set_on_guild_delete");
		});


	thebot->set_on_message_create([&](aegis::gateway::events::message_create obj) {
		if (ignore_all_ending_lmao) return;
		nolock([&] {

			if (obj.msg.author.is_bot()) return;

			guilds_m.lock();
			for (auto& i : guilds) {
				if (i->amI(obj.msg.get_guild_id())) {
					auto cpy = i;
					guilds_m.unlock();
					cpy->handle(obj.channel, obj.msg);
					return;
				}
			}

			logg->info("[!] Got new message from unknown source?! Creating Guild #{}...", obj.msg.get_guild_id());

			guilds.emplace_back(std::make_shared<GuildHandle>(thebot, obj.msg.get_guild_id()));

			auto cpy = guilds.back();
			guilds_m.unlock();

			cpy->handle(obj.channel, obj.msg); // handle

			}, "set_on_message_create");
		});
	thebot->set_on_message_update([&](aegis::gateway::events::message_update obj) {
		if (ignore_all_ending_lmao) return;
		nolock([&] {
			if (obj.msg.get_content().empty()) return;
			if (obj.msg.author.is_bot()) return;

			guilds_m.lock();
			for (auto& i : guilds) {
				if (i->amI(obj.msg.get_guild_id())) {
					auto cpy = i;
					guilds_m.unlock();
					cpy->handle(obj.channel, obj.msg);
					return;
				}
			}

			logg->info("[!] Got new message from unknown source?! Creating Guild #{}...", obj.msg.get_guild_id());

			guilds.emplace_back(std::make_unique<GuildHandle>(thebot, obj.msg.get_guild_id()));

			auto cpy = guilds.back();
			guilds_m.unlock();

			cpy->handle(obj.channel, obj.msg); // handle

			}, "set_on_message_update");
		});
	thebot->set_on_message_reaction_add([&](aegis::gateway::events::message_reaction_add obj) {
		if (ignore_all_ending_lmao) return;
		nolock([&] {
			obj.emoji.user = obj.user_id; // come on

			if ([&] {for (auto& i : bot_ids) if (obj.user_id == i) return true; return false; }()) return;

			guilds_m.lock();
			for (auto& i : guilds) {
				if (i->amI(obj.guild_id)) {
					auto cpy = i;
					guilds_m.unlock();
					aegis::channel* ch = nullptr;
					for (size_t p = 0; p < 5 && !ch; p++) {
						ch = thebot->channel_create(obj.channel_id);
					}
					if (ch) cpy->handle(*ch, obj.message_id, obj.emoji);
					else logg->error("[!] Guild #{} failed to create channel to handle emoji.", obj.guild_id);

					return;
				}
			}

			logg->info("[!] Got new message from unknown source?! Creating Guild #{}...", obj.guild_id);

			guilds.emplace_back(std::make_shared<GuildHandle>(thebot, obj.guild_id));

			auto cpy = guilds.back();
			guilds_m.unlock();

			aegis::channel* ch = nullptr;
			for (size_t p = 0; p < 5 && !ch; p++) {
				ch = thebot->channel_create(obj.channel_id);
			}
			if (ch) cpy->handle(*ch, obj.message_id, obj.emoji);

			}, "set_on_message_create");
		});


	thebot->run();

	std::thread here_lol = std::thread([&] {

		std::this_thread::sleep_for(std::chrono::seconds(5));

		auto keep = [&] {return (!ignore_all_ending_lmao); };

		while (keep()) {

			nolock([&] {
				std::string msg = main_cmd + " - " + version + " - " + std::to_string(thebot->get_guild_count()) + " guild(s)!";
				thebot->update_presence(msg, aegis::gateway::objects::activity::Game, aegis::gateway::objects::presence::Idle);
			});

			for (size_t c = 0; c < 50 && keep(); c++) {
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::seconds(6));
			}
		}

		});

	std::string smth;
	while (smth != "exit") std::cin >> smth;
	logg->info("[!] Bot is shutting down...");

	ignore_all_ending_lmao = true;

	thebot->shutdown();

	// kill all threads
	{
		std::lock_guard<std::mutex> luck(guilds_m);
		for (auto& i : guilds) i->end();
		guilds.clear();
	}


	if (here_lol.joinable()) here_lol.join();
	thebot.reset();

	std::cout << "[!] Ended bot entirely." << std::endl;

	return 0;
}