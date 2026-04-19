#include "Discord.hpp"
#include "log.hpp"

#include <bitset>
#include <pthread.h>
#include <locale>
#include <string>
#include <iostream>
#include <map>

std::string replaceEmojiShortcodes(std::string str) {
	static const std::map<std::string, std::string> emojis = {
		{":smile:", "😄"},
		{":smiley:", "😃"},
		{":grinning:", "😀"},
		{":blush:", "😊"},
		{":relaxed:", "☺️"},
		{":wink:", "😉"},
		{":heart_eyes:", "😍"},
		{":kissing_heart:", "😘"},
		{":kissing_closed_eyes:", "😚"},
		{":kissing:", "😗"},
		{":kissing_smiling_eyes:", "😙"},
		{":stuck_out_tongue_winking_eye:", "😜"},
		{":stuck_out_tongue_closed_eyes:", "😝"},
		{":stuck_out_tongue:", "😛"},
		{":flushed:", "😳"},
		{":grin:", "😁"},
		{":pensive:", "😔"},
		{":relieved:", "😌"},
		{":unamused:", "😒"},
		{":disappointed:", "😞"},
		{":persevere:", "😣"},
		{":cry:", "😢"},
		{":joy:", "😂"},
		{":sob:", "😭"},
		{":sleepy:", "😪"},
		{":disappointed_relieved:", "😥"},
		{":cold_sweat:", "😰"},
		{":sweat_smile:", "😅"},
		{":sweat:", "😓"},
		{":weary:", "😩"},
		{":tired_face:", "😫"},
		{":fearful:", "😨"},
		{":scream:", "😱"},
		{":angry:", "😠"},
		{":rage:", "😡"},
		{":triumph:", "😤"},
		{":confounded:", "😖"},
		{":laughing:", "😆"},
		{":yum:", "😋"},
		{":mask:", "😷"},
		{":sunglasses:", "😎"},
		{":sleeping:", "😴"},
		{":dizzy_face:", "😵"},
		{":astonished:", "😲"},
		{":worried:", "😟"},
		{":frowning:", "😦"},
		{":anguished:", "😧"},
		{":imp:", "👿"},
		{":smiling_imp:", "😈"},
		{":open_mouth:", "😮"},
		{":grimacing:", "😬"},
		{":neutral_face:", "😐"},
		{":confused:", "😕"},
		{":hushed:", "😯"},
		{":smirk:", "😏"},
		{":expressionless:", "😑"},
		{":man_with_gua_pi_mao:", "👲"},
		{":man_with_turban:", "👳"},
		{":cop:", "👮"},
		{":construction_worker:", "👷"},
		{":guardsman:", "💂"},
		{":baby:", "👶"},
		{":boy:", "👦"},
		{":girl:", "👧"},
		{":man:", "👨"},
		{":woman:", "👩"},
		{":older_man:", "👴"},
		{":older_woman:", "👵"},
		{":person_with_blond_hair:", "👱"},
		{":angel:", "👼"},
		{":princess:", "👸"},
		{":smiley_cat:", "😺"},
		{":smile_cat:", "😸"},
		{":heart_eyes_cat:", "😻"},
		{":kissing_cat:", "😽"},
		{":smirk_cat:", "😼"},
		{":scream_cat:", "🙀"},
		{":crying_cat_face:", "😿"},
		{":joy_cat:", "😹"},
		{":pouting_cat:", "😾"},
		{":japanese_ogre:", "👹"},
		{":japanese_goblin:", "👺"},
		{":see_no_evil:", "🙈"},
		{":hear_no_evil:", "🙉"},
		{":speak_no_evil:", "🙊"},
		{":skull:", "💀"},
		{":alien:", "👽"},
		{":poop:", "💩"},
		{":fire:", "🔥"},
		{":sparkles:", "✨"},
		{":star2:", "🌟"},
		{":dizzy:", "💫"},
		{":boom:", "💥"},
		{":anger:", "💢"},
		{":sweat_drops:", "💦"},
		{":droplet:", "💧"},
		{":zzz:", "💤"},
		{":dash:", "💨"},
		{":ear:", "👂"},
		{":eyes:", "👀"},
		{":nose:", "👃"},
		{":tongue:", "👅"},
		{":lips:", "👄"},
		{":thumbsup:", "👍"},
		{":-1:", "👎"},
		{":thumbsdown:", "👎"},
		{":ok_hand:", "👌"},
		{":punch:", "👊"},
		{":facepunch:", "👊"},
		{":fist:", "✊"},
		{":v:", "✌️"},
		{":wave:", "👋"},
		{":hand:", "✋"},
		{":raised_hand:", "✋"},
		{":open_hands:", "👐"},
		{":point_up:", "☝️"},
		{":point_down:", "👇"},
		{":point_left:", "👈"},
		{":point_right:", "👉"},
		{":raised_hands:", "🙌"},
		{":pray:", "🙏"},
		{":point_up_2:", "👆"},
		{":clap:", "👏"},
		{":muscle:", "💪"},
		{":walking:", "🚶"},
		{":runner:", "🏃"},
		{":running:", "🏃"},
		{":dancer:", "💃"},
		{":couple:", "👫"},
		{":family:", "👪"},
		{":two_men_holding_hands:", "👬"},
		{":two_women_holding_hands:", "👭"},
		{":couplekiss:", "💏"},
		{":couple_with_heart:", "💑"},
		{":dancers:", "👯"},
		{":ok_woman:", "🙆"},
		{":no_good:", "🙅"},
		{":information_desk_person:", "💁"},
		{":raising_hand:", "🙋"},
		{":massage:", "💆"},
		{":haircut:", "💇"},
		{":nail_care:", "💅"},
		{":bride_with_veil:", "👰"},
		{":person_with_pouting_face:", "🙎"},
		{":person_frowning:", "🙍"},
		{":bow:", "🙇"},
		{":tophat:", "🎩"},
		{":crown:", "👑"},
		{":womans_hat:", "👒"},
		{":athletic_shoe:", "👟"},
		{":mans_shoe:", "👞"},
		{":shoe:", "👞"},
		{":sandal:", "👡"},
		{":high_heel:", "👠"},
		{":boot:", "👢"},
		{":shirt:", "👕"},
		{":tshirt:", "👕"},
		{":necktie:", "👔"},
		{":womans_clothes:", "👚"},
		{":dress:", "👗"},
		{":running_shirt_with_sash:", "🎽"},
		{":jeans:", "👖"},
		{":kimono:", "👘"},
		{":bikini:", "👙"},
		{":briefcase:", "💼"},
		{":handbag:", "👜"},
		{":pouch:", "👝"},
		{":purse:", "👛"},
		{":eyeglasses:", "👓"},
		{":ribbon:", "🎀"},
		{":closed_umbrella:", "🌂"},
		{":lipstick:", "💄"},
		{":yellow_heart:", "💛"},
		{":blue_heart:", "💙"},
		{":purple_heart:", "💜"},
		{":green_heart:", "💚"},
		{":heart:", "❤️"},
		{":broken_heart:", "💔"},
		{":heartpulse:", "💗"},
		{":heartbeat:", "💓"},
		{":two_hearts:", "💕"},
		{":sparkling_heart:", "💖"},
		{":revolving_hearts:", "💞"},
		{":cupid:", "💘"},
		{":love_letter:", "💌"},
		{":kiss:", "💋"},
		{":ring:", "💍"},
		{":gem:", "💎"},
		{":bust_in_silhouette:", "👤"},
		{":busts_in_silhouette:", "👥"},
		{":speech_balloon:", "💬"},
		{":footprints:", "👣"},
		{":thought_balloon:", "💭"}
	};

	std::string result = str;
	for (const auto& pair : emojis) {
		size_t pos = 0;
		while ((pos = result.find(pair.first, pos)) != std::string::npos) {
			result.replace(pos, pair.first.length(), pair.second);
			pos += pair.second.length();
		}
	}
	return result;
}
#include <iconv.h>
#include <wchar.h>
#include <stdlib.h>
#include <algorithm>   // for reverse
#include <debugnet.h>
#include <psp2/io/fcntl.h>

