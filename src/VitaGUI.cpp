#include "VitaGUI.hpp"
#include <pthread.h>
#include <psp2/photoexport.h>
#include "VitaNet.hpp"
#include "log.hpp"
#include <istream>
#include <sstream>
#include <iterator>
#include <psp2/io/dirent.h>
#include <psp2/power.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <debugnet.h>
#include <psp2/kernel/processmgr.h>
#include <malloc.h>
#include <psp2/sysmodule.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))



std::vector<parsed_url> parseUrls(const std::string& text) {
	std::vector<parsed_url> urls;
	size_t startPos = 0;

	while (true) {
		size_t httpPos = text.find("http://", startPos);
		size_t httpsPos = text.find("https://", startPos);

		size_t foundPos = std::string::npos;
		if (httpPos != std::string::npos && httpsPos != std::string::npos) {
			foundPos = min(httpPos, httpsPos);
		} else if (httpPos != std::string::npos) {
			foundPos = httpPos;
		} else if (httpsPos != std::string::npos) {
			foundPos = httpsPos;
		} else {
			break;
		}

		size_t endPos = text.find_first_of(" \t\n\r", foundPos);
		if (endPos == std::string::npos) {
			endPos = text.length();
		}

		parsed_url pUrl;
		pUrl.url = text.substr(foundPos, endPos - foundPos);
		pUrl.startIdx = foundPos;
		pUrl.endIdx = endPos;
		urls.push_back(pUrl);

		startPos = endPos;
	}

	return urls;
}

std::string cleanMentions(std::string text, const std::unordered_map<std::string, std::string>& localMentions) {
	size_t start = 0;
	while ((start = text.find("<@", start)) != std::string::npos) {
		size_t end = text.find(">", start);
		if (end != std::string::npos) {
			size_t idStart = start + 2;
			if (idStart < text.length() && (text[idStart] == '!' || text[idStart] == '&')) {
				idStart++;
			}
			std::string id = text.substr(idStart, end - idStart);

			auto it = localMentions.find(id);
			std::string replacement = (it != localMentions.end()) ? "@" + it->second : "@" + id;

			text.replace(start, end - start + 1, replacement);
			start += replacement.length();
		} else {
			start += 2;
		}
	}
	return text;
}

void VitaGUI::DrawTextWithEmojis(std::string text, int startX, int startY, int size, int maxWidth, std::vector<parsed_url>* urls) {

    int currentX = startX;
    int currentY = startY;
    size_t i = 0;

    while (i < text.length()) {

        unsigned char c = (unsigned char)text[i];
        size_t charLen = 1;

        if (c == '\n') {
            currentX = startX;
            currentY += size + 4;
            i++;
            continue;
        }

        uint32_t codepoint = 0;
        if (c <= 0x7F) {
            codepoint = c;
        } 
        else if ((c & 0xE0) == 0xC0 && i + 1 < text.length()) {
            uint8_t c1 = (uint8_t)text[i+1];
            codepoint = ((c & 0x1F) << 6) | (c1 & 0x3F);
            charLen = 2;
        } 
        else if ((c & 0xF0) == 0xE0 && i + 2 < text.length()) {
            uint8_t c1 = (uint8_t)text[i+1];
            uint8_t c2 = (uint8_t)text[i+2];
            codepoint = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
            charLen = 3;
        } 
        else if ((c & 0xF8) == 0xF0 && i + 3 < text.length()) {
            uint8_t c1 = (uint8_t)text[i+1];
            uint8_t c2 = (uint8_t)text[i+2];
            uint8_t c3 = (uint8_t)text[i+3];
            codepoint = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            charLen = 4;
        }

        auto it = discordPtr->fastEmojiMap.find(codepoint);
        int activeUrlIndex = -1;
        if (urls != nullptr) {
            for (size_t u = 0; u < urls->size(); u++) {
                if (i >= (*urls)[u].startIdx && i < (*urls)[u].endIdx) {
                    activeUrlIndex = u;
                    break;
                }
            }
        }
    
        if (it != discordPtr->fastEmojiMap.end()) {
            int itemWidth = 16 * 2.0f;
            if (currentX + itemWidth > startX + maxWidth) {
                currentX = startX;
                currentY += size + 4;
            }
            Discord::EmojiData eData = discordPtr->emojiVector[it->second];
            if (discordPtr->spritesheetEmoji != NULL) {
                vita2d_draw_texture_part_scale(discordPtr->spritesheetEmoji,
                                            currentX, currentY - size + 10,
                                            eData.x * discordPtr->emojiWidth,
                                            eData.y * discordPtr->emojiHeight,
                                            discordPtr->emojiWidth,
                                            discordPtr->emojiHeight,
                                            2.0f, 2.0f);
            }
            currentX += itemWidth;
        } else {
            std::string rawChar = text.substr(i, charLen);
            int itemWidth = vita2d_font_text_width(vita2dFont[size], size, rawChar.c_str());

            // La "Ghigliottina" (Ora funzionerà solo come estrema emergenza per parole lunghissime)
            if (currentX + itemWidth > startX + maxWidth) {
                currentX = startX;
                currentY += size + 4;
            }

            unsigned int charColor = RGBA8(255, 255, 255, 255);
            if (activeUrlIndex != -1) {
                charColor = RGBA8(0, 150, 255, 255); // Blu per i link

                if (urls != nullptr) {
                    parsed_url& purl = (*urls)[activeUrlIndex];
                    int fontHeight = vita2d_font_text_height(vita2dFont[size], size, rawChar.c_str());
                    int boxY = currentY - fontHeight; 

                    if (purl.boxes.size() > 0 && abs(purl.boxes.back().y - boxY) < 10 && (currentX - (purl.boxes.back().x + purl.boxes.back().w)) < 10) {
                        purl.boxes.back().w += itemWidth;
                        purl.boxes.back().h = max((float)purl.boxes.back().h, (float)fontHeight);
                    } else {
                        click_rect newBox;
                        newBox.x = currentX;
                        newBox.y = boxY;
                        newBox.w = itemWidth;
                        newBox.h = fontHeight;
                        purl.boxes.push_back(newBox);
                    }
                }
            }

            vita2d_font_draw_text(vita2dFont[size], currentX, currentY, charColor, size, rawChar.c_str());

            if (activeUrlIndex != -1 && charLen > 0) {
                vita2d_draw_rectangle(currentX, currentY + 2, itemWidth, 1, charColor);
            }

            currentX += itemWidth;
        }

        i += charLen;
    }
}

VitaGUI::VitaGUI(){
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));
	//pgf = vita2d_load_default_pgf();
	
	//pgf = vita2d_load_custom_pgf("app0:assets/font/seguiemj.pgf");
	//vita2dFontSymbola = vita2d_load_font_file("app0:assets/font/symbola.ttf");
	//vita2dFontSymbolaHint = vita2d_load_font_file("app0:assets/font/symbolahint.ttf");
	//vita2dFontSeguiemEmoji = vita2d_load_font_file("app0:assets/font/seguiemj.ttf");
	//vita2dFontLastResort = vita2d_load_font_file("app0:assets/font/lastresort.ttf");
	//vita2dFontOpenSansEmoji = vita2d_load_font_file("app0:assets/font/opensansemoji.ttf");
	
	for(int f = 0; f < MAX_FONT_SIZE ; f++){
		vita2dFont[f] = vita2d_load_font_file("app0:assets/font/whitney-book.ttf"); //droidsans.ttf
	}
	
	vita2dFontSmall = vita2d_load_font_file("app0:assets/font/whitney-book.ttf");
	vita2dFontNormal = vita2d_load_font_file("app0:assets/font/whitney-book.ttf");
	vita2dFontBig = vita2d_load_font_file("app0:assets/font/whitney-book.ttf");
	
	texA = vita2d_load_PNG_file("app0:assets/1f64b.png");
	texB = vita2d_load_PNG_file("app0:assets/1f64c.png");
	texC = vita2d_load_PNG_file("app0:assets/1f64d.png");
	
	
	std::string bgPath = "app0:assets/images/Vitacord-Background-8BIT.png";
	backgroundImage = vita2d_load_PNG_file(bgPath.c_str());
	loginFormImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-LoginForm-8BIT.png");
	loadingImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-Loading-8BIT.png");
	guildsBGImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-GuildsBG-8BIT.png");
	dmIconImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-DMIcon-8BIT.png");
	userIconDefaultImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-USERIcondefault-8BIT.png");
	statbarIconImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-statbar-icon.png");
	statbarBatteryImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-statbar-battery.png");
	statbarBatteryChargeImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-statbar-battery-charge.png");
	sidepanelStateIconImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-sidebar-default-usericon.png");
	messageInputImage = vita2d_load_PNG_file("app0:assets/images/Vitacord-messager-input.png");
	defaultBinaryThumbnail = vita2d_load_PNG_file("app0:assets/images/BinaryFile.png");
	
	loginInputs.clear();
	
	inputbox emailI;
	emailI.x = 431;
	emailI.y = 139;
	emailI.w = 375;
	emailI.h = 50;
	loginInputs.push_back(emailI);
	
	inputbox loginI;
	loginI.x = 449;
	loginI.y = 335;
	loginI.w = 349;
	loginI.h = 58;
	loginInputs.push_back(loginI);
	
	loginTexts.clear();
	loginTexts.push_back(" ");
	loginTexts.push_back(" ");
	loginTexts.push_back(" ");
	
	
	inputboxMessageInput.x = 230;
	inputboxMessageInput.y = 473;
	inputboxMessageInput.w = 730;
	inputboxMessageInput.h = 71;
	
	// L8R
	
	std::string filename = "app0:assets/images/loading/thumb0";
	std::string completeName = "";
	for(int fraL = 0 ; fraL < FRAMES_LOADING_IMAGE ; fraL++){
		if(fraL < 9){
			completeName = filename + "0"+ std::to_string(fraL+1) + ".png";
			loadingAnim[fraL] = vita2d_load_PNG_file(completeName.c_str());
		}else{
			completeName = filename + std::to_string(fraL+1) + ".png";
			loadingAnim[fraL] = vita2d_load_PNG_file(completeName.c_str());
		}

	}
	
	
	
	loadEmojiFiles();
	
	
	
}
void VitaGUI::loadEmojiFiles(){

}

