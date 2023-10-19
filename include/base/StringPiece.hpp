#pragma once

#include <string>

namespace cm {
	class StringArg {
	public:
		StringArg(const char *str) : str_(str) {}

		StringArg(const std::string &str) : str_(str.c_str()) {}

		const char *c_str() const { return str_; }

	private:
		const char *str_;
	};
}