#include <cctype>
#include "json.hpp"
#include "easyencryptor.hpp"
#include "utf8.h"

#include <psp2/kernel/processmgr.h>

#include "key.h"

uint64_t Discord::osGetTimeMS(){
	return (sceKernelGetProcessTimeWide() / 1000);
}

Discord::Discord(){

	int fh = sceIoOpen("app0:assets/emoji/emojispritesheettable.json" , SCE_O_RDONLY , 0777);
	if(fh < 0){
		debugNetPrintf(DEBUG , "ERROR : Could not load emojispritesheettable.json !\n" );
	}else {
		debugNetPrintf(DEBUG , "Loaded emojispritesheettable.json !\n" );

		int fileSize = sceIoLseek ( fh, 0, SCE_SEEK_END );
		sceIoLseek ( fh, 0, SCE_SEEK_SET ); // reset 'cursor' in file


		std::string jsonString(fileSize , '\0');
		sceIoRead(fh , &jsonString[0] , fileSize );


		emojiJsonData = nlohmann::json::parse(jsonString);

		if( !emojiJsonData.is_null() ){

			if( !emojiJsonData["emoji"].is_null() ){

				int emojiCount = emojiJsonData["emoji"].size(); // amount of emojis in json

				if( !emojiJsonData["amount"].is_null() ){
					int amount2 = emojiJsonData["amount"].get<int>();
					if(amount2 == emojiCount){
						debugNetPrintf(DEBUG, " Got right amount of emojis in JSON!\n");
						std::string out = "Counted emojis : " + std::to_string( emojiCount) + "  , json amount var : " + std::to_string(amount2) + "\n";
						debugNetPrintf(DEBUG, out.c_str());
					}else{
						debugNetPrintf(DEBUG, " amount of emojis is different in json!\n");
						std::string out = "Counted emojis : " + std::to_string( emojiCount) + "  , json amount var : " + std::to_string(amount2) + "\n";
						debugNetPrintf(DEBUG, out.c_str());
					}

				}

				emojiTestArray.clear();
				int loadedIconsChecked = 0;

				debugNetPrintf(DEBUG, "> Loading big spritesheet !\n");
				spritesheetEmoji = vita2d_load_PNG_file("app0:assets/emoji/emojispritesheet.png");
				debugNetPrintf(DEBUG, ">>> Loaded big spritesheet !!!\n");


				debugNetPrintf(DEBUG, " Get dimensions of emoji!\n");
				if( !emojiJsonData["spritewidth"].is_null() ){
					emojiWidth = emojiJsonData["spritewidth"].get<int>();
				}
				if( !emojiJsonData["spriteheight"].is_null() ){
					emojiHeight = emojiJsonData["spriteheight"].get<int>();
				}
				
				int code = 0;

				debugNetPrintf(DEBUG, " Loop to add emoji coords to map!\n");
				for(int e = 0; e < emojiCount ; e++){
					if( !emojiJsonData["emoji"][e].is_null() ){
						if( !emojiJsonData["emoji"][e]["utf32code"].is_null() ){
							if( !emojiJsonData["emoji"][e]["utf32code"][0].is_null() ){
								if(emojiJsonData["emoji"][e]["utf32code"].size() == 1){
									if( !emojiJsonData["emoji"][e]["x"].is_null() ){
									if( !emojiJsonData["emoji"][e]["y"].is_null() ){
										debugNetPrintf(DEBUG, " Declare new emoji!\n");
										emoji addEmoji;
										debugNetPrintf(DEBUG, " assign emoji x\n");
										addEmoji.x = emojiJsonData["emoji"][e]["x"].get<int>();
										debugNetPrintf(DEBUG, " assign emoji y\n");
										addEmoji.y = emojiJsonData["emoji"][e]["y"].get<int>();
										debugNetPrintf(DEBUG, " assign code \n");
										code = emojiJsonData["emoji"][e]["utf32code"][0].get<int>();
										debugNetPrintf(DEBUG, " assign map key code's value to emoji\n");
										emojiMap[code] = addEmoji;
										debugNetPrintf(DEBUG, " push back code in testarray\n");
										emojiTestArray.push_back(code);

											EmojiData eData = { (uint32_t)code, addEmoji.x, addEmoji.y };
											emojiVector.push_back(eData);
											fastEmojiMap[(uint32_t)code] = emojiVector.size() - 1;

										debugNetPrintf(DEBUG, " inc loadedIcons\n");
										loadedIconsChecked++;
									}
								}
								}
							}
						}
					}
				}

				debugNetPrintf(DEBUG , "Total loaded icons : %d \n\n " , loadedIconsChecked);

			}


		}


	}
	emojiCount = emojiMap.size();
	std::string mapSizeStr = "unordered_map of emoji contains : " + std::to_string(emojiCount) + " elements !\n";
	debugNetPrintf(DEBUG, mapSizeStr.c_str() );


	loadedGuilds = false;
	loadingData = false;
}

Discord::~Discord(){
		//?
}
bool Discord::sendDirectMessage(std::string msg){
	debugNetPrintf(DEBUG , "Sending DM\n" );
	std::string processedMsg = replaceEmojiShortcodes(msg);
	std::string postData = "{ \"content\":\"" + processedMsg + "\" }";
	std::string sendDMMessageUrl = "https://discord.com/api/v9/channels/"
							+ directMessages[currentDirectMessage].id + "/messages" ;
	VitaNet::http_response senddmmessageresponse = vitaNet.curlDiscordPost(sendDMMessageUrl , postData , token);
	if(senddmmessageresponse.httpcode == 200){
		debugNetPrintf(DEBUG , "DM SENT!\n" );
		return true;
	}

	return false;
}

bool Discord::sendMessage(std::string msg){
	debugNetPrintf(DEBUG , "Sending message\n" );
	std::string processedMsg = replaceEmojiShortcodes(msg);
	std::string postData = "{ \"content\":\"" + processedMsg + "\" }";
	std::string sendMessageUrl = "https://discord.com/api/v9/channels/" + guilds[currentGuild].channels[currentChannel].id + "/messages" ;
	VitaNet::http_response sendmessageresponse = vitaNet.curlDiscordPost(sendMessageUrl , postData , token);
	if(sendmessageresponse.httpcode == 200){
		debugNetPrintf(DEBUG , "Message SENT!\n" );
		return true;
	}
	return false;
}