void* VitaGUI::downloadImageWrapper(void* arg) {
    debugNetPrintf(DEBUG, "[WRAPPER] Avviato. Puntatore arg: %p\n", arg);
    
    DownloadImageArgs* args = reinterpret_cast<DownloadImageArgs*>(arg);
    debugNetPrintf(DEBUG, "[WRAPPER] Cast completato. Chiamo downloadImageThread...\n");
    
    args->guiPtr->downloadImageThread(args);
    
    debugNetPrintf(DEBUG, "[WRAPPER] Ritorno da downloadImageThread. Il thread sta per morire.\n");
    return nullptr;
}

void VitaGUI::downloadImageThread(DownloadImageArgs* args) {
    if (args == nullptr || args->filename.length() < 4) {
        if (args != nullptr) delete args;
        return;
    }

    std::string safeName = args->filename;
    for(auto &c : safeName) if(c == '?' || c == '&' || c == '=' || c == '/') c = '_';
    
    std::string tempPath = "ux0:data/" + safeName;

    // --- IL TRUCCO MAGICO PER LA GALLERIA ---
    // Chiediamo esplicitamente a Discord di transcodificare l'immagine 
    // in un JPEG standard, togliendo metadati strani o formati WebP mascherati.
    std::string downloadUrl = args->url;
    if (downloadUrl.find('?') != std::string::npos) {
        downloadUrl += "&format=jpeg";
    } else {
        downloadUrl += "?format=jpeg";
    }

    // 1. DOWNLOAD IN DATA (usando l'URL transcodificato)
    pthread_mutex_lock(&Discord::networkMutex);
    VitaNet::http_response resp = args->discordPtr->vitaNet.curlDiscordDownloadImage(downloadUrl, args->discordPtr->token, tempPath);
    pthread_mutex_unlock(&Discord::networkMutex);

    if (resp.httpcode == 200 || resp.httpcode == 204) {
        
        sceIoSync("ux0:", 0);
        sceKernelDelayThread(100 * 1000); 

        PhotoExportParam param;
        memset(&param, 0, sizeof(PhotoExportParam));
        char outPath[1024]; 
        memset(outPath, 0, sizeof(outPath));

        void* working_mem = memalign(4096, 512 * 1024); 

        if (working_mem) {
            
            // LA CHIAVE DI TUTTO: CARICHIAMO IL MODULO IN RAM!
            sceSysmoduleLoadModule(SCE_SYSMODULE_PHOTO_EXPORT);
            
            // Ora la funzione esiste in memoria e non salterà nel vuoto
            int res = scePhotoExportFromFile(tempPath.c_str(), &param, working_mem, nullptr, nullptr, outPath, sizeof(outPath));
            
            // SPEGNIAMO IL MODULO PER LIBERARE RAM
            sceSysmoduleUnloadModule(SCE_SYSMODULE_PHOTO_EXPORT);

            pthread_mutex_lock(&uiNotificationMutex);
            if (res >= 0) {
                this->downloadNotificationText = "Aggiunto in Galleria!";
            } else {
                this->downloadNotificationText = "Errore API Sony: " + std::to_string(res);
            }
            this->showDownloadNotification = true;
            this->notificationTimer = 180;
            pthread_mutex_unlock(&uiNotificationMutex);

            free(working_mem);
        }
    } else {
        pthread_mutex_lock(&uiNotificationMutex);
        this->downloadNotificationText = "Errore Download: " + std::to_string(resp.httpcode);
        this->showDownloadNotification = true;
        this->notificationTimer = 180;
        pthread_mutex_unlock(&uiNotificationMutex);
    }

    // PULIZIA FINALE
    std::string originalUrl = args->url; // Usiamo l'url originale per rimuoverlo dalla mappa
    delete args;

    pthread_mutex_lock(&downloadMutex);
    activeDownloads.erase(originalUrl);
    pthread_mutex_unlock(&downloadMutex);
}


VitaGUI::~VitaGUI(){
	vita2d_fini();
	vita2d_free_texture(backgroundImage);
	vita2d_free_texture(loginFormImage);
	vita2d_free_texture(loadingImage);
	vita2d_free_font(vita2dFontSmall);
	vita2d_free_font(vita2dFontNormal);
	vita2d_free_font(vita2dFontBig);
	//vita2d_free_pgf(pgf);
}
void VitaGUI::updateBoxes(){
	
	
}


void VitaGUI::DrawStatusBar() {
	
	vita2d_draw_rectangle(0, 0, 960, 30, RGBA8(174, 85, 44, 255));
	vita2d_draw_texture(statbarIconImage, 10, 7);
	
	// Battery
	float batteryX = (949 - vita2d_texture_get_width(statbarBatteryImage));
	float percent = scePowerGetBatteryLifePercent();
	float width = ((29.0f * percent) / 100.0f);
	
	if (scePowerIsBatteryCharging()) {
		batteryChargeCycle += 0.1;
		if (batteryChargeCycle > width)
			batteryChargeCycle = 0;
		float min = min(width, batteryChargeCycle);
		vita2d_draw_rectangle((947.0f - min), 6, min, 16, RGBA8(91, 223, 38, 255));
		vita2d_draw_texture(statbarBatteryChargeImage, batteryX, 4);
		}
	else {
		if (scePowerIsLowBattery())
			vita2d_draw_rectangle((947.0f - width), 6, width, 16, RGBA8(255, 48, 48, 255));
		else
			vita2d_draw_rectangle((947.0f - width), 6, width, 16, RGBA8(91, 223, 38, 255));
		vita2d_draw_texture(statbarBatteryImage, batteryX, 4);
		}

	pthread_mutex_lock(&uiNotificationMutex);
	if (showDownloadNotification && notificationTimer > 0) {
		int textWidth = vita2d_font_text_width(vita2dFont[20], 20, downloadNotificationText.c_str());
		vita2d_font_draw_text(vita2dFont[20], (960 - textWidth) / 2, 22, RGBA8(0, 255, 0, 255), 20, downloadNotificationText.c_str());
		notificationTimer--;
	} else if (notificationTimer <= 0) {
		showDownloadNotification = false;
	}
	pthread_mutex_unlock(&uiNotificationMutex);
	
	// Date & time
	SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	char dateString[16];
	char timeString[24];
	char string[64];
	sprintf(dateString, "%02d/%02d/%04d", time.day, time.month, time.year);
	sprintf(timeString, "%02d:%02d %s", (time.hour > 12) ? (time.hour - 12) : ((time.hour == 0) ? 12 : time.hour), time.minute, time.hour >= 12 ? "PM" : "AM");
	sprintf(string, "%s  %s", dateString, timeString);
	vita2d_font_draw_text(vita2dFontSmall, 875 - vita2d_font_text_width(vita2dFontSmall, 20, string), 22, RGBA8(255, 255, 255, 255), 20, string);
	}

unsigned int currentEmojiIndexTest = 0;
unsigned int testFrames = 0;

