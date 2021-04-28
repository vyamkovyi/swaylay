# swaylay

swaylay is a layout indicator for [sway](https://github.com/swaywm/sway).

Note that Waybar [has this implemented](https://github.com/Alexays/Waybar/blob/master/man/waybar-sway-language.5.scd) already.

## Usage

```
Usage swaylay [options] [identifier]

  -h, --help             Show help message and quit.
  -s, --socket <socket>  Use the specified socket.
  -v, --version          Show the version number and quit.

Default identifier is 1:1:AT_Translated_Set_2_keyboard.
```

## Building

Install dependencies:

* git, meson, ninja \*
* json-c
* sway

_\*Compile-time deps_

Run these commands:

```shell
$ meson build
$ ninja -C build
$ sudo ninja -C build install
```

## License

This code is licensed under MIT license.
See [LICENSE](LICENSE) for more information.

This code is heavily based on swaymsg and other [sway](https://swaywm.org/)
parts, made by [Drew DeVault](https://drewdevault.com/).
