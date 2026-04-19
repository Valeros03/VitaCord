with open("src/VitaGUI.cpp", "r") as f:
    data = f.read()

new_data = data.replace(
"""	inputbox emailI;
	emailI.x = 431;
	emailI.y = 139;
	emailI.w = 375;
	emailI.h = 50;
	loginInputs.push_back(emailI);

	inputbox passwordI;
	passwordI.x = 431;
	passwordI.y = 219;
	passwordI.w = 375;
	passwordI.h = 50;
	loginInputs.push_back(passwordI);""",
"""	inputbox emailI;
	emailI.x = 431;
	emailI.y = 139;
	emailI.w = 375;
	emailI.h = 50;
	loginInputs.push_back(emailI);"""
)

new_data = new_data.replace(
"""	loginTexts.clear();
	loginTexts.push_back(" ");
	loginTexts.push_back(" ");
	loginTexts.push_back(" ");""",
"""	loginTexts.clear();
	loginTexts.push_back(" ");
	loginTexts.push_back(" ");
	loginTexts.push_back(" ");"""
)


new_data = new_data.replace(
"""void VitaGUI::DrawLoginScreen(){


		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(114, 137, 217, 255));
		vita2d_draw_texture( loginFormImage , 161, 53 );
		vita2d_font_draw_text(vita2dFont[18] , 438, 181, RGBA8(255,255,255,255), 18, loginTexts[0].c_str());
		vita2d_font_draw_text(vita2dFont[18] , 438, 261, RGBA8(255,255,255,255), 18, loginTexts[1].c_str());
		vita2d_font_draw_text(vita2dFont[18] , 750, 261, RGBA8(255,0,0,255), 18, loginTexts[2].c_str());

}""",
"""void VitaGUI::DrawLoginScreen(){


		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(114, 137, 217, 255));
		vita2d_draw_texture( loginFormImage , 161, 53 );
		vita2d_font_draw_text(vita2dFont[18] , 438, 181, RGBA8(255,255,255,255), 18, loginTexts[0].c_str());
		vita2d_font_draw_text(vita2dFont[18] , 750, 261, RGBA8(255,0,0,255), 18, loginTexts[2].c_str());

}"""
)

with open("src/VitaGUI.cpp", "w") as f:
    f.write(new_data)
