with open("src/DiscordApp.cpp", "r") as f:
    data = f.read()

import re

new_data = data.replace(
"""		vitaState = vitaGUI.GetState();
		if(vitaState == 0){
			switch(clicked){
				case 0:
					getUserEmailInput();
					break;

				case 1:
					getUserPasswordInput();
					break;

				case 2:
					doLogin();
					break;
			}
		}else if(vitaState == 1){""",
"""		vitaState = vitaGUI.GetState();
		if(vitaState == 0){
			switch(clicked){
				case 0:
					getUserTokenInput();
					break;

				case 1:
					break;

				case 2:
					doLogin();
					break;
			}
		}else if(vitaState == 1){"""
)

new_data = new_data.replace(
"""void DiscordApp::doLogin(){

	vitaGUI.showLoginCue();
	int loginR = discord.login();
	if(loginR  == 200){
		logSD("Login Success");
		vitaGUI.loadingString = "Loading your stuff , " + discord.getUsername();
		saveUserDataToFile(discord.getEmail() , discord.getPassword() , discord.getToken());
		discord.loadData();
		logSD("Loaded data");
		vitaGUI.SetState(1);
	}else if(loginR == 200000){
		int login2faResponse = discord.submit2facode(vitaIME.getUserText(get2facodeTitle));
		if( login2faResponse == 200){
			logSD("Login (2FA) Success");
			vitaGUI.loadingString = "Wait a second " + discord.getUsername();
			saveUserDataToFile(discord.getEmail() , discord.getPassword() , discord.getToken());
			discord.loadData();
			logSD("Loaded data");
			vitaGUI.SetState(1);
			logSD("Set state");
		}else if(login2faResponse == -15){
			vitaGUI.loginTexts[2] = "2FA code too short! ";
		}else if(login2faResponse == -120){
			vitaGUI.loginTexts[2] = "D> JSON parse error! ";
		}else if(login2faResponse == -125){
			vitaGUI.loginTexts[2] = "D> JSON response was empty! ";
		}else if(login2faResponse == -126){
			vitaGUI.loginTexts[2] = "D> 2FA token was empty! ";
		}else{
			vitaGUI.loginTexts[2] = "Error Code " + std::to_string(login2faResponse);;
			std::string errorStr = "Unknown error 2fa = " + std::to_string(login2faResponse);
			criticalLogSD(errorStr.c_str());
		}
	}else if(loginR == -11){
		vitaGUI.loginTexts[2] = "Email too short!";
	}else if(loginR == -12){
		vitaGUI.loginTexts[2] = "Password too short!";
	}else if(loginR == -120){
		vitaGUI.loginTexts[2] = "D> JSON parse error! ";
	}else if(loginR == -125){
		vitaGUI.loginTexts[2] = "D> JSON response was empty! ";
	}else if(loginR == -127){
		vitaGUI.loginTexts[2] = "D> No token or mfa in JSON.";
	}else if(loginR == -129){
		vitaGUI.loginTexts[2] = "D> MFA ticket was empty !";
	}
	else{
		vitaGUI.loginTexts[2] = "Error Code " + std::to_string(loginR);
		std::string errorStr = "Unknown error = " + std::to_string(loginR);
		criticalLogSD(errorStr.c_str());
	}

	vitaGUI.unshowLoginCue();
	vitaGUI.setUserInfo();

	sceKernelDelayThread(SLEEP_CLICK_EXTENDED);


}



void DiscordApp::getUserEmailInput(){
	vitaGUI.loginTexts[2] = "";

	std::string newemail = vitaIME.getUserText(emailTitle , (char *)discord.getEmail().c_str() );
	discord.setEmail(newemail);
	vitaGUI.loginTexts[0] = newemail;
	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
}

void DiscordApp::getUserPasswordInput(){
	vitaGUI.loginTexts[2] = "";

	std::string newpassword = vitaIME.getUserText(passwordTitle );
	discord.setPassword(newpassword);
	vitaGUI.loginTexts[1] = "";
	for(unsigned int i = 0 ; i < newpassword.length() ; i++){
		vitaGUI.loginTexts[1] += "*";
	}

	sceKernelDelayThread(SLEEP_CLICK_NORMAL);
}""",
"""void DiscordApp::doLogin(){

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
}"""
)

with open("src/DiscordApp.cpp", "w") as f:
    f.write(new_data)
