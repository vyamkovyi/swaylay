# sway layout indicator
> swaylay

swaylay is a layout indication utility for
[sway](https://github.com/swaywm/sway).

## Building

Install dependencies:

* git \*
* meson \*
* json-c
* sway

_\*Compile-time dep_

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
