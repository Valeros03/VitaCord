#ifndef DISCORDAPP_HPP
#define DISCORDAPP_HPP


#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <cstdint>

#include "VitaTouch.hpp"
#include "VitaPad.hpp"
#include "VitaIME.hpp"
#include "Discord.hpp"
#include "VitaGUI.hpp"

#define GO_SERVER_IP "192.168.1.24"
#define GO_SERVER_PORT "7777"
#define GO_SERVER_UDP_PORT 5555
#define VOICE_PLUGIN_PORT 9999


// 1. L'enum per mantenere il codice pulito e leggibile
enum CommandType {
    CMD_STOP_STREAMING = 0,
    CMD_START_STREAMING = 1,
    CMD_SHUTDOWN_PLUGIN = 2
};

// 2. La struttura di rete BLINDATA (Usa uint32_t per forzare i 4 byte esatti)
struct __attribute__((packed)) VitaCordCommand {
    uint32_t command;     // Forzato a 32 bit (4 byte)
    char target_ip[16];   // Forzato a 16 byte
    uint32_t target_port; // Forzato a 32 bit (4 byte)
};

class DiscordApp{
	
	public:
	void Start();
	~DiscordApp();
    void cleanupOrphanReceipts();
    void loadUserDataFromFile();
    void saveUserDataToFile(std::string t);

private:
    char emptyMessage[1] = "";
    char tokenTitle[14] = "Discord Token";
    char messageTitle[8] = "Message";
    char get2facodeTitle[30] = "Enter your 2Factor Auth Code!";
    VitaGUI vitaGUI;
    Discord discord;
    VitaIME vitaIME;
    VitaPad vitaPad;
    VitaTouch vitaTouch;
    int clicked = -1;
    int scrolled = -1;
    int vitaState = 0;

    void doLogin();
    void getUserTokenInput();

    void SendChannelMessage();
    void SendDirectMessage();
    void JoinDMChannel(int index);
    void LeaveDMChannel();
    void JoinChannel(int index);
    void OnVoiceChannelPressed(int channelIndex);
    void OnDirectCallStart();
    void LeaveVoiceChannel();
    void CheckVoiceState();
	
};



#endif




