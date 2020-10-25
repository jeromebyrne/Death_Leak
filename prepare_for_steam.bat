REM delete the windows_content folder
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\" /s /q

REM copy just the files in "Game" folder as the first step (exe etc...)
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content /y /i

REM copy the "Media folder"
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\Media C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\Media /y /i /s

REM copy the shaders
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\ShaderFiles C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\ShaderFiles /y /i /s

REM copy the xml files
xcopy C:\Users\jerom\Documents\git\2DPlatformer\Game\XmlFiles C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\XmlFiles /y /i /s

REM delete the steam-appid.txt file used for debugging
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\steam_appid.txt" /s /q

REM delete other unneeded files
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\Game.vcxproj" /s /q
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\Game.vcxproj.user" /s /q
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\Game.log" /s /q
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\DeathLeakGame.ilk" /s /q
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\DeathLeakGame.pdb" /s /q
del "C:\Users\jerom\Documents\git\2DPlatformer\steamworks_sdk_150\sdk\tools\ContentBuilder\content\windows_content\bargon.xml" /s /q

REM TODO: remove deprecated xml levels