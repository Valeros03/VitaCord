#ifndef GUIELEMENTS_HPP
#define GUIELEMENTS_HPP

#include <vita2d.h>
#include <string>
#include <vector>
#include <unordered_map>

typedef struct{
	int posX;
	int posY;
	int spriteSheetX;
	int spriteSheetY;
} m_emoji;
typedef struct{
	std::string name;
	int id;
	vita2d_texture *icon;
} emoji_icon;
typedef struct{
	float x,y,w,h;
	unsigned int color;
}rectangle;

typedef struct{
	std::string name;

}image;


typedef struct{
	float x,y,w,h;
} click_rect;

typedef struct{
	std::string url;
	int startIdx;
	int endIdx;
	std::vector<click_rect> boxes;
} parsed_url;


typedef struct{
	int x;
	int y;
	int w;
	int h;

}inputbox;


typedef struct{
	int x;
	int y;
	int w;
	int h;

}box;

typedef struct{
	int x;
	int y;
	int w;
	int h;
	int lineCount;
	int messageHeight;
	std::string username;
	unsigned int userColor = 0;
	std::string content;
	std::string channelID;
	std::string messageID;
	std::unordered_map<std::string, std::string> mentionsMap;
	bool showAttachmentAsImage;
	bool showAttachmentAsBinary;
	std::string attachmentFilename;
	std::string attachmentUrl;
	vita2d_texture * attachmentThumbnail;
	int attachmentReadableSize;
	std::string attachmentReadableSizeUnit;
	std::string attachmentFullText;

	std::vector<m_emoji> emojis;

	std::vector<parsed_url> urls;
	click_rect attachmentBox;
}messagebox;

typedef struct{
	int x;
	int y;
	int w;
	int h;
	int dmIndex;
	std::string name;
}dmBox;

typedef struct{
	int x;
	int y;
	int w;
	int h;
	int channelIndex;
	std::string name;
}channelBox;


typedef struct{
	int x;
	int y;
	int w;
	int h;
	int guildIndex;
	std::string name;
}guildBox;


#endif
