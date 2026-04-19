with open("src/DiscordApp.cpp", "r") as f:
    data = f.read()

new_data = data.replace(
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
		}else if(vitaState == 1){""",
"""		vitaState = vitaGUI.GetState();
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
		}else if(vitaState == 1){"""
)

with open("src/DiscordApp.cpp", "w") as f:
    f.write(new_data)
