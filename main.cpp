#include "guild_handler.h"


int main() {

	std::vector<std::unique_ptr<GuildHandle>> guilds;
	std::shared_mutex shared;
	bool ignore_all_ending_lmao = false;

	std::shared_ptr<aegis::core> thebot = std::shared_ptr<aegis::core>(new aegis::core(spdlog::level::trace, 0), [](aegis::core* c) {
		c->shutdown();
		delete c;
	});

	std::shared_ptr<spdlog::logger> logg = thebot->log;
	logg->info("Bot has started.");

	thebot->set_on_guild_create_raw([&](const nlohmann::json& result, aegis::shards::shard* _shard){//[&](aegis::gateway::events::guild_create obj) {
		if (ignore_all_ending_lmao) return;

		unsigned long long guild_id = stdstoulla(result["d"]["id"].get<std::string>());
		std::string guild_name = result["d"]["name"];
		std::string guild_region = result["d"]["region"];

		logg->info("Joined/Connected Guild #{} ({}) from {}", guild_id, guild_name, guild_region);

		LockGuardShared guard(shared, lock_shared_mode::SHARED);

		for (auto& i : guilds) {
			if (*i == guild_id) return;
		}

		guard.lock();

		guilds.push_back(std::make_unique<GuildHandle>(thebot, guild_id));
	});

	thebot->set_on_guild_delete_raw([&](const nlohmann::json& result, aegis::shards::shard* _shard) {//[&](aegis::gateway::events::guild_delete obj) {
		if (ignore_all_ending_lmao) return;

		unsigned long long guild_id = stdstoulla(result["d"]["id"].get<std::string>());

		logg->info("Left Guild #{}", guild_id);

		LockGuardShared guard(shared, lock_shared_mode::EXCLUSIVE);
		for (size_t p = 0; p < guilds.size(); p++) {
			if (*guilds[p] == guild_id) {
				guilds.erase(guilds.begin() + p);
				return;
			}
		}
	});

	thebot->set_on_message_create_raw([&](const nlohmann::json& result, aegis::shards::shard* _shard) {//[&](aegis::gateway::events::message_create obj) {
		if (ignore_all_ending_lmao) return;

		LockGuardShared guard(shared, lock_shared_mode::SHARED);

		unsigned long long guild_id = 0;
		if (result["d"].count("guild_id") && !result["d"]["guild_id"].is_null()) guild_id = stdstoulla(result["d"]["guild_id"].get<std::string>());

		aegis::channel* ch = thebot->find_channel(result["d"]["channel_id"]);
		if (!ch) return;
		if (ch->get_guild_id() == 0) return; // dm
		if (result["d"].count("webhook_id")) return; // webhook event

		auto g = &ch->get_guild();
		{
			if (result["d"].count("member") && !result["d"]["member"].is_null())
			{
				aegis::gateway::objects::member u = result["d"]["member"];
				u._user = result["d"]["author"].get<aegis::gateway::objects::user>();
			}
		}

		//user was previously created via presence update, but presence update only contains id
		aegis::gateway::events::message_create obj{ *_shard, aegis::lib::nullopt, std::ref(*ch), {result["d"], thebot.get()} };


		for (auto& i : bot_ids) {
			if (obj.msg.author.id == i) return; // bot react
		}

		//obj.msg = result["d"];
		std::vector<_sticker_addon> stickers;
		if (result["d"].count("stickers") && !result["d"]["stickers"].is_null()) {
			for (const auto& st : result["d"]["stickers"]) {
				stickers.push_back(st);
			}
		}

		for (auto& i : guilds) {
			if (*i == obj.channel.get_guild_id()) {

				if (stickers.size()) for(auto& s : stickers) i->handle(obj.channel, obj.msg, s); // nowadays it is sticker OR message/attachment.
				else i->handle(obj.channel, obj.msg);

				return;
			}
		}

		guard.lock();

		logg->info("Joined/Connected Guild #{} (Weird way)", obj.channel.get_guild_id());
		guilds.push_back(std::make_unique<GuildHandle>(thebot, obj.channel.get_guild_id()));
		guilds.back()->handle(obj.channel, obj.msg);
	});

	thebot->set_on_message_reaction_add_raw([&](const nlohmann::json& result, aegis::shards::shard* _shard) {
		if (ignore_all_ending_lmao) return;

		LockGuardShared guard(shared, lock_shared_mode::SHARED);

		unsigned long long guild_id = 0;
		unsigned long long channel_id = stdstoulla(result["d"]["channel_id"]);
		unsigned long long message_id = stdstoulla(result["d"]["message_id"]);
		aegis::gateway::objects::emoji emoji = result["d"]["emoji"];
		if (result["d"].count("guild_id") && !result["d"]["guild_id"].is_null())
			guild_id = stdstoulla(result["d"]["guild_id"]);

		emoji.user = stdstoulla(result["d"]["user_id"]);

		for (auto& i : bot_ids) {
			if (emoji.user == i) return; // bot react
		}


		if (!guild_id) return; // not a guild

		aegis::channel* ch = thebot->find_channel(channel_id);
		if (!ch) {
			thebot->log->error("Failed to find channel to handle reaction event on guild {}", guild_id);
			return;
		}

		for (auto& i : guilds) {
			if (*i == guild_id) {
				i->handle(*ch, message_id, emoji);
				return;
			}
		}

		guard.lock();

		logg->info("Joined/Connected Guild #{} (Weird way)", guild_id);
		guilds.push_back(std::make_unique<GuildHandle>(thebot, guild_id));
		guilds.back()->handle(*ch, message_id, emoji);
	});


	thebot->run();
	std::thread here_lol = std::thread([&] {

		std::this_thread::sleep_for(std::chrono::seconds(10));

		auto keep = [&] {return (!ignore_all_ending_lmao); };

		while (keep()) {

			try {
				thebot->update_presence(default_command_start + " - " + version, aegis::gateway::objects::activity::Game, aegis::gateway::objects::presence::Idle);
			}
			catch (...) {
				std::cout << "\nFAILED ONCE AT PRESENCE THREAD!" << std::endl;
			}

			for (size_t c = 0; c < 50 && keep(); c++) {
				std::this_thread::yield();
				std::this_thread::sleep_for(std::chrono::seconds(6));
			}
		}

		});

	std::string smth;
	/*while (smth.find("exit") != 0) {
		std::getline(std::cin, smth);

		if (smth.find("list") == 0) {
			std::string combo;
			logg->info("Processing list...");
			{
				LockGuardShared guard(shared, lock_shared_mode::EXCLUSIVE);
				std::lock_guard<std::shared_mutex> luck2(thebot->get_guild_mutex());
				auto& cpyy = thebot->get_guild_map();
				for (auto& i : guilds) {
					for (auto& j : cpyy) {
						if (*i == j.first) {
							combo += "- \"" + std::to_string(j.first) + "\"\n";
							//combo += "- \"" + j.second->get_name() + "\" from \"" + j.second->get_region() + "\"\n";
						}
					}
				}
			}
			logg->info("List: \n{}", combo);
		}
	}*/

	thebot->yield();

	logg->warn("Bot is shutting down...");

	ignore_all_ending_lmao = true;

	thebot->shutdown();

	// kill all threads
	{
		std::cout << "Locking all data..." << std::endl;
		LockGuardShared guard(shared, lock_shared_mode::EXCLUSIVE);
		guilds.clear();
	}


	if (here_lol.joinable()) here_lol.join();
	thebot.reset();

	std::cout << "Ended bot entirely." << std::endl;

	return 0;
}