bool Discord::editMessage(std::string channelID , std::string messageID , std::string newContent){
	std::string editMessageUrl = "https://discord.com/api/v9/channels/" + channelID + "/messages/" + messageID;
	std::string patchData = "{ \"content\":\"" + newContent + "\" }";
	VitaNet::http_response editmessageresponse = vitaNet.curlDiscordPatch(editMessageUrl , patchData , token);

	if(editmessageresponse.httpcode == 200){


		for(unsigned int i = 0 ; i < guilds[currentGuild].channels[currentChannel].messages.size();i++){
			if(guilds[currentGuild].channels[currentChannel].messages[i].id == messageID){

				guilds[currentGuild].channels[currentChannel].messages[i].content = newContent;
				refreshedMessages = true;// MAYBE MAKE ANOTHER VARIABLE
				i= 99999;
				break;
			}

		}




		return true;
	}
	return false;
}

bool Discord::deleteMessage(std::string channelID , std::string messageID){
	std::string deleteMessageUrl = "https://discord.com/api/v9/channels/" + channelID + "/messages/" + messageID;
	VitaNet::http_response deletemessageresponse = vitaNet.curlDiscordDelete(deleteMessageUrl , token);
	if(deletemessageresponse.httpcode == 204){

		/* this code probably is cause of gpu crash , because vitagui tries to read the last message somewhere or so which does not exist .. maybe.
		// delete from deque

		for(unsigned int i = 0 ; i < guilds[currentGuild].channels[currentChannel].messages.size();i++){
			if(guilds[currentGuild].channels[currentChannel].messages[i].id == messageID){

				if(guilds[currentGuild].channels[currentChannel].messages[i].attachment.loadedThumbImage && guilds[currentGuild].channels[currentChannel].messages[i].attachment.thumbnail != NULL){
					vita2d_free_texture(guilds[currentGuild].channels[currentChannel].messages[i].attachment.thumbnail);
				}
				guilds[currentGuild].channels[currentChannel].messages.erase(guilds[currentGuild].channels[currentChannel].messages.begin() + i);
				refreshedMessages = true;// MAYBE MAKE ANOTHER VARIABLE
				i = 99999;
				break;
			}

		}
		refreshedMessages = true;
		*/
		//guilds[currentGuild].channels[currentChannel].messages.clear();

		return true;


	}
	return false;
}


bool Discord::refreshMessages(){

	//debugNetPrintf(DEBUG , "checking time to refresh messages\n" );
	currentTimeMS = osGetTimeMS();
	if(( currentTimeMS - lastFetchTimeMS > fetchTimeMS || forceRefreshMessages ) && !currentlyRefreshingMessages){
		//debugNetPrintf(DEBUG , "get new messages\n" );

		refreshingMessages = true;
		currentlyRefreshingMessages = true;
		getChannelMessages(currentChannel);
		currentlyRefreshingMessages = false;
		lastFetchTimeMS = osGetTimeMS();
		refreshedMessages = true;
		refreshingMessages = false;
		forceRefreshMessages = false;

	}
	return true;

}