void VitaGUI::Draw(){
	
	//COMMENT debugNetPrintf(DEBUG, "Draw()\n");

	if(state == 2){
		
		//COMMENT debugNetPrintf(DEBUG, "Call SetGuildBoxes()\n");
		setGuildBoxes();
	} else if(state == 3){
		//COMMENT debugNetPrintf(DEBUG, "Call SetChannelBoxes()\n");
		setChannelBoxes();
	}else if(state == 4){
		//COMMENT debugNetPrintf(DEBUG, "Call SetChannel+MessageBoxes()\n");
		setChannelBoxes();
		setMessageBoxes();
	}else if(state == 6){
		
		//COMMENT debugNetPrintf(DEBUG, "Call SetDMBoxes()\n");
		setDirectMessageBoxes();
	}else if(state == 7){
		
		//COMMENT debugNetPrintf(DEBUG, "Call SetDM+MsgBoxes()\n");
		setDirectMessageBoxes();
		setDirectMessageMessagesBoxes();
	}
		
		//COMMENT debugNetPrintf(DEBUG, "All boxes set!\n");
	
	
	vita2d_start_drawing();
	vita2d_clear_screen();
	
	
	if(state == 0){
		
		DrawLoginScreen();
		
		/*if(texA != NULL){
			vita2d_draw_texture( texA , 0 , 72);
		}
		if(texB != NULL){
			vita2d_draw_texture( texB , 72 , 72);
		}
		if(texC != NULL){
			vita2d_draw_texture( texC , 144 , 72);
		}*/
		
		
	}else if(state == 1){
		// BG
		
		
		

		
		framePassed++;
		if(framePassed >= 2){
			
			loadingImageFrame++;
			framePassed = 0;
		}
		
		if(loadingImageFrame >= FRAMES_LOADING_IMAGE ){
			loadingImageFrame = 0;
		}else if(loadingImageFrame < 0){
			loadingImageFrame = 0;
		}
		if(loadingAnim[loadingImageFrame] != NULL){
			vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(39, 43, 47, 255));
			// frame of anim
			vita2d_draw_texture(loadingAnim[loadingImageFrame], loadingAnimX , loadingAnimY);
		
		}else{
			vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(95, 118, 198, 255));
			vita2d_draw_texture_rotate(loadingImage, 480 , 247, loadingImageAngle);
			loadingImageAngle += 0.25f;
		}
		// text
		vita2d_font_draw_text(vita2dFont[25] , 300, 355, RGBA8(255,255,255,255), 25, loadingString.c_str());
		

		
	}else if(state == 2){
		
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(54, 57, 62, 255)); // Background

		
		
		
		/// SIDEPANEL
		// BG
		vita2d_draw_rectangle(0, 30, 230, 449, RGBA8(46, 49, 54, 255));
		
		// GUILDS
		DrawGuildsOnSidebar();
		
		// TOP sidepanel to hide guilds underneath
		vita2d_draw_rectangle(0, 0, 230, 100, RGBA8(46, 49, 54, 255));
		vita2d_draw_rectangle(0, 99, 230, 1, RGBA8(5, 5, 6, 255));
		
		// BOTTOM SIDEPANEL
		vita2d_draw_rectangle(0, 479, 230, 1, RGBA8(5, 5, 6, 255));
		vita2d_draw_rectangle(0, 480, 230, 64, RGBA8(40, 43, 48, 255));
		vita2d_font_draw_text(vita2dFont[18], 14, 514, RGBA8(255, 255, 255, 255), 18, panelUsername.c_str());
		vita2d_font_draw_text(vita2dFont[15], 14, 530, RGBA8(255, 255, 255, 255), 15, panelUserDiscriminator.c_str()); // create a vita2dfont for each font-size or your font will get messed up.
		
		vita2d_draw_texture(dmIconImage, 166, 41); // DM ICON 
		
		
		// maybe add something on the big right 
		
		/// STATBAR
		DrawStatusBar();
		
	}else if(state == 3){
		
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(54, 57, 62, 255)); // Background
		
		
		
		/// SIDEPANEL
		// BG
		vita2d_draw_rectangle(0, 30, 230, 449, RGBA8(46, 49, 54, 255));
		
		// CHANNELS
		DrawChannelsOnSidebar();
		
		// TOP sidepanel to hide guilds underneath
		vita2d_draw_rectangle(0, 0, 230, 100, RGBA8(46, 49, 54, 255));
		vita2d_draw_rectangle(0, 99, 230, 1, RGBA8(5, 5, 6, 255));
		
		// BOTTOM SIDEPANEL
		vita2d_draw_rectangle(0, 479, 230, 1, RGBA8(5, 5, 6, 255));
		vita2d_draw_rectangle(0, 480, 230, 64, RGBA8(40, 43, 48, 255));
		vita2d_font_draw_text(vita2dFont[18], 14, 514, RGBA8(255, 255, 255, 255), 18, panelUsername.c_str());
		vita2d_font_draw_text(vita2dFont[15], 14, 530, RGBA8(255, 255, 255, 255), 15, panelUserDiscriminator.c_str()); // create a vita2dfont for each font-size or your font will get messed up.
		
		vita2d_draw_texture(dmIconImage, 166, 41); // DM ICON  
		
		
		// maybe add something on the big right 
		
		/// STATBAR
		DrawStatusBar();
		
	}else if(state == 4){
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(54, 57, 62, 255)); // Background
		
		
		/// SIDEPANEL
		// BG
		vita2d_draw_rectangle(0, 30, 230, 449, RGBA8(46, 49, 54, 255));
		
		
		
		//CHANNELS AND AFTER THAT MESSAGES
		
		DrawChannelsOnSidebar();
		
		// MESSAGES
		DrawMessages();
		
		
		// DELETEMSG
		if(clickedMessage){
			vita2d_draw_rectangle(editMessageBox.x , messageScrollY + editMessageBox.y , editMessageBox.w , editMessageBox.h , RGBA8(0 , 0 , 0 , 255));
			vita2d_font_draw_text(vita2dFont[25] , editMessageBox.x + 64 , messageScrollY + editMessageBox.y + editMessageBox.h - 8 , RGBA8(0,255,0,255), 25 , "Edit"); 
			vita2d_draw_rectangle(deleteMessageBox.x , messageScrollY + deleteMessageBox.y , deleteMessageBox.w , deleteMessageBox.h , RGBA8(0 , 0 , 0 , 255));
			vita2d_font_draw_text(vita2dFont[25] , deleteMessageBox.x + 64 , messageScrollY + deleteMessageBox.y + deleteMessageBox.h - 8, RGBA8(255,0,0,255), 25 , "Delete"); 
		}
		
		
		// TOP sidepanel to hide guilds underneath
		vita2d_draw_rectangle(0, 0, 230, 100, RGBA8(46, 49, 54, 255));
		vita2d_draw_rectangle(0, 99, 230, 1, RGBA8(5, 5, 6, 255));
		
		
		
		
		// BOTTOM SIDEPANEL
		vita2d_draw_rectangle(0, 479, 230, 1, RGBA8(5, 5, 6, 255));
		vita2d_draw_rectangle(0, 480, 230, 64, RGBA8(40, 43, 48, 255));
		vita2d_font_draw_text(vita2dFont[18], 14, 514, RGBA8(255, 255, 255, 255), 18, panelUsername.c_str());
		vita2d_font_draw_text(vita2dFont[15], 14, 530, RGBA8(255, 255, 255, 255), 15, panelUserDiscriminator.c_str()); // create a vita2dfont for each font-size or your font will get messed up.
		
		vita2d_draw_texture(dmIconImage, 166, 41); // DM ICON 
		
		
		// maybe add something on the big right 
		
		
		
		/// STATBAR
		DrawStatusBar();
		
		// MESSAGEINPUT
		vita2d_draw_texture(messageInputImage, 230, 473);
		

		
	}else if(state == 6){

		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(54, 57, 62, 255)); // Background
		
		
		/// SIDEPANEL
		// BG
		vita2d_draw_rectangle(0, 30, 230, 449, RGBA8(46, 49, 54, 255));
		
		DrawDirectMessageUsersOnSidebar();
		
		
		// TOP sidepanel to hide guilds underneath
		vita2d_draw_rectangle(0, 0, 230, 100, RGBA8(46, 49, 54, 255));
		vita2d_draw_rectangle(0, 99, 230, 1, RGBA8(5, 5, 6, 255));
		
		// BOTTOM SIDEPANEL
		vita2d_draw_rectangle(0, 479, 230, 1, RGBA8(5, 5, 6, 255));
		vita2d_draw_rectangle(0, 480, 230, 64, RGBA8(40, 43, 48, 255));
		vita2d_font_draw_text(vita2dFont[18], 14, 514, RGBA8(255, 255, 255, 255), 18, panelUsername.c_str());
		vita2d_font_draw_text(vita2dFont[15], 14, 530, RGBA8(255, 255, 255, 255), 15, panelUserDiscriminator.c_str()); // create a vita2dfont for each font-size or your font will get messed up.
		
		vita2d_draw_rectangle(146, 30, 84, 69, RGBA8(66, 70, 77, 225));
		vita2d_draw_texture(dmIconImage, 166, 41); // DM ICON 
		
		
		// maybe add something on the big right 
		
		/// STATBAR
		DrawStatusBar();
		
	}else if(state == 7){
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(54, 57, 62, 255)); // Background
			
			
		/// SIDEPANEL
		// BG
		vita2d_draw_rectangle(0, 30, 230, 449, RGBA8(46, 49, 54, 255));
		
		DrawDirectMessageUsersOnSidebar();
		
		DrawDirectMessageMessages();
		
		// TOP sidepanel to hide guilds underneath
		vita2d_draw_rectangle(0, 0, 230, 100, RGBA8(46, 49, 54, 255));
		vita2d_draw_rectangle(0, 99, 230, 1, RGBA8(5, 5, 6, 255));
		
		// BOTTOM SIDEPANEL
		vita2d_draw_rectangle(0, 479, 230, 1, RGBA8(5, 5, 6, 255));
		vita2d_draw_rectangle(0, 480, 230, 64, RGBA8(40, 43, 48, 255));
		vita2d_font_draw_text(vita2dFont[18], 14, 514, RGBA8(255, 255, 255, 255), 18, panelUsername.c_str());
		vita2d_font_draw_text(vita2dFont[15], 14, 530, RGBA8(255, 255, 255, 255), 15, panelUserDiscriminator.c_str()); // create a vita2dfont for each font-size or your font will get messed up.
		
		vita2d_draw_rectangle(146, 30, 84, 69, RGBA8(66, 70, 77, 225));
		vita2d_draw_texture(dmIconImage, 166, 41); // DM ICON 
		
		
		// maybe add something on the big right 
		
		/// STATBAR
		DrawStatusBar();
		
		
		// MESSAGEINPUT
		vita2d_draw_texture(messageInputImage, 230, 473);
		
	}else if(state == 9){
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(114, 137, 217, 255));
		unsigned int i = 0;
		for( i = 0; i < rectangles.size() ; i++){
			vita2d_draw_rectangle(rectangles[i].x, rectangles[i].y, rectangles[i].w, rectangles[i].h, rectangles[i].color);
		}
	}

	
	vita2d_end_drawing();
	vita2d_swap_buffers();
	vita2d_wait_rendering_done();
	
}

