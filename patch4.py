with open("src/Discord.cpp", "r") as f:
    data = f.read()

import re

new_data = data.replace(
"""std::string Discord::getEmail(){
	return email;
}
std::string Discord::getPassword(){
	return password;
}
void Discord::setEmail(std::string mail){
	email = mail;
}
void Discord::setPassword(std::string pass){
	password = pass;
}

long Discord::login(){
	return login(email , password);
}
long Discord::login(std::string mail , std::string pass){
	criticalLogSD("Login attempt.\\n");
	email = mail;
	password = pass;

	/*if(token.length() > 20){
		if(fetchUserData() == 200){
			loggedin = true;
			return 200;
		}else{
			token = "";
		}

	}*/

	if(email.length() < 1){
		criticalLogSD("Email to short! \\n");
		return -11;
	}else if(password.length() < 1){
		criticalLogSD("Password too short \\n");
		return -12;
	}

	//std::string loginUrl = "http://jaynapps.com/psvita/httpdump.php";  // DBG
	std::string loginUrl = "https://discord.com/api/v9/auth/login";
	std::string postData = "{ \\"login\\":\\"" + email + "\\" , \\"password\\":\\"" + password + "\\" }";

	criticalLogSD("Login request to : \\n");
	criticalLogSD(loginUrl.c_str());
	criticalLogSD("with data : \\n");
	criticalLogSD((xorEncrypt(postData)).c_str());

	VitaNet::http_response loginresponse = vitaNet.curlDiscordPost(loginUrl , postData , "");
	if(loginresponse.httpcode == 200){


		criticalLogSD("Login request success : \\n");
		criticalLogSD((xorEncrypt(loginresponse.body)).c_str());
		criticalLogSD("\\n");

		// check if Two-Factor-Authentication is activated and needs further user action
		nlohmann::json j_complete = nlohmann::json::parse(loginresponse.body);
		try{
			if(!j_complete.is_null()){
				if(!j_complete["mfa"].is_null()){
					if(!j_complete["ticket"].is_null()){
						// Two factor auth is enabled ! grab ticket
						criticalLogSD("Account uses 2FA. Need 2FA code to proceed login. \\n");
						twoFactorAuthEnabled = true;
						ticket = j_complete["ticket"].get<std::string>();
						logSD("Need 2FA Code");
						loginresponse.httpcode = 200000; // NEED MFA
					}else{
						criticalLogSD("Login failed , mfa ticket was NULL! \\n");
						return -129;
					}
				}else if(!j_complete["token"].is_null()){
					// Logged in !
					criticalLogSD("Logged in without 2FA! \\n");
					token = j_complete["token"].get<std::string>();
					loggedin = true;
					fetchUserData();
				}else{
					criticalLogSD("Login failed, token and mfa were NULL !\\n");
					return -127;
				}

			}else{
				criticalLogSD("Login failed, JSON was null! \\n");
				return -125;
			}
		}catch(const std::exception& e){
			criticalLogSD("Login failed. Couldn't parse JSON ! \\n");
			return -120;
		}


	}else{
		// login failed >_>
		criticalLogSD("Login request failed because : \\n");
		criticalLogSD(loginresponse.body.c_str());
		criticalLogSD("\\n");

	}



	return loginresponse.httpcode;

}


long Discord::submit2facode(std::string code){

	std::string loginUrl = "https://discord.com/api/v9/auth/mfa/totp";
	std::string postData = "{ \\"code\\":\\"" + code + "\\" , \\"ticket\\":\\"" + ticket + "\\" }";

	if(code.length() < 1){
		criticalLogSD("2FA Code to short! \\n");
		return -15;
	}

	criticalLogSD("2FA request to : \\n");
	criticalLogSD(loginUrl.c_str());
	criticalLogSD("with data : \\n");
	criticalLogSD((xorEncrypt(postData)).c_str());

	VitaNet::http_response submit2facoderesponse = vitaNet.curlDiscordPost( "https://discord.com/api/v9/auth/mfa/totp" , postData , token);
	if(submit2facoderesponse.httpcode == 200){


		criticalLogSD("2FA request success : \\n");
		criticalLogSD((xorEncrypt(submit2facoderesponse.body)).c_str());
		criticalLogSD("\\n");


		nlohmann::json j_complete2 = nlohmann::json::parse(submit2facoderesponse.body);
		try{
			if(!j_complete2.is_null()){
				if(!j_complete2["token"].is_null()){
					// Logged in !
					token = j_complete2["token"].get<std::string>();
					loggedin = true;
					fetchUserData();
				}else{
					criticalLogSD("2FA failed, token was empty! \\n");
					return -126;
				}
			}else{
				criticalLogSD("2FA failed, JSON was null! \\n");
				return -125;
			}
		}catch(const std::exception& e){
			criticalLogSD("2FA failed. Couldn't parse JSON ! \\n");
			return -120;
		}

	}else{
		criticalLogSD("2FA request failed because : \\n");
		criticalLogSD(submit2facoderesponse.body.c_str());
		criticalLogSD("\\n");
	}

	return submit2facoderesponse.httpcode;
}

std::string Discord::getToken(){
	return token;
}""",
"""long Discord::login(){
	return login(token);
}
long Discord::login(std::string tok){
	criticalLogSD("Login attempt.\\n");
	token = tok;

	if(token.length() < 1){
		criticalLogSD("Token too short! \\n");
		return -11;
	}

	long fetchStatus = fetchUserData();
	if (fetchStatus == 200) {
		loggedin = true;
	}
	return fetchStatus;
}

std::string Discord::getToken(){
	return token;
}"""
)

with open("src/Discord.cpp", "w") as f:
    f.write(new_data)
