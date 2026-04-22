#ifndef DISCORDAPP_HPP
#define DISCORDAPP_HPP


#include <psp2/kernel/processmgr.h>

#include "VitaTouch.hpp"
#include "VitaPad.hpp"
#include "VitaIME.hpp"
#include "Discord.hpp"
#include "VitaGUI.hpp"


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
	
};



#endif




