1. **Fix Numeri -> Emoji:**
   - Modify `src/Discord.cpp` around line 305 to ensure `emojiJsonData["emoji"][e]["utf32code"]` is loaded and if `emojiJsonData["emoji"][e]["utf32code"].size() == 1`, only then process and add the emoji to the `fastEmojiMap` and `emojiVector`. This prevents numbers with combining characters in the array from overriding the base number.

2. **Word Wrap (Andata a capo nel Custom Loop):**
   - Update `DrawTextWithEmojis` in `src/VitaGUI.hpp` and `src/VitaGUI.cpp` to accept an `int maxWidth` parameter.
   - Modify the inner loop of `DrawTextWithEmojis` to wrap at characters when `currentX` exceeds `startX + maxWidth`. Inside the loop, before drawing an emoji or text, check if `currentX > startX + maxWidth`. If so, set `currentX = startX` and `startY += size + 4`. (Using size + 4 for line height).

3. **Scaling delle Emoji:**
   - In `src/VitaGUI.cpp` within `DrawTextWithEmojis`, use `vita2d_draw_texture_part_scale` instead of `vita2d_draw_texture_part`. Set a scaling factor of 1.5f and update `currentX += (16 * 1.5f)`.

4. **Spaziatura Sender / Messaggio:**
   - In `src/VitaGUI.cpp` inside `DrawMessages`, update the Y-coordinate for the message content. Change `yPos + 50` to `yPos + 60` for `DrawTextWithEmojis`. Inside `DrawDirectMessageMessages`, change `yPos + 50` to `yPos + 60` for `DrawTextWithEmojis`.

5. **Rendering delle Menzioni (<@ID>):**
   - In `src/VitaGUI.cpp`, inside `DrawMessages` and `DrawDirectMessageMessages`, before calling `DrawTextWithEmojis`, format the message content using `std::regex_replace` with `<@!?(\\d+)>` to `@User`. Modify the call to `DrawTextWithEmojis` to pass this formatted string. (Include `<regex>` at the top of the file).

6. **Colori dei Ruoli per gli Username:**
   - In `src/VitaGUI.cpp` around line 1222 (`DrawMessages`), change `RGBA8(255, 255, 255, 255)` to `messageBoxes[i].userColor ? messageBoxes[i].userColor : RGBA8(255, 255, 255, 255)` when drawing the username text with `vita2d_font_draw_text`. The color parsing logic in `src/Discord.cpp` is already correctly getting the user's role color.

7. **Verify & Test Compilation:**
   - Run `./autobuild.sh` or the relevant build command to ensure the codebase still compiles successfully after the C++ modifications. Check that no regressions were introduced.

8. **Pre-commit:**
   - Complete pre commit steps to ensure proper testing, verification, review, and reflection are done.
