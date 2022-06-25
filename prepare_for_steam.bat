REM delete the windows_content folder
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\" /s /q

REM copy each required file to run the game
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\D3DX10d_43.dll C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\ikpFlac.dll C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\ikpMP3.dll C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\irrKlang.dll C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\steam_api64.dll C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i
REM xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\steam_api64.lib C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i
xcopy C:\Users\jerom\Documents\git\2DPlatformer\x64\Release\DeathLeakGame.exe C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i


REM copy the "Media folder"
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\Media C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\Media /y /i /s

REM copy the shaders
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\ShaderFiles C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\ShaderFiles /y /i /s

REM copy the xml files
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\XmlFiles C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\XmlFiles /y /i /s

REM TODO: remove deprecated xml levels