int VitaGUI::scroll(int x , int y , int posx , int posy){
	if(state == 2){
		if(posx < 230 && posx > 0 && posy < 522 && posy > 22){
			guildScrollX = 0;
			guildScrollY += y;
			if(guildScrollY < guildScrollYMin)
				guildScrollY = guildScrollYMin;
			else if(guildScrollY > guildScrollYMax )
				guildScrollY = guildScrollYMax;
			
		}
		

		
		return 0;
	}else if(state == 3){
		if(posx < 230 && posx > 0 && posy < 522 && posy > 22){
			channelScrollX = 0;
			channelScrollY += y;

			
			if(channelScrollY < channelScrollYMin)
				channelScrollY = channelScrollYMin;
			else if(channelScrollY > channelScrollYMax )
				channelScrollY = channelScrollYMax;
		}
		

		
		return 0;
	}else if(state==4){
		if(posx < 230 && posx > 0 && posy < 522 && posy > 22){
			channelScrollX = 0;
			channelScrollY += y;
			
			
			if(channelScrollY < channelScrollYMin)
				channelScrollY = channelScrollYMin;
			else if(channelScrollY > channelScrollYMax )
				channelScrollY = channelScrollYMax;
			


		}else if (posx > 230){
			messageScrollX = 0;
			messageScrollY += y;

			
			if(messageScrollY < messageScrollYMin)
				messageScrollY = messageScrollYMin;
			else if(messageScrollY > messageScrollYMax )
				messageScrollY = messageScrollYMax;
		}
		

		return 0;
	}else if(state==6){
		if(posx < 230 && posx > 0 && posy < 522 && posy > 22){
			directMessageScrollX = 0;
			directMessageScrollY += y;
			
			if(directMessageScrollY < directMessageScrollYMin)
				directMessageScrollY = directMessageScrollYMin;
			else if(directMessageScrollY > directMessageScrollYMax )
				directMessageScrollY = directMessageScrollYMax;
		}

		return 0;
	}else if(state==7){
		
		if(posx < 230 && posx > 0 && posy < 522 && posy > 22){
			directMessageScrollX = 0;
			directMessageScrollY += y;
			
			if(directMessageScrollY < directMessageScrollYMin)
				directMessageScrollY = directMessageScrollYMin;
			else if(directMessageScrollY > directMessageScrollYMax )
				directMessageScrollY = directMessageScrollYMax;
		}
		if(posx> 230){
			directMessageMessagesScrollX = 0;
			directMessageMessagesScrollY += y;
			
			
			if(directMessageMessagesScrollY < directMessageMessagesScrollYMin)
				directMessageMessagesScrollY = directMessageMessagesScrollYMin;
			else if(directMessageMessagesScrollY > directMessageMessagesScrollYMax )
				directMessageMessagesScrollY = directMessageMessagesScrollYMax;
		}
		
		return 0;
	}
	return -1;
}


int VitaGUI::analogScrollRight(int x , int y){
	
	
	if(state == 4){
		messageScrollX = 0;
		messageScrollY += y;

		if(messageScrollYMin > messageScrollYMax) messageScrollYMin = messageScrollYMax; // <--- AGGIUNGI QUESTA
		if(messageScrollY < messageScrollYMin)
			messageScrollY = messageScrollYMin;
		else if(messageScrollY > messageScrollYMax )
			messageScrollY = messageScrollYMax;
		
	}else if(state == 7){
		directMessageMessagesScrollX = 0;
		directMessageMessagesScrollY += y;
		
		if(directMessageScrollYMin > directMessageScrollYMax) directMessageScrollYMin = directMessageScrollYMax;
		if(directMessageMessagesScrollY < directMessageMessagesScrollYMin)
			directMessageMessagesScrollY = directMessageMessagesScrollYMin;
		else if(directMessageMessagesScrollY > directMessageMessagesScrollYMax )
			directMessageMessagesScrollY = directMessageMessagesScrollYMax;
	
	}
	return 1;
	
}

int VitaGUI::analogScrollLeft(int x , int y){
	
	
	if(state==0){
		// No scrollable components in login anymore
	}else if(state == 1){
		loadingAnimX += x;
		loadingAnimY -= y;
	}
	else if (state==2){
		guildScrollX = 0;
		guildScrollY += y;

		if(guildScrollYMin > guildScrollYMax) guildScrollYMin = guildScrollYMax;
		if(guildScrollY < guildScrollYMin)
			guildScrollY = guildScrollYMin;
		else if(guildScrollY > guildScrollYMax )
			guildScrollY = guildScrollYMax;
		
	}else if(state == 3){
		
		channelScrollX = 0;
		channelScrollY += y;

		if(channelScrollYMin > channelScrollYMax) channelScrollYMin = channelScrollYMax;
		if(channelScrollY < channelScrollYMin)
			channelScrollY = channelScrollYMin;
		else if(channelScrollY > channelScrollYMax )
			channelScrollY = channelScrollYMax;
	}else if(state == 4){
		channelScrollX = 0;
		channelScrollY += y;
		
		if(channelScrollYMin > channelScrollYMax) channelScrollYMin = channelScrollYMax;
		if(channelScrollY < channelScrollYMin)
			channelScrollY = channelScrollYMin;
		else if(channelScrollY > channelScrollYMax )
			channelScrollY = channelScrollYMax;
	}else if(state== 6){
		
		directMessageScrollX = 0;
		directMessageScrollY += y;
		
		if(directMessageScrollYMin > directMessageScrollYMax) directMessageScrollYMin = directMessageScrollYMax;
		if(directMessageScrollY < directMessageScrollYMin)
			directMessageScrollY = directMessageScrollYMin;
		else if(directMessageScrollY > directMessageScrollYMax )
			directMessageScrollY = directMessageScrollYMax;
		
	}else if(state == 7){
		directMessageScrollX = 0;
		directMessageScrollY += y;

		if(directMessageScrollYMin > directMessageScrollYMax) directMessageScrollYMin = directMessageScrollYMax;
		
		if(directMessageScrollY < directMessageScrollYMin)
			directMessageScrollY = directMessageScrollYMin;
		else if(directMessageScrollY > directMessageScrollYMax )
			directMessageScrollY = directMessageScrollYMax;
	}
	return 1;
	
}


