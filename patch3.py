with open("src/Discord.hpp", "r") as f:
    data = f.read()

new_data = data.replace(
"""		long login();
		long login(std::string mail , std::string pass);
		long submit2facode(std::string code);""",
"""		long login();
		long login(std::string tok);"""
)

new_data = new_data.replace(
"""		void setEmail(std::string mail);
		void setPassword(std::string pass);
		void setToken(std::string tok);""",
"""		void setToken(std::string tok);"""
)

with open("src/Discord.hpp", "w") as f:
    f.write(new_data)
