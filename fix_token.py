with open("src/Discord.cpp", "r") as f:
    content = f.read()

# I accidentally replaced `TOKEN` with `token` because I thought it was an undeclared variable when the build failed, but actually it is defined in `key.h` which I had to mock during build.
content = content.replace("token", "TOKEN")

with open("src/Discord.cpp", "w") as f:
    f.write(content)
