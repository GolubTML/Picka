# Picka the ModLoader

_Picka_ - is modloader for mobile Terraria, allowing you to write simple mods on Lua. For now, API of picka is small, it considered about 16 method in total. Some example can be:
`picka.log(str)`
`picka.getClass(assembly, namespace, class`
`picka.callMethod(method, arguments)`
And so much more. 

To use picka, firstly you need to compile payload and loader. For this, use following command:
``` Shell
make -C jni/payload j$(nproc)
```
This is for payload.so. 
``` Shell
make -C jni/loader j$(nproc)
```
And this is for loader.

All files should be in `build` folder after compilation. 
Next, you need to connect via `adb` to your mobile device (or emulator). Also, you need acces to Terraria apk file (use apktool for it). 
After all of this, you need to copy paste libloader.so and payload.so to libs folder in Terraria.
<img width="990" height="147" alt="изображение" src="https://github.com/user-attachments/assets/20775b9b-878d-4a7d-8608-2e7e48dbc6fd" />


And also, you need to compile FloatButtonHelper (use `./JavaHelper/javac_compile.sh` in root folder.) After this, take `class.dex` and put it into this path `Terraria/build/apk` (_Terraria_ is a example folder, where you unzip Terraria.apk) and rename your `class.dex` to `class2.dex` (need to run app).

After all of this staff, use `assemble_apk.sh` in root folder (_but adjust path to your Terraria resourses in file_) and here you have it, Picka it self. 
In addition, give Terraria acces to storage in settings of your phone, because Picka runs mods from `Mods` folder in `/storage/emulated/0/` (this will be fixed in launcher)

For documentation, see this [documentation](Documentation.md)

Enjoy!
