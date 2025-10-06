# ReverseNX-RT

Alternative version of ReverseNX that can switch between handheld and docked mode in **R**eal **T**ime.

Requires SaltyNX 1.4.3+ and Tesla environment installed. Links at the end of readme.

I'm not planning to add any more functionality to it. Next updates will only contain bug fixes.

Overlay contains multiple modes, from which 2 are main ones, other are error notifications.

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/N4N5UMFN)

# Main modes:
- When running game and everything works as should:

![Gra](https://i.imgur.com/ThUbEZ6.jpg) 

You have only two options here:
* Change system control (It also refers to ReverseNX-Tool flags - f.e. if you have docked flag set for this game in ReverseNX-Tool, when system control is enabled, it will force docked mode)
* Change mode (doesn't show up if system control is enabled)

Additional options if it's detected that they can be compatible:
* Change Handheld DDR (doesn't show up if system control is enabled and if game didn't use GetDefaultDisplayResolution even once)
* Change Docked DDR (doesn't show up if system control is enabled and if game didn't use GetDefaultDisplayResolution even once)

And:
* Save settings

> DDR

Some games utilize Default Display Resolution (DDR) to set game's output resolution. List of known compatible games can be found [HERE](https://github.com/masagrator/ReverseNX-RT/blob/compatible_games/README.md). Some games may use that function, but do nothing with the result (For example `Dragon Ball: Xenoverse 2`)

---

# Error notifications:
- **SaltyNX is not working!** - SaltyNX crashed or is not installed correctly.
- **Game was closed! Overlay disabled! Exit from overlay and run game first!** - If you were using overlay in last running game and closed game without closing overlay, you need to close overlay to use it again.
- **ReverseNX-RT is not running!** - plugin was not injected. It's either 32-bit game or is in SaltyNX exceptions list.
- **Game doesn't support changing modes!** - plugin was injected and is working, but function for checking modes was not used. Either game doesn't have one (which means there is no difference between handheld and docked) or game just checks it later than when you wanted to check (for example game may not use it for few seconds after booting was finished). You need to exit from overlay and run it again later to check if error still occurs...
- **WRONG MAGIC!** - something went horribly wrong and overlay is reading value from wrong memory position. It should not happen if you are opening overlay after game boot process will finish. It may also happen if you will close overlay before closing game and open it after running next game, this time not compatible with SaltyNX.
- **Defauly Display Resolution was not checked!** - if it shows up, it means you won't have access to `Change Handheld DDR` and `Change Docked DDR` since resolutions are hardcoded.

# Troubleshooting:
List of titles having compability issues with ReverseNX-RT:

| Title | Versions | Why? |
| ------------- | ------------- | ------------- |
| Robotics;Notes Elite | 1.0.1 | Broken PopNotificationMessage() thread, not working at all |

Q: Often when I'm closing game, Atmosphere crashes with error 0x41001. Why?

A: Atmosphere 0.12.0 with new options for cheat engine bringed bug that causes showing this error in random instances. For some reason this overlay makes this bug occur more frequently. Either go back to 0.11.1 or update to newer version.

# Links:

- https://github.com/masagrator/SaltyNX/releases
- https://github.com/WerWolv/nx-ovlloader
- https://github.com/WerWolv/Tesla-Menu