int VitaGUI::click(int x , int y){
	if(state == 0){
		for(unsigned int i = 0 ; i < loginInputs.size() ; i++){
			if( x > loginInputs[i].x && x < loginInputs[i].x + loginInputs[i].w){
				if( y > loginInputs[i].y && y < loginInputs[i].y + loginInputs[i].h){
					return i;
				}
			}
		}
	}else if(state == 2){
		
		if(x > DMICONX && x < DMICONX2 && y > DMICONY && y < DMICONY2){
			return CLICKED_DM_ICON;
		}
		if( y < 515  &&  y > 99){
			for(unsigned int i = 0 ; i < guildBoxes.size() ; i++){
				if( x  > guildBoxes[i].x && x  < guildBoxes[i].x + guildBoxes[i].w){
					if( y > guildBoxes[i].y && y  < guildBoxes[i].y + guildBoxes[i].h){
						return guildBoxes[i].guildIndex;
					}
				}
			}
		}
		
	}else if(state == 3){
		
		if(x > DMICONX && x < DMICONX2 && y > DMICONY && y < DMICONY2){
			return CLICKED_DM_ICON;
		}
		
		if( y < 515  &&  y > 99){
			for(unsigned int i = 0 ; i < channelBoxes.size() ; i++){
				if( x  > channelBoxes[i].x && x  < channelBoxes[i].x + channelBoxes[i].w){
					if( y  > channelBoxes[i].y && y  < channelBoxes[i].y + channelBoxes[i].h){
						return channelBoxes[i].channelIndex;
					}
				}
			}
		}
	}else if(state == 4){
		
		
		if(x > DMICONX && x < DMICONX2 && y > DMICONY && y < DMICONY2){
			return CLICKED_DM_ICON;
		}
		
		if( x > inputboxMessageInput.x && y < inputboxMessageInput.x + inputboxMessageInput.w){
			if( y > inputboxMessageInput.y && y < inputboxMessageInput.y + inputboxMessageInput.h){
				return CLICKED_MESSAGE_INPUT;
			}
		}
		
		if( y < 515  &&  y > 99 && x < 230){
			for(unsigned int i = 0 ; i < channelBoxes.size() ; i++){
				if( x  > channelBoxes[i].x && x  < channelBoxes[i].x + channelBoxes[i].w){
					if( y  > channelBoxes[i].y && y  < channelBoxes[i].y + channelBoxes[i].h){
						return channelBoxes[i].channelIndex;
					}
				}
			}
		}
		
		if(clickedMessage){
			if( x > deleteMessageBox.x && x < deleteMessageBox.x + deleteMessageBox.w){
				if( y > messageScrollY + deleteMessageBox.y && y < messageScrollY + deleteMessageBox.y + deleteMessageBox.h){
					debugNetPrintf(DEBUG,"DELETING MESSAGE!\n");
					discordPtr->deleteMessage(deleteMessageBox.channelID , deleteMessageBox.messageID);
					debugNetPrintf(DEBUG,"DELETED MESSAGE!!!\n");
					clickedMessage = false;
					
					return -1;
				}
				
			}
			if( x > editMessageBox.x && x < editMessageBox.x + editMessageBox.w){
				if( y > messageScrollY + editMessageBox.y && y < messageScrollY + editMessageBox.y + editMessageBox.h){
					debugNetPrintf(DEBUG,"DELETING MESSAGE!\n");
					std::string newContent = vitaIMEPtr->getUserText((char*)"Edit message" , (char*) editMessageBox.content.c_str());
					discordPtr->editMessage(editMessageBox.channelID , editMessageBox.messageID , newContent);
					debugNetPrintf(DEBUG,"DELETED MESSAGE!!!\n");
					clickedMessage = false;
					
					return -1;
				}
				
			}
			
		}

		
		if( y < 515  &&  y > 30 && x > 230){
			for(unsigned int i = 0 ; i < messageBoxes.size() ; i++){
				if( messageScrollX + x  > messageBoxes[i].x && messageScrollX + x  < messageBoxes[i].x + messageBoxes[i].w){
					if( (-messageScrollY) + y  > messageBoxes[i].y && (-messageScrollY) + y  < messageBoxes[i].y + messageBoxes[i].h){
						
						// Check URLs
						for (auto& url : messageBoxes[i].urls) {
							for (auto& box : url.boxes) {
								if (x > box.x && x < box.x + box.w && y > box.y && y < box.y + box.h) {
									// TODO: Handle URL click
									debugNetPrintf(DEBUG, "Clicked URL: %s\n", url.url.c_str());
									return -1;
								}
							}
						}

						// Check Attachment
						if (messageBoxes[i].showAttachmentAsImage || messageBoxes[i].showAttachmentAsBinary) {
							if (x > messageBoxes[i].attachmentBox.x && x < messageBoxes[i].attachmentBox.x + messageBoxes[i].attachmentBox.w &&
								y > messageBoxes[i].attachmentBox.y && y < messageBoxes[i].attachmentBox.y + messageBoxes[i].attachmentBox.h) {
								if (messageBoxes[i].attachmentUrl != "") {

									// =================================================================
									// 🛡️ FIREWALL: VALIDAZIONE URL ED ERROR HANDLING
									// =================================================================
									std::string targetUrl = messageBoxes[i].attachmentUrl;
									
									// Controlla se l'URL è troppo corto o non è un link http/https valido
									if (targetUrl.length() < 4 || (targetUrl.substr(0, 4) != "http" && targetUrl.substr(0, 4) != "HTTP")) {
										
										// Logghiamo l'errore in console
										debugNetPrintf(DEBUG, "[FIREWALL] CRASH EVITATO! URL malformato: '%s'\n", targetUrl.c_str());
										
										// Notifichiamo l'utente visivamente
										pthread_mutex_lock(&uiNotificationMutex);
										this->downloadNotificationText = "Errore: URL Allegato Non Valido";
										this->showDownloadNotification = true;
										this->notificationTimer = 180;
										pthread_mutex_unlock(&uiNotificationMutex);
										
										return -1; // INTERROMPE IL CLICK: Niente thread = niente crash
									}
									// =================================================================

									if (messageBoxes[i].showAttachmentAsImage) {
										debugNetPrintf(DEBUG, "Clicked Attachment: %s\n", messageBoxes[i].attachmentFilename.c_str());

										pthread_mutex_lock(&downloadMutex);
										if (activeDownloads.find(messageBoxes[i].attachmentUrl) == activeDownloads.end()) {
											activeDownloads[messageBoxes[i].attachmentUrl] = true;
											pthread_mutex_unlock(&downloadMutex);

											DownloadImageArgs* args = new DownloadImageArgs();
											args->discordPtr = this->discordPtr;
											args->url = messageBoxes[i].attachmentUrl;
											args->filename = messageBoxes[i].attachmentFilename;
											args->guiPtr = this;

											// === FIX DEL CRASH (Aumentiamo lo stack del thread!) ===
											pthread_t downloadThread;
											pthread_attr_t attr;
											pthread_attr_init(&attr);
											
											// Diamo 256 KB di memoria al thread (fondamentale per cURL/SSL)
											pthread_attr_setstacksize(&attr, 2 * 1024 * 1024); 
											
											pthread_create(&downloadThread, &attr, &VitaGUI::downloadImageWrapper, args);
											
											pthread_attr_destroy(&attr); // Puliamo l'attributo
											pthread_detach(downloadThread);

											pthread_mutex_lock(&uiNotificationMutex);
											this->downloadNotificationText = "Download in corso...";
											this->showDownloadNotification = true;
											this->notificationTimer = 180;
											pthread_mutex_unlock(&uiNotificationMutex);
										} else {
											pthread_mutex_unlock(&downloadMutex);
										}
									}
									return -1;
								}
							}
						}

						if( clickedMessage ){
							debugNetPrintf(DEBUG , "un-clicked message\n");
							clickedMessage = false;
						}else{
							debugNetPrintf(DEBUG , "clicked message : \n");
							debugNetPrintf(DEBUG , messageBoxes[i].content.c_str());
							debugNetPrintf(DEBUG , " \n");
							clickedMessage = true;
							editMessageBox.w = 200;
							editMessageBox.h = 64;
							deleteMessageBox.w = 200;
							deleteMessageBox.h = 64;
							
							editMessageBox.y = messageBoxes[i].y + - 64;
							
							deleteMessageBox.y = messageBoxes[i].y + 8;//static_cast<int>( messageBoxes[i].y + (messageBoxes[i].h / 2 ) ); // "updating" height in draw call so => - messageScrollY
							if(x > 704){
								editMessageBox.x = 704;
								deleteMessageBox.x = 704;
							}else{
								editMessageBox.x = x;
								deleteMessageBox.x = x;
							}
							editMessageBox.messageID = messageBoxes[i].messageID;
							editMessageBox.channelID = messageBoxes[i].channelID;
							editMessageBox.content = messageBoxes[i].content;
							deleteMessageBox.messageID = messageBoxes[i].messageID;
							deleteMessageBox.channelID = messageBoxes[i].channelID;
						}
						
						return -1;
					}
				}
			}
		}
		
	}else if(state == 6){
		
		if(x > DMICONX && x < DMICONX2 && y > DMICONY && y < DMICONY2){
			return CLICKED_DM_ICON;
		}
		
		if( y < 515  &&  y > 99){
			for(unsigned int i = 0 ; i < directMessageBoxes.size() ; i++){
				if( x  > directMessageBoxes[i].x && x  < directMessageBoxes[i].x + directMessageBoxes[i].w){
					if( y  > directMessageBoxes[i].y && y  < directMessageBoxes[i].y + directMessageBoxes[i].h){
						return directMessageBoxes[i].dmIndex;
					}
				}
			}
		}
	}else if(state == 7){
		
		if(x > DMICONX && x < DMICONX2 && y > DMICONY && y < DMICONY2){
			return CLICKED_DM_ICON;
		}
		
		
		if( x > inputboxMessageInput.x && y < inputboxMessageInput.x + inputboxMessageInput.w){
			if( y > inputboxMessageInput.y && y < inputboxMessageInput.y + inputboxMessageInput.h){
				return CLICKED_MESSAGE_INPUT;
			}
		}
		
		if( y < 515  &&  y > 99){
			for(unsigned int i = 0 ; i < directMessageBoxes.size() ; i++){
				if( x  > directMessageBoxes[i].x && x  < directMessageBoxes[i].x + directMessageBoxes[i].w){
					if( y  > directMessageBoxes[i].y && y  < directMessageBoxes[i].y + directMessageBoxes[i].h){
						return directMessageBoxes[i].dmIndex;
					}
				}
			}
		}
		

		
		// ? messages

		if( y < 515  &&  y > 30 && x > 230){
			for(unsigned int i = 0 ; i < directMessageMessagesBoxes.size() ; i++){
				if( directMessageMessagesScrollX + x  > directMessageMessagesBoxes[i].x && directMessageMessagesScrollX + x  < directMessageMessagesBoxes[i].x + directMessageMessagesBoxes[i].w){
					if( (-directMessageMessagesScrollY) + y  > directMessageMessagesBoxes[i].y && (-directMessageMessagesScrollY) + y  < directMessageMessagesBoxes[i].y + directMessageMessagesBoxes[i].h){

						// Check URLs
						for (auto& url : directMessageMessagesBoxes[i].urls) {
							for (auto& box : url.boxes) {
								if (x > box.x && x < box.x + box.w && y > box.y && y < box.y + box.h) {
									// TODO: Handle URL click
									debugNetPrintf(DEBUG, "Clicked DM URL: %s\n", url.url.c_str());
									return -1;
								}
							}
						}

						// DM attachment checking could go here if DMs have attachments populated like normal messages
					}
				}
			}
		}

	}
	return -1;
}

