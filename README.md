# ReverseNX-RT

Alternative version of ReverseNX that can switch between handheld and docked mode in **R**eal **T**ime.

Requires SaltyNX 0.7.0+ and Tesla environment installed. Links at the end of readme.

It's compatible with ReverseNX patches/ReverseNX-Tool 2.0.0+.

It's not compatible with deprecated ReverseNX plugin.

**Do not use ReverseNX-RT next to Status Monitor 0.6.0 or older** (Tesla can crash on Atmosphere if you are using NX-FPS)

I'm not planning to add any more functionality to it. Next updates will only contain bug fixes.

Overlay contains multiple modes, from which 2 are main ones, other are error notifications.

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/N4N5UMFN)

# Main modes:
- When running game and everything works as should:

![Gra](https://i.imgur.com/ThUbEZ6.jpg) 

You have only two options here:
* Change system control (It also refers to ReverseNX-Tool flags - f.e. if you have docked flag set for this game in ReverseNX-Tool, when system control is enabled, it will force docked mode)
* Change mode (disabled if system control is enabled)

Game stays in current configuration until you will close game.

---

# Error notifications:
- **SaltyNX is not working!** - SaltyNX crashed or is not installed correctly.
- **Game was closed! Overlay disabled! Exit from overlay and run game first!** - If you were using overlay in last running game and closed game without closing overlay, you need to close overlay to use it again.
- **ReverseNX-RT is not running!** - plugin was not injected. It's either 32-bit game or is in SaltyNX exceptions list.
- **Game doesn't support changing modes!** - plugin was injected and is working, but function for checking modes was not used. Either game doesn't have one (which means there is no difference between handheld and docked) or game just checks it later than when you wanted to check (for example game may not use it for few seconds after booting was finished). You need to exit from overlay and run it again later to check if error still occurs...
- **WRONG MAGIC!** - something went horribly wrong and overlay is reading value from wrong memory position. It should not happen if you are opening overlay after game boot process will finish. It may also happen if you will close overlay before closing game and open it after running next game, this time not compatible with SaltyNX.

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
