Clone spresense-test repo in spritzer_sdk_beta/test,

and move combination_player/combi-player-defconfig to spritzer_sdk_beta/sdk/config .

Edit .config and build it.

build

Ex, 
CONFIG_COMBI_SET_PLAY_ONLY=y
#CONFIG_COMBI_SET_STEP_ONLY=y
#CONFIG_COMBI_SET_PLAY_W_STEP=y

#CONFIG_COMBI_SET_LTE=y
CONFIG_COMBI_SET_LED=y
#CONFIG_COMBI_SET_GNSS=y
CONFIG_COMBI_SET_AUDIO=y

This config are builds Audio player, GPIO.

This combination program follow comand is

lte
led
gnss
audio
video
camera
asmp
decode
step
sh_init
flash
fat



How to use:
Type this.

nsh > combination_player audio led

This interface doesn't metter command order.

nsh > combination_player audio led
nsh > combination_player led audio

it means same.


Attention:
No, 1
For use LTE, 
argv[2] = URL.
Type command this.

nsh > combination_player lte http://www.JonDoe/index.html led

No, 2
Do not use PLAY_ONLY and STEP_ONLY with PLAY_W_STEP.

No, 3
if use PLAY_W_STEP config, type "audio" or "step" command before "sh_init" command.

No, 4
if use asmp command, type argv[1] in "mp3" or "wav" will be using cpu core was decriment.
