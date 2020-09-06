#pragma once

#include <aegis.hpp>

#include "printer.h"

bool slow_flush(aegis::create_message_t, aegis::channel&, unsigned long long);
bool slow_flush(aegis::rest::aegis_file, aegis::channel&, unsigned long long);
bool slow_flush(std::string, aegis::channel&, unsigned long long);

bool slow_flush_embed(nlohmann::json, aegis::channel&, unsigned long long);
bool slow_flush_embed(aegis::gateway::objects::embed, aegis::channel&, unsigned long long);