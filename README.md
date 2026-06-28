# CloneMC-entirely-in-Pure-C
CloneMc is currently a highly early alpha version of a recreation or supposed recreation of Minecraft beta 1.7.3 in Pure C With Open GL Rendering 1.1 that is able to run on systems as low as Windows XP and technically can even run on Windows 98. 

If anyone is needing to play on the newest zip version of this program CloneMCSingleMultiCodeTest.zip, the zlib.dll from the DLLS folder is a bit too new for old systems, so it is highly recommded it is swapped out for older versions of operating system chosen in C:\Windows\System32. Singleplayer worlds is a bit weird. 

The second newest version of this program, in the CloneMCtest + Makefile folder, has good singleplayer worlds, but doesn't support multiplayer like the newest version. 

Testing100.exe was the very first version of this program, it is basically just there now. 

Commits and forks are highly recommend for this project to continue and actually succeed. 

Many tools such as extensive research through YouTube videos, Minecraft Wiki/Fandom, Reddit, java examinations of beta verion, AI tools, and Open Watcom have helped significantly in making this project possible. 

Fair warning that some of the textures and rendering are a bit severly broken, but still playable. 

Here are still functions that still need to be implemented into Pure C program , 

THERE WILL NOT AND SHOULD NEVER HAVE SUPPORT FOR OFFICAL MODERN MINECRAFT SERVERS, MINECRAFT ACCOUNTS, AND MORE RELATED TO AVOID COPYRIGHT ISSUES.

GAME CREATED USING CLEAN ROOM IMPLEMENTATIONS AND REVERSE ENGINEERING IN OPEN WATCOM C. 

## Feature gaps still not fully added to the C game
- **Full Java `ModelRenderer` hierarchy:** C now has cuboid mobs with Java-style limb math, but it still does not parse every Java model box/rotation point exactly.
- Saved Worlds will not be saved correctly, even if properly saved through Save And Quit.  
- **Complete `RenderLiving` hurt/death render passes:** Basic hurt flash/lighting exists, but all armor/pass/layer rendering is not fully ported.
- **Full `ContainerWorkbench` and `CraftingManager` recipe book:** 2x2 and several 3x3 recipes exist, but the full Java shaped/shapeless recipe list is not completely loaded.
- **NBT/McRegion save format:** Needs more implementations for exact `.mcr`/NBT chunks.
- **Tile entities:**  port does not yet simulate redstone wire, repeaters, rails, doors, levers, pressure plates, and powered states.
- **Full item durability/tools/combat:
- **Complete biome colorizers:** 
- **Full sound pool randomization:
- **Networking/multiplayer

Here are some early test screenshots 

<img width="694" height="403" alt="Screenshot 2026-06-15 011818" src="https://github.com/user-attachments/assets/6eb74565-fa59-4ade-be6c-a7acd4c1c093" />

<img width="697" height="401" alt="Screenshot 2026-06-15 012523" src="https://github.com/user-attachments/assets/e5f954de-1b81-49ad-8d5c-ffce60ccc03d" />

<img width="959" height="503" alt="Screenshot 2026-06-15 020015" src="https://github.com/user-attachments/assets/45127caf-e314-4531-a879-a8e81c9d7763" />


Improved Alpha Test Screenshot


<img width="1280" height="720" alt="screenshot_1042968968" src="https://github.com/user-attachments/assets/6a7d351d-5d86-41bb-aba7-72483d0d012e" />

TO RUN PROGRAM, extract the zip files the get the folders of assets, docs, and saves, and also extract the additonal music zip to get music and put that along in the assets folder. THE ASSET FOLDER SHOULD NOT HAVE ANOTHER ASSET FOLDER INSIDE OF IT, IF IT DOES, TAKE IT OUT AND MAKE SURE WHEN OPENING ASSET DOLER THAT ALL ACTUAL ASSET FILES AND TEXTURES COME OUT FIRST. SAME GOES FOR THE OTHER FILES. Then run testing100.exe or the other one and it should all work. ASSETNEW FOLDER IS RECOMMENDED TO USE WHICH SHOULD BE RENAMED BACK TO JUST "ASSETS" FOLDER IN THE SAME FOLDER AS WHERE THE EXECUTABLE IS with also the asset files coming out first instead of just another assets folder. 

