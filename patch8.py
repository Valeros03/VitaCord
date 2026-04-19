with open("src/DiscordApp.cpp", "r") as f:
    data = f.read()

import re

new_data = data.replace(
"""		email = simpleDecrypt(email);
		password = simpleDecrypt(password);
		token = simpleDecrypt(token);

		saveUserDataToFile(email , password , token);

	}


	discord.setEmail(email);
	discord.setPassword(password);
	discord.setToken(token);

	vitaGUI.loginTexts[0] = discord.getEmail();
	vitaGUI.loginTexts[1] = "";
	for( i = 0 ; i < password.length() ; i++){
		vitaGUI.loginTexts[1] += "*";
	}
}

void DiscordApp::saveUserDataToFile(std::string mail , std::string pass , std::string _tok){


	mail = xorEncrypt(mail);
	pass = xorEncrypt(pass);
	_tok = xorEncrypt(_tok);

	//std::string userdata = mail + "\n" + pass + "\n" + _tok + "\n";
	int fh = sceIoOpen("ux0:data/vitacord/user/loc.ecr", SCE_O_WRONLY | SCE_O_CREAT, 0777);
	sceIoWrite(fh, mail.c_str(), strlen(mail.c_str()));
	sceIoClose(fh);
	fh = sceIoOpen("ux0:data/vitacord/user/set.ecr", SCE_O_WRONLY | SCE_O_CREAT, 0777);
	sceIoWrite(fh, pass.c_str(), strlen(pass.c_str()));
	sceIoClose(fh);
	fh = sceIoOpen("ux0:data/vitacord/user/cr.ecr", SCE_O_WRONLY | SCE_O_CREAT, 0777);
	sceIoWrite(fh, _tok.c_str(), strlen(_tok.c_str()));
	sceIoClose(fh);
}""",
"""		token = simpleDecrypt(token);

		saveUserDataToFile(token);

	}


	discord.setToken(token);

	vitaGUI.loginTexts[0] = discord.getToken();
	vitaGUI.loginTexts[1] = "";
}

void DiscordApp::saveUserDataToFile(std::string _tok){


	_tok = xorEncrypt(_tok);

	int fh = sceIoOpen("ux0:data/vitacord/user/cr.ecr", SCE_O_WRONLY | SCE_O_CREAT, 0777);
	sceIoWrite(fh, _tok.c_str(), strlen(_tok.c_str()));
	sceIoClose(fh);
}"""
)

with open("src/DiscordApp.cpp", "w") as f:
    f.write(new_data)
