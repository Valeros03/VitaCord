#include "DiscordApp.hpp"
#include "log.hpp"
#include "easyencryptor.hpp"
#include <psp2/io/stat.h> 
#include <psp2/net/net.h>

#include <debugnet.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>

#include "key.h"


void DiscordApp::loadUserDataFromFile ( ) {
	debugNetPrintf(DEBUG , "loadUserDataFromFile \n");
	std::string parserString = "";
	bool oldEnc = false;
	std::string enckey = "Toastie";
	debugNetPrintf(DEBUG , "sceioopen");
	int fh = sceIoOpen ( "ux0:data/vitacord-userdata.txt", SCE_O_RDONLY , 0777 );

	if(fh >= 0){
		
		logSD ( "getfilesize" );
		int filesize = sceIoLseek ( fh, 0, SCE_SEEK_END );
		logSD( "filesize is : " + std::to_string ( filesize ) );
		logSD("sceioseekfront");
		sceIoLseek(fh, 0, SCE_SEEK_SET);
		logSD("char* buffer = malloc(filesize)");
		char* buffer = (char*)malloc(filesize);
		logSD("sceioread");
		int readbytes = sceIoRead(fh, buffer, filesize); 
		logSD("readbytes is : " + std::to_string(readbytes));
		logSD("sceioclose");
		sceIoClose( fh );
		sceIoRemove( "ux0:data/vitacord-userdata.txt" );
		
		if ( filesize < 4 || readbytes < 4 ) {
			debugNetPrintf(DEBUG , "oldFile too small or not existant \n");
			logSD( "file too small no settings loaded" );
			oldEnc = false;
		}else{
			debugNetPrintf(DEBUG , "oldFile existant \n");
			parserString = std::string( buffer , readbytes);
			oldEnc = true;
		}
		free(buffer);
	}
	
	logSD( "declare strings" );
	
	
	std::string email = ""  , password = "" , token = "";
	bool getmail= true , getpass = false, gettoken = false;
	unsigned int i = 0;
	logSD( "declare strings" );
	
	
	// new enc
	if ( !oldEnc ) {
		debugNetPrintf(DEBUG , "new loading!\n");
		int fhMail = sceIoOpen ( "ux0:data/vitacord/user/loc.ecr" , SCE_O_RDONLY , 0777 );
		int fileSize = sceIoLseek ( fhMail, 0, SCE_SEEK_END );
		sceIoLseek ( fhMail, 0, SCE_SEEK_SET );
		if ( fileSize >= 5 ) {
			char * bufferMail = ( char * ) malloc (fileSize);
			int readBytes = sceIoRead ( fhMail , bufferMail , fileSize );
			std::string encMailStr = std::string ( bufferMail , readBytes );
			email = xorDecrypt ( encMailStr );
			free(bufferMail);
		}
		sceIoClose ( fhMail );
		
		debugNetPrintf(DEBUG , "loading pass!\n");
		int fhPass = sceIoOpen ( "ux0:data/vitacord/user/set.ecr" , SCE_O_RDONLY , 0777 );
		fileSize = sceIoLseek ( fhPass, 0, SCE_SEEK_END ); 
		sceIoLseek ( fhPass, 0, SCE_SEEK_SET );
		if ( fileSize >= 1 ) {
			char * bufferPass = ( char * ) malloc ( fileSize );
			int readBytes = sceIoRead ( fhPass , bufferPass , fileSize );
			std::string encPassStr = std::string ( bufferPass , readBytes );
			password = xorDecrypt ( encPassStr );
			free(bufferPass);
		}
		sceIoClose ( fhPass );
		
		debugNetPrintf(DEBUG , "loading token!\n");
		int fhTok = sceIoOpen ( "ux0:data/vitacord/user/cr.ecr" , SCE_O_RDONLY , 0777 );
		fileSize = sceIoLseek ( fhTok, 0, SCE_SEEK_END );
		sceIoLseek ( fhTok, 0, SCE_SEEK_SET );
		if ( fileSize >= 5 ) {
			char * bufferToken = ( char * ) malloc ( fileSize );
			if(bufferToken) {
				int readBytes = sceIoRead ( fhTok , bufferToken , fileSize );
				if(readBytes == fileSize) {
					std::string encTokenStr = std::string ( bufferToken , readBytes );
					token = xorDecrypt ( encTokenStr );
				}
				free(bufferToken);
			}
		}
		sceIoClose ( fhTok );
		
	} else if ( oldEnc ) {
		// old enc
		debugNetPrintf(DEBUG , "Old loading!\n");
		for(i = 0 ; i < parserString.length() ; i++){
			
		
			
			if(parserString[i] == '\n'){
				if(getmail){
					logSD("newline . switching to getpass");
					getmail = false;
					getpass = true;
				}else if(getpass){
					logSD("newline . switching to token");
					getpass = false;
					gettoken = true;
				}else if(gettoken){
					gettoken = false;
					i = 9999;
					break;
				}
			}else if(parserString[i] == '\r'){
				
			}else if(parserString[i] == '\0'){
				logSD("NIL character");
			}else{
				if(getmail){
					email += parserString[i];
				}else if(getpass){
					password += parserString[i];
				}else if(gettoken){
					token += parserString[i];
				}
			}
			
		}
		
		token = simpleDecrypt(token);
		
		saveUserDataToFile(token);
		
	}
	
	
	if (token.empty() || token.length() < 5) {
		token = TOKEN;
		saveUserDataToFile(token);
	}
	discord.setToken(token);
	
	vitaGUI.loginTexts[0] = discord.getToken();
	vitaGUI.loginTexts[1] = "";
}

