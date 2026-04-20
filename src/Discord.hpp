#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <string>
#include <vector>
#include <deque>
#include <atomic>
#include <sstream>
#include <pthread.h>
#include <vita2d.h>

#include <unordered_map>

#include "VitaNet.hpp"
#include "json.hpp"

#define PERMISSION_READ_MESSAGES 0x400 //1024


class Discord{
	public:
		typedef struct {
			std::string username;
			std::string discriminator;
			std::string id;
			std::string avatar;
			unsigned int color;
		}user;
		typedef struct{
			int codepoint;
			int posX;
			int posY;
			int spriteSheetX;
			int spriteSheetY;
		} message_emoji;
		typedef struct {
			
			bool isEmpty ;
			bool isImage ;
			bool loadedThumbImage ;
			bool isData ;
			
			
			std::string url;
			std::string proxy_url;
			std::string filename;
			int width;
			int height;
			std::string id;
			int size;
			int readableSize;
			std::string readableSizeUnit;
			
			vita2d_texture * thumbnail;
			 
		}message_attachment;
		typedef struct {
			user author;
			std::string content;
			//std::u32string contentUTF32;
			std::string embed;
			message_attachment attachment;
			std::string mentions;
			std::string timestamp;
			std::string id;
			std::vector<message_emoji> emojis;
			std::unordered_map<std::string, std::string> mentionsMap; 
		}message;
		typedef struct{
			int allow;
			std::string type;
			std::string id;
			int deny;
		} permission_overwrite;
		typedef struct {
			std::string name;
			std::string last_pin_timestamp;
			std::string topic;
			std::string last_message_id;
			int type;
			std::string id;
			bool is_private;
			bool readallowed = true;
			
			bool gotMessagesOnce = false;
			
			std::vector<permission_overwrite> permission_overwrites;
			
			std::deque<message> messages;
		}channel;
		typedef struct {
			bool owner;
				uint64_t permissions;
			std::string icon; 
			std::string id;
			std::string name;
			std::vector<channel> channels;
			std::vector<std::string> myroles;
		}guild;
		typedef struct {
			std::string last_message_id;
			long type;
			std::string id;
			std::vector<user> recipients;
			std::vector<message> messages;
		}directMessage;
				
		typedef struct {
			vita2d_texture * icon;
			int offset;
			int size;
			bool loaded = false;
		}emojiOld;
		typedef struct {
			int x;
			int y;
		}emoji;
		
		typedef struct {
			uint32_t code;
			int x;
			int y;
		} EmojiData;

		Discord();
		~Discord();
		void setToken(std::string tok);
		long login();
		long login(std::string tok);
		std::string getToken();
		std::string getTicket();
		std::string getUsername();
		std::string getEmail();
		std::string getPassword();
		void loadData();
		bool loadingData;
		bool currentlyRefreshingMessages;
		std::vector<guild> guilds;
		std::vector<directMessage> directMessages;
		int guildsAmount = 0;
		static void* loadData_wrapper(void* object)
		{
			reinterpret_cast<Discord*>(object)->thread_loadData(object);
			return 0;
		}
		static void* refreshMessages_wrapper(void* object)
		{
			reinterpret_cast<Discord*>(object)->thread_refreshMessages(object);
			return 0;
		}
		void JoinGuild(int gIndex);
		uint64_t osGetTimeMS();
		void JoinChannel(int cIndex);
		void LeaveChannel();
		void JoinDirectMessageChannel(int dIndex);
		void LeaveDirectMessageChannel();
		int currentGuild = 0;
		int currentChannel = 0;
		int currentDirectMessage = 0;
		bool sendMessage(std::string msg);
		bool sendDirectMessage(std::string msg);
		
		bool editMessage(std::string channelID , std::string messageID , std::string newContent);
		bool deleteMessage(std::string channelID , std::string messageID);
		
		bool refreshMessages();
		bool refreshDirectMessages();
		bool refreshCurrentDirectMessages();
		bool refreshingMessages;
		void utf16_to_utf8(uint16_t *src, uint8_t *dst);
		user client;
		std::string email = "", password = "", code2fa , token , ticket , username , id , avatar , discriminator , phone;
		bool refreshedMessages;
		bool inChannel;
		bool inDirectMessageChannel;
		bool loadingDirectMessages ;
	
		vita2d_texture * spritesheetEmoji;
		int emojiWidth = 32;
		int emojiHeight = 32;
		std::unordered_map <int, emoji> emojiMap; // first int is utf32 codepoint , second int is offset in file
		std::unordered_map<int, emoji>::iterator emojiMapIterator;
		int emojiCount;
		std::vector<int> emojiTestArray;

		std::vector<EmojiData> emojiVector;
		std::unordered_map<uint32_t, size_t> fastEmojiMap;
	private:
		VitaNet vitaNet;
		bool verified, mfa_enabled; // mfa == twofactor its the same
		bool twoFactorAuthEnabled;
		bool loggedin;
		bool loadedGuilds ;
		bool loadedChannels;
		bool loadedDMs;
		long fetchUserData();
		void getGuilds();
		void getChannels();
		void getDirectMessageChannels();
		void getChannelMessages(int channelIndex);
		void getCurrentDirectMessages();
		void *thread_loadData(void *arg);
		void *thread_refreshMessages(void *arg);
		std::string safeUtf8WordWrap(const std::string& rawContent, int maxCharsPerLine = 40);
		
		pthread_t loadMessagesThread;
		bool pthreadStarted;
		
		std::stringstream stringStream;
		
		uint64_t lastFetchTimeMS;
		uint64_t fetchTimeMS = 4000; // 4 seconds refreshing
		uint64_t currentTimeMS;
		bool forceRefreshMessages;

		nlohmann::json emojiJsonData;
};








#endif