void VitaGUI::AddRectangle(float nx , float ny , float nw , float nh , unsigned int ncolor){
	rectangle r;
	r.x = nx;
	r.y = ny;
	r.w = nw;
	r.h = nh;
	r.color = ncolor;
	rectangles.push_back(r);
}
void VitaGUI::RemoveRectangle(unsigned int index){
	
	if(index < rectangles.size()){
		rectangles.erase(rectangles.begin()+index);
	}
	
}

int VitaGUI::GetState(){
	return state;
}
void VitaGUI::SetState(int s){
	lastState = state;
	state = s;
	
	if(state == 4){
		messageScrollY = 0;
		directMessageMessagesScrollY = 0;
		directMessageScrollY = 0;
		messageScrollSet = false;
		setMessageBoxes();
	}else if(state == 7 && lastState == 6){
		messageScrollY = 0;
		directMessageMessagesScrollY = 0;
		setDirectMessageMessagesBoxes();
		directMessageMessagesScrollSet = false;
		
	}else if(lastState == 7 && state == 7){
		
		directMessageMessagesScrollY = 0;
		setDirectMessageMessagesBoxes();
		directMessageMessagesScrollSet = false;
		
		
	}else{
		messageScrollY = 0;
		channelScrollY = 0;
		directMessageMessagesScrollY = 0;
		directMessageScrollY = 0;
	}
	

	
}
void VitaGUI::SetStateToLastState(){
	state = lastState;
}

void VitaGUI::passVITAIMEPointer( VitaIME * vmeptr ){
	vitaIMEPtr = vmeptr;
}

void VitaGUI::passDiscordPointer(Discord *ptr){
	discordPtr = ptr;
}
void VitaGUI::setGuildBoxes(){
	guildBoxes.clear();
	for(unsigned int i = 0; i < discordPtr->guilds.size() ; i++){
		guildBox boxG;
		boxG.x = guildScrollX ;
		boxG.y = 100 + guildScrollY + i * GUILD_HEIGHT;
		boxG.w = 230;
		boxG.h = GUILD_HEIGHT;
		boxG.guildIndex = i;
		boxG.name = discordPtr->guilds[i].name;
		guildBoxes.push_back(boxG);
	}
	guildScrollYMin = -((guildBoxes.size())*GUILD_HEIGHT - 100);
	guildScrollYMax = 0;
}
void VitaGUI::setChannelBoxes(){
	channelBoxes.clear();
	int amount = 0;
	for(unsigned int i = 0; i < discordPtr->guilds[discordPtr->currentGuild].channels.size() ; i++){
		if(discordPtr->guilds[discordPtr->currentGuild].channels[i].type == 0 && discordPtr->guilds[discordPtr->currentGuild].channels[i].readallowed){
			channelBox boxC;
			boxC.x = channelScrollX ;
			boxC.y = 100 + channelScrollY + amount * CHANNEL_HEIGHT;
			boxC.w = 230;
			boxC.h = CHANNEL_HEIGHT;
			boxC.channelIndex = i;
			boxC.name = discordPtr->guilds[discordPtr->currentGuild].channels[i].name;
			channelBoxes.push_back(boxC);
			amount++;
		}
	}
	channelScrollYMin = -((amount)*CHANNEL_HEIGHT - 100) ;
	channelScrollYMax = 0;
}
bool VitaGUI::setMessageBoxes(){
		
	int topMargin = 45;
	int bottomMargin = 18;
	int textHeight = 0;
	int allHeight = 0;
	if(!discordPtr->refreshingMessages && discordPtr->refreshedMessages){
		discordPtr->refreshedMessages = false;
		messageBoxes.clear();
		for(unsigned int i = 0; i < discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages.size() ; i++){
			
			messagebox boxC;
			
			boxC.messageID = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].id;
			boxC.x = messageScrollX + 230;
			boxC.y = 40  + allHeight ; // 40 = statusbar height
			boxC.username = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].author.username;
			boxC.userColor = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].author.color;

			// Resolve dynamically in case roles fetched after messages
			if(boxC.userColor == 0) {
				boxC.userColor = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].author.color;
			}

			boxC.mentionsMap = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].mentionsMap;
			// 1. Pulisci le menzioni
			// Cerca questo punto in VitaGUI::setMessageBoxes
			boxC.content = cleanMentions(discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].content, boxC.mentionsMap);
			boxC.mentionsMap.clear();

        // 1. Esegui il wordWrap intelligente (usiamo 640 pixel di maxWidth)
        	int numLines = wordWrap(boxC.content, 640, boxC.content); 
			textHeight = numLines * (32 + 4) + topMargin + bottomMargin; 
        
        	boxC.messageHeight = max(64, textHeight); // 64 è l'altezza minima per i messaggi con poco testo, per evitare che siano troppo piccoli
			boxC.urls = parseUrls(boxC.content);
			
			
			boxC.channelID = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].id;

			// ATTACHMENTS
			if( ! discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.isEmpty ){
				if(discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.isData ){
					boxC.showAttachmentAsBinary = true;
					boxC.showAttachmentAsImage = false;
					//boxC.attachmentThumbnail =  defaultBinaryThumbnail;// default thumbnail ( txt or binary indicator )  . ACTUALLY not gonna do this for now
					boxC.attachmentReadableSize = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.readableSize;
					boxC.attachmentReadableSizeUnit = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.readableSizeUnit;
					boxC.attachmentFilename = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.filename;
					boxC.attachmentUrl = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.url;
					boxC.attachmentFullText = std::to_string(  boxC.attachmentReadableSize ) + " " +  boxC.attachmentReadableSizeUnit + " " + boxC.attachmentFilename;
					
					// adjust box height !
					boxC.messageHeight += 60; // 16 = margin
				}else if ( discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.isImage ){
					if( discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.loadedThumbImage ){
						boxC.showAttachmentAsImage = true;
						boxC.showAttachmentAsBinary = false;
						//boxC.attachmentThumbnail = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.thumbnail;
						boxC.attachmentReadableSize = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.readableSize;
						boxC.attachmentReadableSizeUnit = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.readableSizeUnit;
						boxC.attachmentFilename = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.filename;
						boxC.attachmentUrl = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].attachment.url;
						boxC.attachmentFullText = std::to_string(  boxC.attachmentReadableSize ) + " " +  boxC.attachmentReadableSizeUnit + " " + boxC.attachmentFilename;
					
						// adjust box height !
						boxC.messageHeight += 60;
					}
				}else{
					boxC.showAttachmentAsImage = false;
					boxC.showAttachmentAsBinary = false;
				}
			}else{
				boxC.showAttachmentAsImage = false;
				boxC.showAttachmentAsBinary = false;
			}
			boxC.w = 730;
			boxC.h = boxC.messageHeight;
			
			// EMOJIS:
			boxC.emojis.clear();
			for(unsigned int e = 0; e < discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].emojis.size() ; e++){
				boxC.emojis.push_back(m_emoji());
				boxC.emojis[e].posX = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].emojis[e].posX;
				boxC.emojis[e].posY = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].emojis[e].posY;
				boxC.emojis[e].spriteSheetX = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].emojis[e].spriteSheetX;
				boxC.emojis[e].spriteSheetY = discordPtr->guilds[discordPtr->currentGuild].channels[discordPtr->currentChannel].messages[i].emojis[e].spriteSheetY;
			}
			
			
			allHeight += boxC.messageHeight;
			messageBoxes.push_back(boxC);
		}
		messageScrollYMin =  -( allHeight )  + 430; 
		messageScrollYMax = 0;
		
		if( !messageScrollSet ){
			messageScrollSet = true;
			messageScrollY = messageScrollYMin;
		}
		
		return true;
	}
	return false;
}

