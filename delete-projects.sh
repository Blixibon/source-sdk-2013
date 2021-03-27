# Prompt the user to be sure
echo "This script will remove all vcxproj-related files for VPC changes. Are you sure you wish to continue?"
echo "Hit any key to proceed. Close the window to cancel."
read input

# Clear server projects
rm sp/src/game/server/server_*.vcxproj
rm sp/src/game/server/server_*.vcxproj.filters
rm sp/src/game/server/server_*.vcxproj.vpc_crc
rm sp/src/game/server/server_*.vpc.sentinel
rm mp/src/game/server/server_*.vcxproj
rm mp/src/game/server/server_*.vcxproj.filters
rm mp/src/game/server/server_*.vcxproj.vpc_crc
rm mp/src/game/server/server_*.vpc.sentinel

# Clear client projects
rm sp/src/game/client/client_*.vcxproj
rm sp/src/game/client/client_*.vcxproj.filters
rm sp/src/game/client/client_*.vcxproj.vpc_crc
rm sp/src/game/client/client_*.vpc.sentinel
rm mp/src/game/client/client_*.vcxproj
rm mp/src/game/client/client_*.vcxproj.filters
rm mp/src/game/client/client_*.vcxproj.vpc_crc
rm mp/src/game/client/client_*.vpc.sentinel

# Clear shader projects
rm sp/src/materialsystem/stdshaders/game_shader_dx9_*.vcxproj
rm sp/src/materialsystem/stdshaders/game_shader_dx9_*.vcxproj.filters
rm sp/src/materialsystem/stdshaders/game_shader_dx9_*.vcxproj.vpc_crc
rm sp/src/materialsystem/stdshaders/game_shader_dx9_*.vpc.sentinel
rm mp/src/materialsystem/stdshaders/game_shader_dx9_*.vcxproj
rm mp/src/materialsystem/stdshaders/game_shader_dx9_*.vcxproj.filters
rm mp/src/materialsystem/stdshaders/game_shader_dx9_*.vcxproj.vpc_crc
rm mp/src/materialsystem/stdshaders/game_shader_dx9_*.vpc.sentinel

echo "Old projects purged. Hit any key to continue."
read input