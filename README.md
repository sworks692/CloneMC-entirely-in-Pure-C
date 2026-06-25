# CloneMC-entirely-in-Pure-C
CloneMc is currently a highly early alpha version of a recreation or supposed recreation of Minecraft beta 1.7.3 in Pure C With Open GL Rendering 1.1 that is able to run on systems as low as Windows XP and even Windows 98 (not tested). It currently only supports survival mode.

Commits and forks are highly recommend for this project to continue and actually succeed. 

Many tools such as extensive research through YouTube videos, Minecraft Wiki/Fandom, Reddit, Java information of beta verion, AI tools, and Open Watcom have helped significantly in making this project possible. 

Fair warning that some of the textures and mob models are a bit severly broken, but still playable. 

Here are still functions that still need to be implemented into Pure C program , 

THERE WILL NOT AND SHOULD NEVER HAVE SUPPORT FOR OFFICAL MODERN MINECRAFT SERVERS, MINECRAFT ACCOUNTS, AND MORE RELATED TO AVOID COPYRIGHT ISSUES.

GAME CREATED USING CLEAN ROOM IMPLEMENTATIONS AND REVERSE ENGINEERING IN OPEN WATCOM C. 

## Feature gaps still not fully added to the C game
- **Full Java `ModelRenderer` hierarchy:** C now has cuboid mobs with Java-style limb math, but it still does not parse every Java model box/rotation point exactly.
- Saved Worlds will not be saved correctly, even if properly saved through Save And Quit.  
- **Complete `RenderLiving` hurt/death render passes:** Basic hurt flash/lighting exists, but all armor/pass/layer rendering is not fully ported.
- **Full `ContainerWorkbench` and `CraftingManager` recipe book:** 2x2 and several 3x3 recipes exist, but the full Java shaped/shapeless recipe list is not completely loaded.
- **NBT/McRegion save format:** The C save system is text/binary-lightweight for Win98 compatibility, not real `.mcr`/NBT chunks.
- **Tile entities:**  port does not yet simulate redstone wire, repeaters, rails, doors, levers, pressure plates, and powered states.
- **Full item durability/tools/combat:
- **Complete biome colorizers:** 
- **Full sound pool randomization:
- **Networking/multiplayer

Here are some early test screenshots 

<img width="694" height="403" alt="Screenshot 2026-06-15 011818" src="https://github.com/user-attachments/assets/6eb74565-fa59-4ade-be6c-a7acd4c1c093" />

<img width="697" height="401" alt="Screenshot 2026-06-15 012523" src="https://github.com/user-attachments/assets/e5f954de-1b81-49ad-8d5c-ffce60ccc03d" />

<img width="959" height="503" alt="Screenshot 2026-06-15 020015" src="https://github.com/user-attachments/assets/45127caf-e314-4531-a879-a8e81c9d7763" />

To Run program, extract the zip files the get the folders of assets, docs, and saves, and also extract the additonal zip to get the music folder and put that along in the assets folder. THE ASSET FOLDER SHOULD NOT HAVE ANOTHER ASSET FOLDER INSIDE OF IT, IF IT DOES, TAKE IT OUT AND MAKE SURE WHEN OPENING ASSET DOLER THAT ALL ACTUAL ASSET FILES AND TEXTURES COME OUT FIRST. SAME GOES FOR THE OTHER FILES Then run testing100.exe and it should all work. 


Important Information And More: Saved Worlds do not currently save correctly, clonemc experimental implementations program can work though.

The goal of this program really is to show that the resources and tools available like AI tools currently can help spread and make programming development accessible to almost everyone around the world to create complex games/applications. Compared to just a couple of years ago where making complex games/applications would require extensive own knowledge of a programming langauge and years of experience, many people which don't have access to such higher education. 

If any code in the program wants to be added or edited, it can be done through the project2finalalpha.c which contains over 15000 lines of code. Ctrl+F can help find code lines on Visual Studio Code related to wanted functions or worlds like editing textures. AI tools can also help significanlty understand and even add new code/functions like adding ridable boats for example in the C program. It is also highly recommded to make edits and code changes inside the clonemc experimental implementations 7z file as it has a lot more implemtations that has the potential to look and work like the real deal. 



To compile the .c file after desired edits are finished, install Open Watcom C/C++ compiler, then in command prompt run cd C:\Watcom, then run owsetenv.bat and then cd binnt64, and then run  "wcl386 project2finalalpha.c -fe=testing100.exe -lr=nt_win -l=win"  testing100 is the name of the desired exe file

Heavy inspiration taken from ClassiCube






