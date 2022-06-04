#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void __stop_program__();

#define trace(...)		spdlog::trace(__VA_ARGS__)
#define info(...)		spdlog::info(__VA_ARGS__)
#define warn(...)		spdlog::warn(__VA_ARGS__)
#define error(...)		spdlog::error(__VA_ARGS__)
#define fatal(...)		{ spdlog::critical(__VA_ARGS__); __stop_program__(); } 


	