## How This Program Works With Compatibility

Mobs and items are mapped with textures using UV mapping of tga which Windows 98 and later operating systems like Windows XP supports. The uv mapping however in blocks is severly messed up, but still playable and able to be figured out. There will be fixes for that soon. The cuboid models/math were reverse engineered from the java files examined in the beta version which heavily helped with this project. 

OpenGL 1.1 was mainly used for this entire program becuase of its compatibility with older operating systsem like WIndows XP or 98 and it able to immediately start up for instance without proper grpahics drivers, though with that, perforamnce may significanlty suffer. The lighting, color shading, chunk meshing, and even optimizations like adding vsync and coded limited render distance limits were added to help give as much performance and fps as possible in this program. The game also doesn't draw hidden block faces and groups visible faces to prevent the lag spikes which used to be really bad during development. Running complex servers however may significantly suffer currently, espeically if it has a lot of stuff and complex shapes, but it can still work, more info in the next paragraph. 

Compatibility for this program can definetly run on systems as low as Windows XP, although, with some performance issues currently, but still playable. Windows 98 is uncertain, but it has all the code and period correct dependencies to run the program, and possibly even be playable. The network stack even has Winsock 2 which should be able to connect to servers and have multiplayer, though for optimal terrain generation for multiplayer, zlib.dll must be found which is usually located in C:\Windows\system32\ or C:\Windows\SysWow64\


## Important Information For Compiling And More 

Saved Worlds do save somwehat well and correctly in the new version, not really in testing100.exe though.

The goal of this program really is to show that the resources and tools available that can help spread and make programming development accessible to almost everyone around the world to create complex games/applications without much if any money. Compared to just a couple of years ago where making complex games/applications would require extensive own knowledge of a programming langauge and years of experience, many people which don't have access to such higher education. AI tools were mainly used for the UV mapping of textures (needs significant improvement), world generation, crafting recipes, basic movement, and day and night cycle while textures, sounds, mob models, icons, items, algorithims, GUIs, and many more were hand written.  

If extracting the newest current test zip "CloneMCSingleMultiCodeTest.zip" is done and the code in src is edited and finished for compiling. Install Open Watcom C/C++ compiler, and in command prompt, run cd C:\Watcom, then run owsetenv.bat and then cd binnt64, and then from the extracted and code edited CloneMCSingleMultiCodeTest folder, take the project2finalalpha.c, clonemc_v74_nt_win link file, and Makefile.v74.wat and put it in binnt64 folder, binnt folder can also work if compiling in 32 bit systems. Then run "wmake -f Makefile.v74.wat" which should create a new executable and put it in the CloneMCtest + Makefile folder along with the asset folder. Make sure to delete the .obj and src folders related to the project in binnt64 if process is needed to start over, the makefiles and link files can be edited to change the output names of the executables created for example. 

If the src folder in the github repo is edited and should be put in the second new test folder, CloneMCtest + Makefile, Install Open Watcom C/C++ compiler, and in command prompt, run cd C:\Watcom, then run owsetenv.bat and then cd binnt64, and then from the CloneMCtest + Makefile folder, take the src folder, project2finalalpha.c, clonemc_v58_nt_win link file, and Makefile.v58.wat and put it in binnt64 folder, binnt folder can also work if compiling in 32 bit systems. Then run "wmake -f Makefile.v58.wat" which should create executable and put it in the CloneMCtest + Makefile folder along with the asset folder. Make sure to delete the .obj and src folders related to the project in binnt64 if process is needed to start over, the makefiles and link files can be edited to change the output names of the executables created for example. project2finalalpharecreation.c used to be entire code in a c file, now it is just a linker file for the src folder. 


Some of the c files for functions and features like mob rendering, textures, and world gen, and a little more may be mixed throughout the files, but it likely should be manageable to still work on the project. 

There is proper documentation if anyone wants to work on this a little. 

If anyone wants to compile the everythinginonefile.c, just run the bat file in watcom directory, copy the file into the binnt64 or binnt, run and run wcl386 everythinginonefile.c -fe=testing102.exe -lr=nt_win -l=win, from either chosen two binnt folders. 

Heavy inspiration taken from ClassiCube