int VitaGUI::wordWrap(std::string str, unsigned int maxWidthPixels, std::string &out) {
    out = "";
    if (str.empty()) return 0;

    int currentLineWidth = 0;
    std::string currentWord = "";
    bool inUrl = false;
    int fontSize = 32;

    // === FASE 1: FORMATTAZIONE (Protegge i link, manda a capo le parole) ===
    auto flushWord = [&]() {
        if (currentWord.empty()) return;

        int wordWidth = 0;
        for (size_t j = 0; j < currentWord.length(); ) {
            unsigned char cw = currentWord[j];
            size_t charLen = 1;
            if (cw <= 0x7F) charLen = 1;
            else if ((cw & 0xE0) == 0xC0) charLen = 2;
            else if ((cw & 0xF0) == 0xE0) charLen = 3;
            else if ((cw & 0xF8) == 0xF0) charLen = 4;
            
            std::string rawChar = currentWord.substr(j, charLen);
            int charW = vita2d_font_text_width(vita2dFont[fontSize], fontSize, rawChar.c_str());
            
            // Stima l'ingombro delle emoji se la libreria fallisce
            if (charW <= 0 && charLen >= 3) charW = 32; 
            
            wordWidth += charW;
            j += charLen;
        }

        // Manda a capo SOLO se NON è un link
        if (currentLineWidth + wordWidth > maxWidthPixels && currentLineWidth > 0 && !inUrl) {
            out += "\n";
            currentLineWidth = 0;
        }
        
        out += currentWord;
        currentLineWidth += wordWidth;
        currentWord = "";
        inUrl = false;
    };

    for (size_t k = 0; k < str.length(); ) {
        unsigned char c = (unsigned char)str[k];
        size_t charLen = 1;

        if (c <= 0x7F) charLen = 1;
        else if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;

        std::string rawChar = str.substr(k, charLen);

        // Identifica se siamo dentro un link
        if (!inUrl && currentWord.empty() && str.length() - k >= 4 && str.substr(k, 4) == "http") {
            inUrl = true;
        }

        if (c == '\n') {
            flushWord();
            out += "\n";
            currentLineWidth = 0;
        } 
        else if (c == ' ' || c == '\t') {
            flushWord();
            out += rawChar;
            int spaceW = vita2d_font_text_width(vita2dFont[fontSize], fontSize, rawChar.c_str());
            currentLineWidth += spaceW;
        } 
        else {
            currentWord += rawChar;
        }
        k += charLen;
    }
    flushWord(); 

    // === FASE 2: SIMULATORE DI SCHERMO ===
    // Ora leggiamo la stringa finale esattamente come farà DrawTextWithEmojis
    // e contiamo i "tagli" forzati per sapere il NUMERO REALE di righe.
    
    int visualLines = 1;
    int currentX = 0;

    for (size_t i = 0; i < out.length(); ) {
        unsigned char c = (unsigned char)out[i];
        size_t charLen = 1;
        
        if (c == '\n') {
            currentX = 0;
            visualLines++;
            i++;
            continue;
        }

        if (c <= 0x7F) charLen = 1;
        else if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;

        std::string rawChar = out.substr(i, charLen);
        int itemWidth = vita2d_font_text_width(vita2dFont[fontSize], fontSize, rawChar.c_str());
        if (itemWidth <= 0 && charLen >= 3) itemWidth = 32;

        // Se una lettera fa sbordare (es. un link lunghissimo), DrawTextWithEmojis la manderà a capo.
        // Noi facciamo lo stesso conto in anticipo!
        if (currentX + itemWidth > maxWidthPixels) {
            currentX = 0;
            visualLines++;
        }
        currentX += itemWidth;
        i += charLen;
    }

    return visualLines; // Ritorna le righe ESATTE per calcolare l'altezza!
}



void VitaGUI::setDirectMessageBoxes(){
	directMessageBoxes.clear();
	for(unsigned int i = 0; i < discordPtr->directMessages.size() ; i++){
		dmBox boxDM;
		boxDM.x = directMessageScrollX ;
		boxDM.y = 100 + directMessageScrollY + i * GUILD_HEIGHT;
		boxDM.w = 230;
		boxDM.h = GUILD_HEIGHT;
		boxDM.name = discordPtr->directMessages[i].recipients[0].username;
		boxDM.dmIndex = i;
		directMessageBoxes.push_back(boxDM);
	}
	directMessageMessagesScrollYMin = -((directMessageBoxes.size())*CHANNEL_HEIGHT - 100) ;
	directMessageMessagesScrollYMax = 0;
	//directMessageScrollYMin = -((discordPtr->directMessages.size()-1)*GUILD_HEIGHT -300) ;
	
}

void VitaGUI::setDirectMessageMessagesBoxes(){
	
	int topMargin = 45;
	int bottomMargin = 18;
	int textHeight = 0;
	int allHeight = 0;
	// TODO CHECK FOR REFRESHED AND REFRESHING DMSGS + PTHREAD THE refreshDm() in Discord.cpp
	//if(!discordPtr->refreshingMessages && discordPtr->refreshedMessages){
		//discordPtr->refreshedMessages = false;
		directMessageMessagesBoxes.clear();
		for(unsigned int i = 0; i < discordPtr->directMessages[discordPtr->currentDirectMessage].messages.size() ; i++){
			messagebox boxC;
			boxC.x = directMessageMessagesScrollX + 230;
			boxC.y = directMessageMessagesScrollY + allHeight;
			boxC.username = discordPtr->directMessages[discordPtr->currentDirectMessage].messages[i].author.username;
			boxC.userColor = discordPtr->directMessages[discordPtr->currentDirectMessage].messages[i].author.color;
			
			boxC.mentionsMap = discordPtr->directMessages[discordPtr->currentDirectMessage].messages[i].mentionsMap;
			
			// 1. Pulisci le menzioni (CORREZIONE: Leggiamo da directMessages, NON da guilds!)
			std::string rawContent = discordPtr->directMessages[discordPtr->currentDirectMessage].messages[i].content;
			boxC.content = cleanMentions(rawContent, boxC.mentionsMap);
			boxC.mentionsMap.clear();

			// 2. Esegui il wordWrap intelligente in pixel
			int numLines = wordWrap(boxC.content, 640, boxC.content);

			// 3. Calcola l'altezza matematicamente
			textHeight = numLines * (32 + 4) + topMargin + bottomMargin; 
			boxC.messageHeight = max(64, textHeight);

			// 4. Solo ora calcola gli URL
			boxC.urls = parseUrls(boxC.content);
			
			// 5. Assegna larghezza e altezza finali al box
			boxC.w = 730;
			boxC.h = boxC.messageHeight;

			allHeight += boxC.messageHeight;
			directMessageMessagesBoxes.push_back(boxC);
		}
		directMessageMessagesScrollYMin =  -( allHeight - 100  ); //-( allHeight )
		directMessageMessagesScrollYMax =   0; 
		
		
		if(!directMessageMessagesScrollSet){
			directMessageMessagesScrollSet = true;
			directMessageMessagesScrollY = directMessageMessagesScrollYMin;
		}
		

	//}
	

	
}


void VitaGUI::setUserInfo(){
	
	panelUsername = discordPtr->username;
	panelUserDiscriminator = "#" + discordPtr->discriminator;
}


void VitaGUI::showLoginCue(){
	vita2d_start_drawing();
	vita2d_clear_screen();
	
	DrawLoginScreen();
	
	vita2d_draw_rectangle(455 , 335, 330 , 58 , RGBA8(0xFF , 0xFF , 0xFF , 0x24));
	
	vita2d_end_drawing();
	vita2d_swap_buffers();
	vita2d_wait_rendering_done();
}

void VitaGUI::unshowLoginCue(){
	
	vita2d_start_drawing();
	vita2d_clear_screen();
	
	DrawLoginScreen();
	
	vita2d_end_drawing();
	vita2d_swap_buffers();
	vita2d_wait_rendering_done();
}

void VitaGUI::DrawLoginScreen(){
	
	
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(114, 137, 217, 255));
		vita2d_draw_texture( loginFormImage , 161, 53 );
		// Cover the EMAIL and PASSWORD areas with the background color
		vita2d_draw_rectangle( 430, 110, 380, 200, RGBA8(54, 57, 63, 255) );
		vita2d_font_draw_text(vita2dFont[18] , 438, 130, RGBA8(130, 134, 142, 255), 18, "TOKEN");
		vita2d_font_draw_text(vita2dFont[18] , 438, 181, RGBA8(255,255,255,255), 18, loginTexts[0].c_str());
		vita2d_draw_rectangle( 438, 190, 360, 2, RGBA8(130, 134, 142, 255) );
		vita2d_font_draw_text(vita2dFont[18] , 750, 261, RGBA8(255,0,0,255), 18, loginTexts[2].c_str());
	
}