void Discord::utf16_to_utf8(uint16_t *src, uint8_t *dst) {
	int i;
	for (i = 0; src[i]; i++) {
		if ((src[i] & 0xFF80) == 0) {
			*(dst++) = src[i] & 0xFF;
		} else if((src[i] & 0xF800) == 0) {
			*(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		} else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
			*(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
			*(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
			*(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
			*(dst++) = (src[i + 1] & 0x3F) | 0x80;
			i += 1;
		} else {
			*(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
			*(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		}
	}

	*dst = '\0';
}

/*static int strConv(const string &src, wstring &dst)
{
    iconv_t cd = iconv_open("UCS-4-INTERNAL", "UTF-8");
    if (cd == (iconv_t)-1)
        return -1;

    size_t src_length = strlen(src.c_str());
    int wlen = (int)src_length/3;
    size_t buf_length = src_length + wlen;

    char src_buf[src_length];
    strcpy(src_buf, src.c_str());
    char *buf = new char [buf_length];
    char *inptr = src_buf;
    char *outptr = buf;
    if (iconv(cd, &inptr, &src_length, &outptr, &buf_length) == -1)
    {
        if (buf!=NULL)
            delete [] buf;
        return -1;
    }
    iconv_close(cd);

    dst = wstring(reinterpret_cast<wchar_t*>(buf));
    dst = dst.substr(0, wlen);

    if (buf!=NULL)
        delete [] buf;

    return wlen;
}*/

void Discord::parseMessageContentEmoji(message *m , std::string str){


	int currentLineY = 0;
	m->emojis.clear();
	std::vector<unsigned char> utf8result;
	std::vector<unsigned int> utf32str;
	utf8::utf8to32(str.begin(), str.end(), back_inserter(utf32str));
	for(unsigned int x = 0 ; x < utf32str.size() ; x++){
		emojiMapIterator = emojiMap.find( static_cast<int>(utf32str[x]) );
		if(utf32str[x] > 0xFF){
			// ignoring all <0xFF (ascii) for now until supporting emojis consisting of two emoji ( modifier )
			
			if(emojiMapIterator != emojiMap.end()){
				
				message_emoji mEmoji;
				
				mEmoji.codepoint = static_cast<int>(utf32str[x]);
				mEmoji.spriteSheetX = emojiMapIterator->second.x;
				mEmoji.spriteSheetY = emojiMapIterator->second.y;
				mEmoji.posX = x % 40;  // HARDCODED max char per line = 30 :>
				mEmoji.posY = currentLineY;
				
				m->emojis.push_back(mEmoji);
				
				
				
				utf32str[x] = 0x20;
				
			}else if(utf32str[x] >= 0x2139 && utf32str[x] <= 0x3299){
				utf32str[x] = 0x20;
			}else if(utf32str[x] >= 0x1F004 && utf32str[x] <= 0x1F9E6){
				utf32str[x] = 0x20;
				
			}
		}
		
		
		
		if(x % 39 == 0 && x != 0){
			//utf32str.insert(utf32str.begin() + x , 0xA);
			utf32str.insert( utf32str.begin() + x , 0xA);
			currentLineY++;
		}
	}

	utf8::utf32to8(utf32str.begin(), utf32str.end(), back_inserter(utf8result));
	
	m->content = std::string( utf8result.begin() , utf8result.end() );
}

void Discord::getChannelMessages(int channelIndex){
	currentChannel = channelIndex;
	std::string channelMessagesUrl = "https://discord.com/api/v9/channels/" + guilds[currentGuild].channels[currentChannel].id + "/messages?limit=100";

	if(guilds[currentGuild].channels[currentChannel].gotMessagesOnce ){
		channelMessagesUrl += "&after=" + guilds[currentGuild].channels[currentChannel].last_message_id;
	}

	VitaNet::http_response channelmessagesresponse = vitaNet.curlDiscordGet(channelMessagesUrl , token);
	logSD(channelmessagesresponse.body);
	if(channelmessagesresponse.httpcode == 200){
		nlohmann::json j_complete = nlohmann::json::parse(channelmessagesresponse.body);
		int messagesAmount = j_complete.size();


		if(!j_complete.is_null()){

			//guilds[currentGuild].channels[currentChannel].messages.clear();

			for( int i = 0 ; i < messagesAmount ; i++){

				int iR = messagesAmount - i - 1;

				message newMessage;

				if(!j_complete[iR].is_null()){

					if(!j_complete[iR]["timestamp"].is_null()){
						newMessage.timestamp = j_complete[iR]["timestamp"].get<std::string>();
					}else{
						newMessage.timestamp = "0";
					}

					if(!j_complete[iR]["id"].is_null()){
						newMessage.id = j_complete[iR]["id"].get<std::string>();
						guilds[currentGuild].channels[currentChannel].last_message_id = newMessage.id;
					}else{
						newMessage.id = "0";
					}

					if(!j_complete[iR]["content"].is_null()){

						//std::string str =
						//char * content = new char [str.length()+1];
						//std::strcpy (content, str.c_str());
						//char * contentUtf8 = new char [str.length()+1];
						//utf16_to_utf8((uint16_t *)content , (uint8_t *) contentUtf8);
						parseMessageContentEmoji(&newMessage , j_complete[iR]["content"].get<std::string>() );
						//newMessage.content = j_complete[iR]["content"].get<std::string>();
					}else{
						newMessage.content = "";
					}

					if (j_complete[iR].contains("mentions") && !j_complete[iR]["mentions"].is_null()) {
						int mAmount = j_complete[iR]["mentions"].size();
						for (int m = 0; m < mAmount; m++) {
							if (!j_complete[iR]["mentions"][m]["id"].is_null() && !j_complete[iR]["mentions"][m]["username"].is_null()) {
								std::string mId = j_complete[iR]["mentions"][m]["id"].get<std::string>();
								std::string mUser = j_complete[iR]["mentions"][m]["username"].get<std::string>();
								newMessage.mentionsMap[mId] = mUser;
							}
						}
					}

					// author :
					if(!j_complete[iR]["author"]["username"].is_null()){
						newMessage.author.username = j_complete[iR]["author"]["username"].get<std::string>();
					}else{
						newMessage.author.username = "N/A";
					}

					if(!j_complete[iR]["author"]["discriminator"].is_null()){
						newMessage.author.discriminator = j_complete[iR]["author"]["discriminator"].get<std::string>();
					}else{
						newMessage.author.discriminator = "N/A";
					}

					if(!j_complete[iR]["author"]["id"].is_null()){
						newMessage.author.id = j_complete[iR]["author"]["id"].get<std::string>();
					}else{
						newMessage.author.id = "0";
					}

					if(!j_complete[iR]["author"]["avatar"].is_null()){
						newMessage.author.avatar = j_complete[iR]["author"]["avatar"].get<std::string>();
					}else{
						newMessage.author.avatar = "0";
					}

					std::string authorId = newMessage.author.id;
					unsigned int hash = 0;
					for (char c : authorId) {
						hash = hash * 31 + c;
					}
					unsigned char r_col = (hash & 0xFF) % 128 + 127;
					unsigned char g_col = ((hash >> 8) & 0xFF) % 128 + 127;
					unsigned char b_col = ((hash >> 16) & 0xFF) % 128 + 127;
					newMessage.author.color = RGBA8(r_col, g_col, b_col, 255);

					newMessage.attachment.isEmpty = true;
					
					bool attachmentDownloadEnabled = false; // Always false for now
					
					if(!j_complete[iR]["attachments"].is_null()){
						if(!j_complete[iR]["attachments"][0].is_null()){

							newMessage.attachment.isEmpty = false;
							newMessage.attachment.isImage = false ;
							newMessage.attachment.isData = false ;
							newMessage.attachment.loadedThumbImage = false;

							bool proxyAvailable = false;
							bool filenameAvailable = false;
							bool imageDimensionAvailable = false;
							bool sizeAvailable = false;
							bool urlAvailable = false;

							if(!j_complete[iR]["attachments"][0]["url"].is_null()){
								newMessage.attachment.url = j_complete[iR]["attachments"][0]["url"].get<std::string>();
								urlAvailable=true;
							}else{
								newMessage.attachment.url = "";
							}

							if(!j_complete[iR]["attachments"][0]["proxy_url"].is_null()){
								newMessage.attachment.proxy_url = j_complete[iR]["attachments"][0]["proxy_url"].get<std::string>();
								proxyAvailable = true;
							}else{
								newMessage.attachment.proxy_url = "";
							}

							if(!j_complete[iR]["attachments"][0]["filename"].is_null()){
								newMessage.attachment.filename = j_complete[iR]["attachments"][0]["filename"].get<std::string>();
								filenameAvailable = true;
							}else{
								newMessage.attachment.filename = "noname.png";
							}

							if(!j_complete[iR]["attachments"][0]["id"].is_null()){
								newMessage.attachment.id = j_complete[iR]["attachments"][0]["id"].get<std::string>();
							}else{
								newMessage.attachment.id = "";
							}


							if ( !j_complete[iR]["attachments"][0]["width"].is_null() ){
								newMessage.attachment.width = j_complete[iR]["attachments"][0]["width"].get<int>();
								imageDimensionAvailable = true;
							} else {
								newMessage.attachment.width = -1;
							}

							if ( !j_complete[iR]["attachments"][0]["height"].is_null() ){
								newMessage.attachment.height = j_complete[iR]["attachments"][0]["height"].get<int>();
							} else {
								newMessage.attachment.height = -1;
							}

							if ( !j_complete[iR]["attachments"][0]["size"].is_null() ){
								newMessage.attachment.size = j_complete[iR]["attachments"][0]["size"].get<int>();
								sizeAvailable = true;

								if(newMessage.attachment.size > 1024*1024){
									newMessage.attachment.readableSize = static_cast<int> (  newMessage.attachment.size / ( 1024 * 1024 ) );
									newMessage.attachment.readableSizeUnit = "MiB";
								}else if(newMessage.attachment.size > 1024){
									newMessage.attachment.readableSize =  static_cast<int> (  newMessage.attachment.size / ( 1024 ) );
									newMessage.attachment.readableSizeUnit = "KiB";
								}else{
									newMessage.attachment.readableSize =  static_cast<int> (  newMessage.attachment.size );
									newMessage.attachment.readableSizeUnit = "Byte";
								}


							} else {
								newMessage.attachment.size = -1;
							}

							if ( proxyAvailable && filenameAvailable && imageDimensionAvailable ){
								newMessage.attachment.isImage = true ;
								newMessage.attachment.isData = false ;
								// Disable downloading thumb data
								newMessage.attachment.loadedThumbImage = true; // Use text placeholder instead
								newMessage.attachment.isImage = true;
								newMessage.attachment.isData = false;
							}else if ( urlAvailable && filenameAvailable && sizeAvailable ){

								loaddata:		// Label for goto
								// Disable downloading file data
								newMessage.attachment.isImage = false ;
								newMessage.attachment.isData = true ;
							}else{
								newMessage.attachment.isEmpty = true ;
								newMessage.attachment.isImage = false ;
								newMessage.attachment.isData = false ;

							}


						}
					}



				}


				guilds[currentGuild].channels[currentChannel].messages.push_back(newMessage);



			}


			//std::reverse(guilds[currentGuild].channels[currentChannel].messages.begin() , guilds[currentGuild].channels[currentChannel].messages.end());

			guilds[currentGuild].channels[currentChannel].gotMessagesOnce = true;

		}
		debugNetPrintf(DEBUG , "End of getchannelmessages!!");
		lastFetchTimeMS = osGetTimeMS();

	}

}
void Discord::JoinGuild(int gIndex){
	currentGuild = gIndex;
}
void Discord::JoinChannel(int cIndex){
	inChannel = true;
	currentChannel = cIndex;
	forceRefreshMessages = true;
	refreshMessages();
	forceRefreshMessages = false;

	if(!pthreadStarted){
		debugNetPrintf(DEBUG , "Startint pthread refresh Messages\n");

		pthreadStarted = true;
		logSD("pthread_create( loadDataThread , NULL , wrapper , 0)");
		debugNetPrintf(DEBUG , "pthread_create coming\n");
		if( int errP = pthread_create(&loadMessagesThread, NULL, &Discord::refreshMessages_wrapper, this) != 0){
			debugNetPrintf(DEBUG , "PTHREAD_CREATE ERROR : %d\n" , errP);
			debugNetPrintf(DEBUG , "PTHREAD_CREATE ERROR : %d\n" , errP);
			debugNetPrintf(DEBUG , "PTHREAD_CREATE ERROR : %d\n" , errP);
			pthreadStarted = false;

		}else{

			debugNetPrintf(DEBUG , "successfully started pthread\n");

		}
	}
	//getChannelMessages(currentChannel);
}
void Discord::LeaveChannel(){
	inChannel = false;
	currentChannel = 0;
}
void Discord::setToken(std::string tok){
	token = tok;
}


void * Discord::thread_loadData(void *arg){

	Discord *discordPtr = reinterpret_cast<Discord *>(arg);
	logSD("start of thread_loadData");
	discordPtr->loadingData = true;
	while(discordPtr->loadingData){
		if(!discordPtr->loadedGuilds){
			std::string guildsUrl = "https://discord.com/api/v9/users/@me/guilds";
			VitaNet::http_response guildsresponse = discordPtr->vitaNet.curlDiscordGet(guildsUrl , discordPtr->token);
			logSD(guildsresponse.body);
			if(guildsresponse.httpcode == 200){
				try{
					nlohmann::json j_complete = nlohmann::json::parse(guildsresponse.body);
					if(!j_complete.is_null()){
						discordPtr->guilds.clear();
						discordPtr->guildsAmount = j_complete.size();
						for(int i = 0; i < guildsAmount; i++){

							discordPtr->guilds.push_back(guild());

							if(!j_complete[i].is_null()){


								if(!j_complete[i]["owner"].is_null()){
									discordPtr->guilds[i].owner = j_complete[i]["owner"].get<bool>();
								}else{
									discordPtr->guilds[i].owner = false;
								}

								if(!j_complete[i]["permissions"].is_null()){
									if (j_complete[i]["permissions"].is_string()) {
										discordPtr->guilds[i].permissions = std::stoull(j_complete[i]["permissions"].get<std::string>());
									} else {
										discordPtr->guilds[i].permissions = j_complete[i]["permissions"].get<uint64_t>();
									}
								}else{
									discordPtr->guilds[i].permissions = 0;
								}

								if(!j_complete[i]["icon"].is_null()){
									discordPtr->guilds[i].icon = j_complete[i]["icon"].get<std::string>();
								}else{
									discordPtr->guilds[i].icon = "";
								}

								if(!j_complete[i]["id"].is_null()){
									discordPtr->guilds[i].id = j_complete[i]["id"].get<std::string>();
									logSD(discordPtr->guilds[i].id);
								}else{
									discordPtr->guilds[i].id = "";
									logSD(discordPtr->guilds[i].id);
								}

								if(!j_complete[i]["name"].is_null()){
									discordPtr->guilds[i].name = j_complete[i]["name"].get<std::string>();
									logSD(discordPtr->guilds[i].name);

								}else{
									discordPtr->guilds[i].name = "";
									logSD(discordPtr->guilds[i].name);
								}



							}


						}
						discordPtr->loadedGuilds = true;
					}
				}catch(const std::exception& e){
					discordPtr->loadedGuilds = true;
				}

			}else{
				discordPtr->loadedGuilds = true;
			}
		}else if(discordPtr->loadedGuilds && !discordPtr->loadedChannels){





			for(int i = 0; i < discordPtr->guildsAmount ; i++){



				std::string myRolesUrl ="https://discord.com/api/v9/guilds/" + discordPtr->guilds[i].id + "/members/" + discordPtr->id;
				VitaNet::http_response myRolesResponse = discordPtr->vitaNet.curlDiscordGet(myRolesUrl , discordPtr->token);
				if(myRolesResponse.httpcode == 200){
					try{
						nlohmann::json j_complete = nlohmann::json::parse(myRolesResponse.body);
						if(!j_complete.is_null()){

							if(!j_complete["roles"].is_null()){

								discordPtr->guilds[i].myroles.clear();
								int rolesAmount = j_complete["roles"].size();
								for(int rol = 0; rol < rolesAmount ; rol++){
									if(!j_complete["roles"][rol].is_null()){
										std::string role = j_complete["roles"][rol].get<std::string>();
										discordPtr->guilds[i].myroles.push_back(role);

									}

								}

							}

						}
					}catch(const std::exception& e){
						// nothing
					}

				}






				std::string channelUrl = "https://discord.com/api/v9/guilds/" + discordPtr->guilds[i].id + "/channels";
				VitaNet::http_response channelresponse = discordPtr->vitaNet.curlDiscordGet(channelUrl , discordPtr->token);
				logSD(channelresponse.body);
				if(channelresponse.httpcode == 200){
					try{
						logSD("Create nlohmann json object by parsing response");
						nlohmann::json j_complete = nlohmann::json::parse(channelresponse.body);
						if(!j_complete.is_null()){
							discordPtr->guilds[i].channels.clear();
							int channelsAmount = j_complete.size();

							logSD("Channel amount " + std::to_string(channelsAmount));
							
							for(int c = 0; c < channelsAmount; c++){

								logSD("Adding to be filled out channel to channel vector");
								discordPtr->guilds[i].channels.push_back(channel());
								logSD("Added to be filled out channel to channel vector");

								logSD("Check current json object is null");
								if(!j_complete[c].is_null()){

									logSD(" current json object was not null.");
									if(!j_complete[c]["type"].is_null()){
										logSD("parsing type, which was not null");
										discordPtr->guilds[i].channels[c].type = j_complete[c]["type"].get<int>();
										logSD(std::to_string(discordPtr->guilds[i].channels[c].type));
									}else{
										logSD("setting type = 0 because json.type was null");
										discordPtr->guilds[i].channels[c].type = 0;
										logSD(std::to_string(discordPtr->guilds[i].channels[c].type));
									}

									if(!j_complete[c]["id"].is_null()){
										logSD("parsing id, which was not null");
										discordPtr->guilds[i].channels[c].id = j_complete[c]["id"].get<std::string>();
										logSD(discordPtr->guilds[i].channels[c].id);
									}else{
										logSD("setting id = 00000000 because json.id was null");
										discordPtr->guilds[i].channels[c].id = "00000000";
										logSD(discordPtr->guilds[i].channels[c].id);
									}

									if(!j_complete[c]["name"].is_null()){
										logSD("parsing name, which was not null");
										discordPtr->guilds[i].channels[c].name = j_complete[c]["name"].get<std::string>();
										logSD(discordPtr->guilds[i].channels[c].name);
									}else{
										logSD("setting name unavailable because json.name was null");
										discordPtr->guilds[i].channels[c].name = "name unavailable";
										logSD(discordPtr->guilds[i].channels[c].name);
									}

									if(!j_complete[c]["topic"].is_null()){
										logSD("parsing topic, which was not null");
										discordPtr->guilds[i].channels[c].topic = j_complete[c]["topic"].get<std::string>();
										logSD(discordPtr->guilds[i].channels[c].topic);
									}else{
										logSD("setting topic empty because json.topic was null");
										discordPtr->guilds[i].channels[c].topic = " ";
										logSD(discordPtr->guilds[i].channels[c].topic);
									}

									if(!j_complete[c]["is_private"].is_null()){
										logSD("parsing is_private, which was not null");
										discordPtr->guilds[i].channels[c].is_private = j_complete[c]["is_private"].get<bool>();
										logSD(std::to_string(discordPtr->guilds[i].channels[c].is_private));
									}else{
										logSD("setting is_private false because json.is_private was null");
										discordPtr->guilds[i].channels[c].is_private = false;
										logSD(std::to_string(discordPtr->guilds[i].channels[c].is_private));
									}

									if(!j_complete[c]["last_message_id"].is_null()){
										logSD("parsing last_message_id, which was not null");
										discordPtr->guilds[i].channels[c].last_message_id = j_complete[c]["last_message_id"].get<std::string>();
										logSD(discordPtr->guilds[i].channels[c].last_message_id);
									}else{
										logSD("setting last_message_id = 00000000 because json.last_message_id was null");
										discordPtr->guilds[i].channels[c].last_message_id = "00000000";
										logSD(discordPtr->guilds[i].channels[c].last_message_id);
									}

									if(!j_complete[c]["permission_overwrites"].is_null()){

										logSD("parsing permission_overwrites , which was not null");
										int p = j_complete[c]["permission_overwrites"].size();
										discordPtr->guilds[i].channels[c].permission_overwrites.clear();
										for(int per = 0; per < p; per++){
											discordPtr->guilds[i].channels[c].permission_overwrites.push_back(permission_overwrite());
											if(!j_complete[c]["permission_overwrites"][per]["allow"].is_null()){
												discordPtr->guilds[i].channels[c].permission_overwrites[per].allow = j_complete[c]["permission_overwrites"][per]["allow"].get<int>();

											}else{
												discordPtr->guilds[i].channels[c].permission_overwrites[per].allow = 0;
											}

											if(!j_complete[c]["permission_overwrites"][per]["type"].is_null()){
												discordPtr->guilds[i].channels[c].permission_overwrites[per].type = j_complete[c]["permission_overwrites"][per]["type"].get<std::string>();
											}else{
												discordPtr->guilds[i].channels[c].permission_overwrites[per].type = "role";
											}

											if(!j_complete[c]["permission_overwrites"][per]["id"].is_null()){
												discordPtr->guilds[i].channels[c].permission_overwrites[per].id = j_complete[c]["permission_overwrites"][per]["id"].get<std::string>();
											}else{
												discordPtr->guilds[i].channels[c].permission_overwrites[per].id = "0";
											}

											if(!j_complete[c]["permission_overwrites"][per]["deny"].is_null()){
												discordPtr->guilds[i].channels[c].permission_overwrites[per].deny = j_complete[c]["permission_overwrites"][per]["deny"].get<int>();
											}else{
												discordPtr->guilds[i].channels[c].permission_overwrites[per].deny = 0;
											}

										}
										logSD("end of parsing permission_overwrites.");


										// TODO : LEARN HOW TO REALLY CHECK PERMISSION !!!
										//bool readAllowedForMeOnce = false; // if one role has read rest doesnt matter
										//bool readDeniedForMeOnce = false;
										//
										//discordPtr->guilds[i].channels[c].readallowed = false;
										//
										//for(int permC = 0; permC < discordPtr->guilds[i].channels[c].permission_overwrites.size() ; permC++){
										//
										//	// check role "@everyone" ( = guildid)
										//	if(discordPtr->guilds[i].channels[c].permission_overwrites[permC].id == discordPtr->guilds[i].id){
										//		if(!(discordPtr->guilds[i].channels[c].permission_overwrites[permC].deny & PERMISSION_READ_MESSAGES)){
										//			readDeniedForMeOnce = true;
										//		}else{
										//			readAllowedForMeOnce = true;
										//		}
										//	}
										//
										//	// check all roles i have
										//	for(int myR = 0; myR < discordPtr->guilds[i].myroles.size() ; myR++){
										//		if(discordPtr->guilds[i].channels[c].permission_overwrites[permC].id == discordPtr->guilds[i].myroles[myR]){
										//
										//			if(!(discordPtr->guilds[i].channels[c].permission_overwrites[permC].deny & PERMISSION_READ_MESSAGES)){
										//				readDeniedForMeOnce = true;
										//			}else{
										//				readAllowedForMeOnce = true;
										//			}
										//		}
										//	}
										//
										//
										//
										//
										//
										//}
										//
										//if(readAllowedForMeOnce){
										//	discordPtr->guilds[i].channels[c].readallowed = true;
										//}else if(readDeniedForMeOnce){
										//	discordPtr->guilds[i].channels[c].readallowed = false;
										//}else{
										//	discordPtr->guilds[i].channels[c].readallowed = true;
										//}


									}else{
										// no permission_overwrites
									}



								}else{
									logSD(" channel json is null");
								}

							}

						}else{
							
							logSD(" channels! json is null");
						}
					}catch(const std::exception& e){

						logSD(" exception while loading channels");
						discordPtr->loadedChannels = true;
					}
				}

			}
			discordPtr->loadedChannels = true;
		}else if(discordPtr->loadedGuilds && discordPtr->loadedChannels && !discordPtr->loadedDMs){

			std::string directMessagesChannelsUrl = "https://discord.com/api/v9/users/@me/channels";
			VitaNet::http_response dmChannelsResponse = discordPtr->vitaNet.curlDiscordGet(directMessagesChannelsUrl , discordPtr->token);
			logSD(dmChannelsResponse.body);
			if(dmChannelsResponse.httpcode == 200){
				try{
					nlohmann::json j_complete = nlohmann::json::parse(dmChannelsResponse.body);
					if(!j_complete.is_null()){
						discordPtr->directMessages.clear();
						int dmChannels = j_complete.size();
						logSD("Amount of DM channels : " + std::to_string(dmChannels));
						for(int i = 0; i < dmChannels; i++){
							discordPtr->directMessages.push_back(directMessage());
							logSD("dm channel added.");

							if(!j_complete[i]["last_message_id"].is_null()){
								discordPtr->directMessages[i].last_message_id = j_complete[i]["last_message_id"].get<std::string>();
								logSD("last message id : ." + discordPtr->directMessages[i].last_message_id);
							}else{
								discordPtr->directMessages[i].last_message_id = "0000000000000000";
								logSD("last message id : ." + discordPtr->directMessages[i].last_message_id);
							}
							if(!j_complete[i]["type"].is_null()){
								discordPtr->directMessages[i].type = j_complete[i]["type"].get<long>();
								logSD("type : ." + std::to_string(discordPtr->directMessages[i].type));
							}else{
								discordPtr->directMessages[i].type = 1;
								logSD("type : ." + std::to_string(discordPtr->directMessages[i].type));
							}
							if(!j_complete[i]["id"].is_null()){
								discordPtr->directMessages[i].id = j_complete[i]["id"].get<std::string>();
								logSD("last id : ." + discordPtr->directMessages[i].id);
							}else{
								discordPtr->directMessages[i].id = "0000000000000000";
								logSD("last id : ." + discordPtr->directMessages[i].id);
							}
							logSD("checking for recipients");
							if(!j_complete[i]["recipients"].is_null()){
								discordPtr->directMessages[i].recipients.clear();
								int recAmount = j_complete[i]["recipients"].size();
								logSD("Amount of recipients : " + std::to_string(recAmount));
								for(int r = 0; r < recAmount  ; r++){
									// author :
									logSD("Adding recipient ");
									discordPtr->directMessages[i].recipients.push_back(user());
									if(!j_complete[i]["recipients"][r]["username"].is_null()){
										discordPtr->directMessages[i].recipients[r].username = j_complete[i]["recipients"][r]["username"].get<std::string>();
										logSD("username : " + discordPtr->directMessages[i].recipients[r].username);
									}else{
										discordPtr->directMessages[i].recipients[r].username = "";
										logSD("username : " + discordPtr->directMessages[i].recipients[r].username);
									}

									if(!j_complete[i]["recipients"][r]["discriminator"].is_null()){
										discordPtr->directMessages[i].recipients[r].discriminator = j_complete[i]["recipients"][r]["discriminator"].get<std::string>();
										logSD("discriminator : " + discordPtr->directMessages[i].recipients[r].discriminator);
									}else{
										discordPtr->directMessages[i].recipients[r].discriminator = "";
										logSD("discriminator : " + discordPtr->directMessages[i].recipients[r].discriminator);
									}

									if(!j_complete[i]["recipients"][r]["id"].is_null()){
										discordPtr->directMessages[i].recipients[r].id = j_complete[i]["recipients"][r]["id"].get<std::string>();
										logSD("id : " + discordPtr->directMessages[i].recipients[r].id);
									}else{
										discordPtr->directMessages[i].recipients[r].id = "";
										logSD("id : " + discordPtr->directMessages[i].recipients[r].id);
									}

									if(!j_complete[i]["recipients"][r]["avatar"].is_null()){
										discordPtr->directMessages[i].recipients[r].avatar = j_complete[i]["recipients"][r]["avatar"].get<std::string>();
										logSD("avatar : " + discordPtr->directMessages[i].recipients[r].avatar);
									}else{
										discordPtr->directMessages[i].recipients[r].avatar = "";
										logSD("avatar : " + discordPtr->directMessages[i].recipients[r].avatar);
									}


									logSD("end of adding recipient.");
								}
							}

							logSD("end of this DM channel.");



						}

					}

				}catch(const std::exception& e){
					logSD("/EXCEPTION THROWN!!!");
					logSD(":EXCEPTION THROWN!!!");
					logSD(":EXCEPTION THROWN!!!");
					logSD(":EXCEPTION THROWN!!!");
					logSD("\\EXCEPTION THROWN!!!");
				}

			}
			discordPtr->loadedDMs = true;
			discordPtr->loadingData = false;
		}
	}
	logSD("end of thread_loadData()");
	pthread_exit(NULL);
	return NULL;
}


void * Discord::thread_refreshMessages(void *arg){
	Discord *discordPtr = reinterpret_cast<Discord *>(arg);

	//discordPtr->getChannelMessages(discordPtr->currentChannel);
	while(discordPtr->inChannel){
		discordPtr->refreshMessages();
		sceKernelDelayThread(1000000);

	}
	pthreadStarted = false;
	pthread_exit(NULL);
	return NULL;
}

void Discord::LeaveDirectMessageChannel(){
	currentDirectMessage = 0;
	inDirectMessageChannel = false;


}
void Discord::JoinDirectMessageChannel(int dIndex){
	currentDirectMessage = dIndex;
	inDirectMessageChannel = true;
	loadingDirectMessages = true;
	getCurrentDirectMessages();
	loadingDirectMessages = false;


}

void Discord::getDirectMessageChannels(){
	std::string directMessagesChannelsUrl = "https://discord.com/api/v9/users/@me/channels";
	VitaNet::http_response dmChannelsResponse = vitaNet.curlDiscordGet(directMessagesChannelsUrl , token);

	if(dmChannelsResponse.httpcode == 200){
		try{
			nlohmann::json j_complete = nlohmann::json::parse(dmChannelsResponse.body);
			if(!j_complete.is_null()){
				directMessages.clear();
				int dmChannels = j_complete.size();
				for(int i = 0; i < dmChannels; i++){
					directMessages.push_back(directMessage());

					if(!j_complete[i]["last_message_id"].is_null()){
						directMessages[i].last_message_id = j_complete[i]["last_message_id"].get<std::string>();
					}else{
						directMessages[i].last_message_id = "0000000000000000";
					}
					if(!j_complete[i]["type"].is_null()){
						directMessages[i].type = j_complete[i]["type"].get<long>();
					}else{
						directMessages[i].type = 1;
					}
					if(!j_complete[i]["id"].is_null()){
						directMessages[i].id = j_complete[i]["id"].get<std::string>();
					}else{
						directMessages[i].id = "0000000000000000";
					}
					if(!j_complete[i]["recipients"].is_null()){
						directMessages[i].recipients.clear();
						int recAmount = j_complete[i]["recipients"].size();
						for(int r = 0; r < recAmount  ; r++){
							// author :
							directMessages[i].recipients.push_back(user());
							if(!j_complete[i]["recipients"][r]["username"].is_null()){
								directMessages[i].recipients[r].username = j_complete[i]["recipients"][r]["username"].get<std::string>();
							}else{
								directMessages[i].recipients[r].username = "";
							}

							if(!j_complete[i]["recipients"][r]["discriminator"].is_null()){
								directMessages[i].recipients[r].discriminator = j_complete[i]["recipients"][r]["discriminator"].get<std::string>();
							}else{
								directMessages[i].recipients[r].discriminator = "";
							}

							if(!j_complete[i]["recipients"][r]["id"].is_null()){
								directMessages[i].recipients[r].id = j_complete[i]["recipients"][r]["id"].get<std::string>();
							}else{
								directMessages[i].recipients[r].id = "";
							}

							if(!j_complete[i]["recipients"][r]["avatar"].is_null()){
								directMessages[i].recipients[r].avatar = j_complete[i]["recipients"][r]["avatar"].get<std::string>();
							}else{
								directMessages[i].recipients[r].avatar = "";
							}


						}
					}




				}

			}

		}catch(const std::exception& e){
			logSD("/EXCEPTION THROWN!!!");
			logSD(":EXCEPTION THROWN!!!");
			logSD(":EXCEPTION THROWN!!!");
			logSD(":EXCEPTION THROWN!!!");
			logSD("\\EXCEPTION THROWN!!!");
		}

	}
	lastFetchTimeMS = osGetTimeMS();

}


bool Discord::refreshDirectMessages(){

	currentTimeMS = osGetTimeMS();
	if(currentTimeMS - lastFetchTimeMS > fetchTimeMS){
		lastFetchTimeMS = osGetTimeMS();
		getDirectMessageChannels();
		return true;
	}
	return false;
}
bool Discord::refreshCurrentDirectMessages(){

	currentTimeMS = osGetTimeMS();
	if(currentTimeMS - lastFetchTimeMS > fetchTimeMS){
		lastFetchTimeMS = osGetTimeMS();
		getCurrentDirectMessages();
		return true;
	}
	return false;
}

void Discord::getCurrentDirectMessages(){
	std::string dmChannelUrl = "https://discord.com/api/v9/channels/" + directMessages[currentDirectMessage].id + "/messages";
	VitaNet::http_response dmChannelResponse = vitaNet.curlDiscordGet(dmChannelUrl , token);



	if(dmChannelResponse.httpcode == 200){
		try{
			nlohmann::json j_complete = nlohmann::json::parse(dmChannelResponse.body);
			if(!j_complete.is_null()){
				directMessages[currentDirectMessage].messages.clear();
				int msgAmount = j_complete.size();
				for(int i = 0; i < msgAmount; i++){

					directMessages[currentDirectMessage].messages.push_back(message());

					if(!j_complete[i]["timestamp"].is_null()){
						directMessages[currentDirectMessage].messages[i].timestamp = j_complete[i]["timestamp"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].timestamp = "";
					}

					if(!j_complete[i]["id"].is_null()){
						directMessages[currentDirectMessage].messages[i].id = j_complete[i]["id"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].id = "";
					}

					if(!j_complete[i]["content"].is_null()){
						directMessages[currentDirectMessage].messages[i].content = j_complete[i]["content"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].content = "";
					}

					if (j_complete[i].contains("mentions") && !j_complete[i]["mentions"].is_null()) {
						int mAmount = j_complete[i]["mentions"].size();
						for (int m = 0; m < mAmount; m++) {
							if (!j_complete[i]["mentions"][m]["id"].is_null() && !j_complete[i]["mentions"][m]["username"].is_null()) {
								std::string mId = j_complete[i]["mentions"][m]["id"].get<std::string>();
								std::string mUser = j_complete[i]["mentions"][m]["username"].get<std::string>();
								directMessages[currentDirectMessage].messages[i].mentionsMap[mId] = mUser;
							}
						}
					}

					// author :
					if(!j_complete[i]["author"]["username"].is_null()){
						directMessages[currentDirectMessage].messages[i].author.username = j_complete[i]["author"]["username"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].author.username = "";
					}

					if(!j_complete[i]["author"]["discriminator"].is_null()){
						directMessages[currentDirectMessage].messages[i].author.discriminator = j_complete[i]["author"]["discriminator"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].author.discriminator = "";
					}

					if(!j_complete[i]["author"]["id"].is_null()){
						directMessages[currentDirectMessage].messages[i].author.id = j_complete[i]["author"]["id"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].author.id = "";
					}

					if(!j_complete[i]["author"]["avatar"].is_null()){
						directMessages[currentDirectMessage].messages[i].author.avatar = j_complete[i]["author"]["avatar"].get<std::string>();
					}else{
						directMessages[currentDirectMessage].messages[i].author.avatar = "";
					}
					std::string authorId = directMessages[currentDirectMessage].messages[i].author.id;
					unsigned int hash = 0;
					for (char c : authorId) {
						hash = hash * 31 + c;
					}
					unsigned char r_col = (hash & 0xFF) % 128 + 127;
					unsigned char g_col = ((hash >> 8) & 0xFF) % 128 + 127;
					unsigned char b_col = ((hash >> 16) & 0xFF) % 128 + 127;
					directMessages[currentDirectMessage].messages[i].author.color = RGBA8(r_col, g_col, b_col, 255);
				}
			}
		}catch(const std::exception& e){

		}


		std::reverse( directMessages[currentDirectMessage].messages.begin() , directMessages[currentDirectMessage].messages.end() );

	}
	lastFetchTimeMS = osGetTimeMS();
}

void Discord::loadData(){
	logSD("inside loadData()");
	loadingData = true;
	logSD("pthread_t loadDataThread");
	pthread_t loadDataThread;
	logSD("pthread_create( loadDataThread , NULL , wrapper , 0)");
	pthread_create(&loadDataThread, NULL, &Discord::loadData_wrapper, this);
	logSD("end of loadData()");

}

long Discord::fetchUserData(){

	logSD("Fetching userdata");
	std::string userDataUrl = "https://discord.com/api/v9/users/@me";
	VitaNet::http_response userdataresponse = vitaNet.curlDiscordGet(userDataUrl , token);
	logSD("userdata response : " + userdataresponse.body);
	if(userdataresponse.httpcode == 200){
		// check if Two-Factor-Authentication is activated and needs further user action
		nlohmann::json j_complete = nlohmann::json::parse(userdataresponse.body);
		if(!j_complete.is_null()){
			if(!j_complete["username"].is_null()){
				username = j_complete["username"].get<std::string>();
			}
			if(!j_complete["verified"].is_null()){
				verified = j_complete["verified"].get<bool>();
			}
			if(!j_complete["mfa_enabled"].is_null()){
				mfa_enabled = j_complete["mfa_enabled"].get<bool>();
			}
			if(!j_complete["id"].is_null()){
				id = j_complete["id"].get<std::string>();
			}
			if(!j_complete["phone"].is_null()){
				phone = j_complete["phone"].get<std::string>();
			}
			if(!j_complete["avatar"].is_null()){
				avatar = j_complete["avatar"].get<std::string>();
			}
			if(!j_complete["discriminator"].is_null()){
				discriminator = j_complete["discriminator"].get<std::string>();
			}
		}


	}

	return userdataresponse.httpcode;

}

void Discord::getGuilds(){
	std::string guildUrl = "https://discord.com/api/v9/users/@me/guilds";
}
void Discord::getChannels(){

}
std::string Discord::getUsername(){
	return username;
}
long Discord::login(){
	return login(TOKEN);
}
long Discord::login(std::string tok){
	criticalLogSD("Login attempt.\n");

	token = tok;
	if(tok.length() < 1){
		criticalLogSD("Token too short! \n");
		return -11;
	}

	long fetchStatus = fetchUserData();
	if (fetchStatus == 200) {
		loggedin = true;
	}
	return fetchStatus;
}
std::string Discord::getToken(){
	return TOKEN;
}
std::string Discord::getTicket(){
	return ticket;
}

