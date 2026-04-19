#ifndef DISCORD_HELPER_HPP
#define DISCORD_HELPER_HPP

#include <vector>
#include <string>
#include <stdint.h>

struct messageauthor{
	std::string username;
	std::string discriminator;
	std::string id;
	std::string avatar;

};

#include <unordered_map>

struct message{
	std::string timestamp;
	std::string id;
	struct messageauthor author;
	std::string content;
	std::unordered_map<std::string, std::string> mentionsMap;
};

struct channel{
	std::string name;

	std::string last_pin_timestamp;
	std::string topic;
	std::string last_message_id;
	int type;
	std::string id;
	bool is_private;

	std::vector<message> messages;
};


struct guild{
	bool owner;
	uint64_t permissions;
	std::string icon;
	std::string id;
	std::string name;
	std::vector<channel> channels;
};

struct user {
	std::string id;
	std::string name;
	std::string icon;

};
#endif
>>>>>>> 71117d0c85f13ce0d1d5d133f9923d267853656b