void VitaGUI::DrawGuildsOnSidebar(){
	int height = 0;
	for(unsigned int i = 0 ; i < guildBoxes.size() ; i++){
		height = guildScrollY + i * GUILD_HEIGHT;
		if(height < MAX_DRAW_HEIGHT && height  > MIN_DRAW_HEIGHT){
			vita2d_font_draw_text(vita2dFont[GUILD_TITLE_TEXT_SIZE_PIXEL] , guildScrollX + 8, 100 + guildScrollY + i * GUILD_HEIGHT + 40, RGBA8(255,255,255,255), GUILD_TITLE_TEXT_SIZE_PIXEL, guildBoxes[i].name.c_str());
			
		}
	}
}

void VitaGUI::DrawChannelsOnSidebar(){
	int channelsAmount = static_cast<int>(channelBoxes.size());
	for(int i = 0 ; i < channelsAmount ; i++){
		if(channelScrollY + i * CHANNEL_HEIGHT < MAX_DRAW_HEIGHT && channelScrollY + i * CHANNEL_HEIGHT > MIN_DRAW_HEIGHT){
			if(discordPtr->currentChannel == channelBoxes[i].channelIndex && discordPtr->inChannel){
				vita2d_draw_rectangle(channelScrollX , 100 + channelScrollY + i * CHANNEL_HEIGHT, 215 , CHANNEL_HEIGHT, RGBA8(40, 43, 48, 255));
				vita2d_draw_rectangle(channelScrollX , 100 + channelScrollY + i * CHANNEL_HEIGHT, 4 , CHANNEL_HEIGHT, RGBA8(95, 118, 198, 255));
			}
			vita2d_font_draw_text(vita2dFont[CHANNEL_TITLE_TEXT_SIZE_PIXEL] , channelScrollX + 8, 100 + channelScrollY + i * CHANNEL_HEIGHT + 42  , RGBA8(255,255,255,255), CHANNEL_TITLE_TEXT_SIZE_PIXEL, channelBoxes[i].name.c_str());
		}
	}
	
}

void VitaGUI::DrawMessages(){
	
	int yPos = messageScrollY + 40 , height;
	unsigned int messageBoxesAmount = messageBoxes.size();
	
	for(unsigned int i =  0 ; i < messageBoxesAmount ; i++){
		
		height = messageBoxes[i].messageHeight;
		
		if(yPos < MAX_DRAW_HEIGHT && yPos > MIN_DRAW_HEIGHT){
			
			
			if(i == messageBoxesAmount-1){
				
				vita2d_draw_rectangle(242, yPos + height - 1, 706, 1, RGBA8(120, 115, 120, 255)); 
				vita2d_draw_rectangle(240, yPos + height , 710, 1, RGBA8(100, 100, 100, 255)); 
				vita2d_draw_rectangle(242, yPos + height + 1, 706, 1, RGBA8(120, 115, 120, 255)); 
			}else{
				vita2d_draw_rectangle(242, yPos + height - 1, 706, 1, RGBA8(62, 65, 70, 255));
				vita2d_draw_rectangle(240, yPos + height, 710, 1, RGBA8(51, 53, 55, 255)); 
				vita2d_draw_rectangle(242, yPos + height + 1, 706, 1, RGBA8(62, 65, 70, 255)); 
				
			}
			

				vita2d_font_draw_text(vita2dFont[26], 283, yPos + 26, messageBoxes[i].userColor != 0 ? messageBoxes[i].userColor : RGBA8(255, 255, 255, 255), 26, messageBoxes[i].username.c_str());

				// Clear previous dynamic boxes before redraw
				for (auto& url : messageBoxes[i].urls) {
					url.boxes.clear();
				}
				DrawTextWithEmojis(messageBoxes[i].content, 293, yPos + 60, 32, 650, &(messageBoxes[i].urls));
				 
			if( messageBoxes[i].showAttachmentAsImage || messageBoxes[i].showAttachmentAsBinary ){
				std::string attText = messageBoxes[i].showAttachmentAsImage ? "[ 📷 Immagine ]" : "[ 📎 Allegato ]";

				// Draw button background and border
				int btnX = 243;
				int btnY = yPos + height - 50;
				int btnW = 250;
				int btnH = 45;

				// Border
				vita2d_draw_rectangle(btnX, btnY, btnW, btnH, RGBA8(62, 65, 70, 255));
				// Background
				vita2d_draw_rectangle(btnX + 1, btnY + 1, btnW - 2, btnH - 2, RGBA8(40, 43, 48, 255));

				// Draw Text (Centered approximately)
				vita2d_font_draw_text(vita2dFont[24], btnX + 10, btnY + 32, messageBoxes[i].userColor ? messageBoxes[i].userColor : RGBA8(255, 255, 255, 255), 24, attText.c_str());

				messageBoxes[i].attachmentBox.x = btnX;
				messageBoxes[i].attachmentBox.y = btnY;
				messageBoxes[i].attachmentBox.w = btnW;
				messageBoxes[i].attachmentBox.h = btnH;
			}

			// DRAW EMOJIS:
			// Old emoji draw code removed, now using inline DrawTextWithEmojis.
				
			// Not drawing default icons anymore.
		}

		
		yPos += height; // add message height to yPos
		
	}

		
}


void VitaGUI::DrawDirectMessageUsersOnSidebar(){
	
	
	int dmBoxes = static_cast<int>(directMessageBoxes.size());
	for(int i = 0 ; i < dmBoxes ; i++){
		if(directMessageScrollY + i * CHANNEL_HEIGHT < MAX_DRAW_HEIGHT && directMessageScrollY + i * CHANNEL_HEIGHT > MIN_DRAW_HEIGHT){
			
			if(discordPtr->currentDirectMessage == directMessageBoxes[i].dmIndex && discordPtr->inDirectMessageChannel){
				vita2d_draw_rectangle(directMessageScrollX , 100 + directMessageScrollY + i * CHANNEL_HEIGHT, 215 , CHANNEL_HEIGHT, RGBA8(40, 43, 48, 255));
				vita2d_draw_rectangle(directMessageScrollX , 100 + directMessageScrollY + i * CHANNEL_HEIGHT, 4 , CHANNEL_HEIGHT, RGBA8(95, 118, 198, 255));
			}

			vita2d_font_draw_text(vita2dFont[CHANNEL_TITLE_TEXT_SIZE_PIXEL] , directMessageScrollX + 8, 100 + directMessageScrollY + i * CHANNEL_HEIGHT + 42, RGBA8(255,255,255,255), CHANNEL_TITLE_TEXT_SIZE_PIXEL, directMessageBoxes[i].name.c_str());
			
		}
	}

	
}

void VitaGUI::DrawDirectMessageMessages(){

	
	int yPos = 0,height;
	unsigned int messageBoxesAmount = directMessageMessagesBoxes.size();

	yPos = directMessageMessagesScrollY + 40;
	for(unsigned int i =  0 ; i < messageBoxesAmount ; i++){
		
		height = directMessageMessagesBoxes[i].messageHeight;
		
		if(yPos < MAX_DRAW_HEIGHT && yPos > MIN_DRAW_HEIGHT){
			if(i == messageBoxesAmount-1){
				
				vita2d_draw_rectangle(242, yPos + height - 1, 706, 1, RGBA8(120, 115, 120, 255));
				vita2d_draw_rectangle(240, yPos + height , 710, 1, RGBA8(100, 100, 100, 255)); 
				vita2d_draw_rectangle(242, yPos + height + 1, 706, 1, RGBA8(120, 115, 120, 255)); 
			}else{
				
				vita2d_draw_rectangle(242, yPos + height - 1, 706, 1, RGBA8(62, 65, 70, 255)); 
				vita2d_draw_rectangle(240, yPos + height, 710, 1, RGBA8(51, 53, 55, 255));
				vita2d_draw_rectangle(242, yPos + height + 1, 706, 1, RGBA8(62, 65, 70, 255)); 
				
			}
			
				vita2d_font_draw_text(vita2dFont[15], 243, yPos + 26, directMessageMessagesBoxes[i].userColor != 0 ? directMessageMessagesBoxes[i].userColor : RGBA8(255, 255, 255, 255), 15, directMessageMessagesBoxes[i].username.c_str());

				// Clear previous dynamic boxes before redraw
				for (auto& url : directMessageMessagesBoxes[i].urls) {
					url.boxes.clear();
				}
				DrawTextWithEmojis(directMessageMessagesBoxes[i].content, 293, yPos + 60, 15, 650, &(directMessageMessagesBoxes[i].urls));

			
			// Not drawing default icons anymore.
			}

		
		yPos += height; // add message height to yPos
	}
	
}