void DiscordApp::saveUserDataToFile(std::string _tok){
	
	
	_tok = xorEncrypt(_tok);
	
	int fh = sceIoOpen("ux0:data/vitacord/user/cr.ecr", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	sceIoWrite(fh, _tok.c_str(), _tok.length());
	sceIoClose(fh);
	
}


void DiscordApp::Start(){
	
	std::string directories[8] = { "ux0:data" , "ux0:data/vitacord" , "ux0:data/vitacord/user" ,"ux0:data/vitacord/attachments"
									, "ux0:data/vitacord/attachments/images" , "ux0:data/vitacord/attachments/other"
										, "ux0:data/vitacord/attachments/thumbnails" , "ux0:data/vitacord/settings" };
	
	
	struct SceIoStat * dirStat = (SceIoStat*)malloc(sizeof(SceIoStat));
	for(int i = 0 ; i < 8 ; i++){
		if(sceIoGetstat(directories[i].c_str() , dirStat) < 0){
			sceIoMkdir(directories[i].c_str() , 0777);
		}
	}
	free(dirStat);


	sceSysmoduleLoadModule(SCE_SYSMODULE_PHOTO_EXPORT);

	logSD("load userdata file");
	loadUserDataFromFile();
	logSD("pass discord pointer to vitaGUI");
	vitaGUI.passDiscordPointer( &discord );
	vitaGUI.passVITAIMEPointer( &vitaIME );
	logSD("check voice state");
	CheckVoiceState();
	logSD("start program loop");


/*---------------VIDEOCALL LOAD MODULE----------------------------------------------------------------------------------*/


// 1. CARICA IL MODULO HARDWARE IN MEMORIA (FONDAMENTALE)
    // Prima dell'inizializzazione hardware:
    debugNetPrintf(DEBUG, "[MAIN] Sto per caricare SCE_SYSMODULE_AVCDEC...\n");
    int modRes = sceSysmoduleLoadModule(SCE_SYSMODULE_AVCDEC);
    debugNetPrintf(DEBUG, "[MAIN] sceSysmoduleLoadModule ha restituito: 0x%08X\n", modRes);

    // Usiamo direttamente la struttura specifica richiesta da VitaSDK
    SceVideodecQueryInitInfoHwAvcdec queryInitInfo;
    memset(&queryInitInfo, 0, sizeof(queryInitInfo));
    
    // Configuriamo le specifiche massime che ci aspettiamo per questo progetto (480x272)
    queryInitInfo.size = sizeof(SceVideodecQueryInitInfoHwAvcdec);
    queryInitInfo.horizontal = 480; 
    queryInitInfo.vertical = 272;   
    queryInitInfo.numOfRefFrames = 3; 
    queryInitInfo.numOfStreams = 1;

    // Ora passiamo il puntatore della struttura corretta
    debugNetPrintf(DEBUG, "[MAIN] Sto per chiamare sceVideodecInitLibrary...\n");
    int videoDecRes = sceVideodecInitLibrary(SCE_VIDEODEC_TYPE_HW_AVCDEC, &queryInitInfo);
    debugNetPrintf(DEBUG, "[MAIN] sceVideodecInitLibrary ha restituito: 0x%08X\n", videoDecRes);

/*----------------------------------------------------------------------------------------------------------------------*/



	for(;;){
		
		
		
		
		logSD("vitagui draw");
		// DRAW 
		// instead of delay use -> vita2d_wait_rendering_done();
		//sceKernelDelayThread(5000);
		vitaGUI.Draw();
		//sceKernelDelayThread(5000);
		
		
		logSD("vitapad read");
		vitaPad.Read();
		logSD("vitatouch read");
		vitaTouch.readTouch();
		
		
		vitaGUI.analogScrollRight( vitaPad.right_analog_calibrated_x , -vitaPad.right_analog_calibrated_y );
		vitaGUI.analogScrollLeft( vitaPad.left_analog_calibrated_x , -vitaPad.left_analog_calibrated_y );
		
		if(vitaTouch.clicking){
			logSD("clicking check");
			clicked = vitaGUI.click(vitaTouch.lastClickPoint.x , vitaTouch.lastClickPoint.y, vitaTouch.touchDuration);
		}else{
			clicked = -1;
		}
		if(vitaTouch.scrolling){
			logSD("scolling check");
			scrolled = vitaGUI.scroll(vitaTouch.scrollDirX , vitaTouch.scrollDirY , vitaTouch.lastTouchPoint.x, vitaTouch.lastTouchPoint.y);
		}else{
			scrolled = -1;
		}
		vitaState = vitaGUI.GetState();
		if(vitaState == 0){
			switch(clicked){
				case 0:
					getUserTokenInput();
					break;
					
				case 1:
					doLogin();
					break;
					
				case 2:
					break;
			}
		}else if(vitaState == 1){
			if(discord.loadingData){

			}else{
				vitaGUI.SetState(2);
				sceKernelDelayThread(SLEEP_CLICK_NORMAL);
			}
		}else if(vitaState == 2){

			switch(clicked){
				case -1:
					break;
				case CLICKED_DM_ICON:
					vitaGUI.SetState(6);
					break;
				default:
					logSD("join guild");
					discord.JoinGuild(clicked);
					logSD("vitagui setstate(3)");
					vitaGUI.SetState(3);
					sceKernelDelayThread(SLEEP_CLICK_NORMAL);
					break;
				
			}
		}else if(vitaState == 3){
			if(vitaPad.circle){
				vitaGUI.SetState(2);
				sceKernelDelayThread(SLEEP_CLICK_NORMAL);
			}
			switch(clicked){
				case -1:
					break;
				case CLICKED_DM_ICON:
					vitaGUI.SetState(6);
					break;
				case CLICKED_VOICE_CHANNELS_TOGGLE:
					vitaGUI.showingVoiceChannels = !vitaGUI.showingVoiceChannels;
					vitaGUI.setChannelBoxes();
					break;
				case CLICKED_DISCONNECT_VOICE:
				    LeaveVoiceChannel();
				    break;
				default:
					if (vitaGUI.showingVoiceChannels) {
						OnVoiceChannelPressed(clicked);
					} else {
						JoinChannel(clicked);
					}
					break;
				
			}
			
		}else if(vitaState == 4){
			if(vitaPad.cross){
				SendChannelMessage();
			}else if(vitaPad.circle){
				discord.LeaveChannel();
				vitaGUI.SetState(2);
			}else{
				
			}
			
			switch(clicked){
				case -1:
					break;
				case CLICKED_DM_ICON:
					discord.LeaveChannel();
					vitaGUI.SetState(6);
					break;
					
				case CLICKED_VOICE_CHANNELS_TOGGLE:
					vitaGUI.showingVoiceChannels = !vitaGUI.showingVoiceChannels;
					vitaGUI.setChannelBoxes();
					break;

				case CLICKED_MESSAGE_INPUT:
					SendChannelMessage();
					break;
				
				case CLICKED_DISCONNECT_VOICE:
				    LeaveVoiceChannel();
				    break;

				default:
					if (vitaGUI.showingVoiceChannels) {
						OnVoiceChannelPressed(clicked);
					} else {
						JoinChannel(clicked);
					}
					break;
				
			}
			
			
		}else if(vitaState == 6){
			
			
			if(vitaPad.circle){
				LeaveDMChannel();
			}
			
			switch(clicked){
				case -1:
					break;
				case CLICKED_DM_ICON:
					LeaveDMChannel();
					break;
					
				case CLICKED_DISCONNECT_VOICE:
				    LeaveVoiceChannel();
				    break;

				default:
					JoinDMChannel(clicked);
					break;
				
			}
			discord.refreshDirectMessages();
			
		}else if(vitaState == 7){
			
			
			if(vitaPad.cross){
				SendDirectMessage();
			}else if(vitaPad.circle){
				LeaveDMChannel();
			}
			
			switch(clicked){
				case -1:
					break;
					
				case CLICKED_DM_ICON:
					LeaveDMChannel();
					break; 
					
				case CLICKED_MESSAGE_INPUT:
					SendDirectMessage();
					break;

				case CLICKED_DIRECT_CALL_START:
					OnDirectCallStart();
					break;
					
				case CLICKED_DISCONNECT_VOICE:
				    LeaveVoiceChannel();
				    break;

				default:
					JoinDMChannel(clicked);
					break;
				
			}
			discord.refreshCurrentDirectMessages();
			
		}
		
		
	}
	
}

DiscordApp::~DiscordApp()
{

sceSysmoduleUnloadModule(SCE_SYSMODULE_PHOTO_EXPORT);

}

/*Can't use this method beacuse psVita doesn't make you access sceIoGetStatus on a system folder as photo:*/
void DiscordApp::cleanupOrphanReceipts() {
    // Apriamo la cartella delle ricevute
    SceUID dfd = sceIoDopen("ux0:data/vitacord/receipts");
    if (dfd >= 0) {
        SceIoDirent dir;
        
        // Leggiamo ogni singolo file nella cartella
        while (sceIoDread(dfd, &dir) > 0) {
            std::string fileName = dir.d_name;
            
            // Ignoriamo le cartelle di sistema
            if (fileName == "." || fileName == "..") continue;

            std::string receiptPath = "ux0:data/vitacord/receipts/" + fileName;
            
            // Leggiamo il percorso scritto dentro la ricevuta
            SceUID fd = sceIoOpen(receiptPath.c_str(), SCE_O_RDONLY, 0);
            if (fd >= 0) {
                char savedPath[1024] = {0};
                int bytesRead = sceIoRead(fd, savedPath, sizeof(savedPath) - 1);
                sceIoClose(fd);

                if (bytesRead > 0) {
                    // FILTRO WHITELIST IN LETTURA
                    std::string cleanSavedPath = "";
                    for(int i = 0; i < bytesRead; i++){
                        char c = savedPath[i];
                        if(isalnum(c) || c == ':' || c == '/' || c == '.' || c == '_' || c == '-') {
                            cleanSavedPath += c;
                        }
                    }

                    // Se il percorso è valido (minimo 5 caratteri tipo "ux0:a")
                    if (cleanSavedPath.length() > 5) {
                        struct SceIoStat photoStat;
                        // Il momento della verità: la foto esiste ancora in Galleria?
                        if (sceIoGetstat(cleanSavedPath.c_str(), &photoStat) < 0) {
                            // LA FOTO NON ESISTE PIÙ! La ricevuta è orfana: la eliminiamo.
                            sceIoRemove(receiptPath.c_str());
                        }
                    } else {
                        // Il file txt conteneva spazzatura, lo distruggiamo
                        sceIoRemove(receiptPath.c_str());
                    }
                } else {
                    // Il file txt era da 0 byte, lo distruggiamo
                    sceIoRemove(receiptPath.c_str());
                }
            }
        }
        sceIoDclose(dfd);
    }
}

void DiscordApp::CheckVoiceState(){
    std::string proxyUrl = std::string("http://") + GO_SERVER_IP + ":" + GO_SERVER_PORT + "/api/status";
    VitaNet::http_response resp = discord.vitaNet.curlGet(proxyUrl);
    if(resp.httpcode == 200){
        try{
            nlohmann::json parsed = nlohmann::json::parse(resp.body);
            if(parsed.count("status") > 0 && parsed["status"] == "connected"){
                vitaGUI.showCallStrip = true;
            }
        }catch(...){
            logSD("Failed to parse status JSON");
        }
    }
}

void DiscordApp::OnVoiceChannelPressed(int channelIndex){
    logSD("Voice channel pressed: " + std::to_string(channelIndex));

    if(channelIndex >= 0 && static_cast<size_t>(channelIndex) < discord.guilds[discord.currentGuild].channels.size()){
        std::string guild_id = discord.guilds[discord.currentGuild].id;
        std::string channel_id = discord.guilds[discord.currentGuild].channels[channelIndex].id;

        // 1. RICHIESTA HTTP: Diciamo al bot di entrare nel canale
        nlohmann::json payload;
        payload["guild_id"] = guild_id;
        payload["channel_id"] = channel_id;

        // Assicurati che API_PORT (7777) sia corretta per il server Python
        std::string proxyUrl = std::string("http://") + GO_SERVER_IP + ":7777/api/join";
        VitaNet::http_response resp = discord.vitaNet.curlDiscordPost(proxyUrl, payload.dump(), "");

        if(resp.httpcode == 200){
            debugNetPrintf(DEBUG, "✅ API OK: Il bot è entrato. Avvio microfono...\n");
            
            // 2. COMANDO TCP: Accendiamo il plugin
            int s = sceNetSocket("VitaCordVoiceSocket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
            if (s >= 0) {
                SceNetSockaddrIn serveraddr;
                serveraddr.sin_family = SCE_NET_AF_INET;
                serveraddr.sin_addr.s_addr = sceNetHtonl(0x7F000001); // 127.0.0.1
                serveraddr.sin_port = sceNetHtons(VOICE_PLUGIN_PORT); // 9999

                if (sceNetConnect(s, (SceNetSockaddr *)&serveraddr, sizeof(serveraddr)) >= 0) {
                    VitaCordCommand cmd;
                    memset(&cmd, 0, sizeof(cmd)); // Pulizia memoria

                    cmd.command = static_cast<uint32_t>(CMD_START_STREAMING);
                    // Manda l'audio all'IP del server Python
                    snprintf(cmd.target_ip, sizeof(cmd.target_ip), "%s", GO_SERVER_IP); 
                    cmd.target_port = 5555; // Porta UDP
                    
                    sceNetSend(s, &cmd, sizeof(cmd), 0);
                    debugNetPrintf(DEBUG, "✅ COMANDO TCP INVIATO AL PLUGIN!\n");
                }
                sceNetSocketClose(s);
            }

            // 3. Aggiorna interfaccia grafica
            vitaGUI.showCallStrip = true;
            vitaGUI.connectedVoiceChannelName = discord.guilds[discord.currentGuild].channels[channelIndex].name;
        } else {
            debugNetPrintf(DEBUG, "❌ ERRORE API: Il bot non è entrato (HTTP %d)\n", resp.httpcode);
        }
    }
}

void DiscordApp::LeaveVoiceChannel(){
    logSD("Leaving Voice Channel");

    // 1. COMANDO TCP: Spegniamo il microfono PRIMA di cacciare il bot
    int s = sceNetSocket("VitaCordVoiceSocket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
    if (s >= 0) {
        SceNetSockaddrIn serveraddr;
        serveraddr.sin_family = SCE_NET_AF_INET;
        serveraddr.sin_addr.s_addr = sceNetHtonl(0x7F000001); // 127.0.0.1
        serveraddr.sin_port = sceNetHtons(VOICE_PLUGIN_PORT); // 9999

        if (sceNetConnect(s, (SceNetSockaddr *)&serveraddr, sizeof(serveraddr)) >= 0) {
            VitaCordCommand cmd;
            memset(&cmd, 0, sizeof(cmd));
            cmd.command = static_cast<uint32_t>(CMD_STOP_STREAMING);

            sceNetSend(s, &cmd, sizeof(cmd), 0);
            debugNetPrintf(DEBUG, "🛑 COMANDO TCP STOP INVIATO.\n");
        }
        sceNetSocketClose(s);
    }

    // 2. RICHIESTA HTTP: Diciamo al bot di scollegarsi
    if(discord.currentGuild >= 0 && discord.currentGuild < discord.guilds.size()){
        std::string guild_id = discord.guilds[discord.currentGuild].id;
        nlohmann::json payload;
        payload["guild_id"] = guild_id;

        std::string proxyUrl = std::string("http://") + GO_SERVER_IP + ":7777/api/leave";
        discord.vitaNet.curlDiscordPost(proxyUrl, payload.dump(), "");
    }

    // 3. Nascondi interfaccia
    vitaGUI.showCallStrip = false;
    vitaGUI.connectedVoiceChannelName = "";
}

void DiscordApp::OnDirectCallStart(){
	logSD("Direct call started");
}

void DiscordApp::doLogin(){
	
	vitaGUI.showLoginCue();
	int loginR = discord.login();
	if(loginR  == 200){
		logSD("Login Success");
		vitaGUI.loadingString = "Loading your stuff , " + discord.getUsername();
		saveUserDataToFile(discord.getToken());
		discord.loadData();
		logSD("Loaded data");
		vitaGUI.SetState(1);
	}else if(loginR == -11){
		vitaGUI.loginTexts[2] = "Token too short!";
	}else{
		vitaGUI.loginTexts[2] = "Error Code " + std::to_string(loginR);
		std::string errorStr = "Unknown error = " + std::to_string(loginR);
		criticalLogSD(errorStr.c_str());
	}
	
	vitaGUI.unshowLoginCue();
	vitaGUI.setUserInfo();
	
	sceKernelDelayThread(SLEEP_CLICK_EXTENDED);
	
	
}



void DiscordApp::getUserTokenInput(){
	vitaGUI.loginTexts[2] = "";
	
	std::string newtoken = vitaIME.getUserText(tokenTitle , (char *)discord.getToken().c_str() );
	discord.setToken(newtoken);
	vitaGUI.loginTexts[0] = newtoken;
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
}


void DiscordApp::SendChannelMessage(){
	
	std::string userMessage = vitaIME.getUserText(messageTitle);
	discord.sendMessage(userMessage);
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
	
}

void DiscordApp::SendDirectMessage(){
	
	std::string userMessage = vitaIME.getUserText(messageTitle);
	discord.sendDirectMessage(userMessage);
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
}

void DiscordApp::JoinDMChannel(int index){
	
	discord.LeaveDirectMessageChannel();	
	discord.JoinDirectMessageChannel(index);
	vitaGUI.SetState(7);
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
}

void DiscordApp::LeaveDMChannel(){
	discord.LeaveDirectMessageChannel();
	vitaGUI.SetState(2);
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
	
}

void DiscordApp::JoinChannel(int index){

	discord.LeaveChannel();
	discord.JoinChannel(index);
	vitaGUI.SetState(4);
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
}


