# geoconn

![demo](./demo.gif)

> [!WARNING]
> This is an unfinished hobby project, subject to being abandoned on a whim.

Inspired by the Hollywood depiction of various world maps in places like NASA mission control or token evil organizations,
this is an attack map for the personal computer.

Currently, it merely does geolocation on IP addresses you give it and places a dot on the world map.
Not much else.

It listens on localhost:40343 for TCP connections containing IP addresses, specifically IPv6.
I.e. netcatting a list of IPs is presently the best way of inputting them.
```
nc localhost 40343 < ips.csv
```

You can also request IPs from a running [connwatch](https://github.com/FallibleVagrant/connwatch) instance.

The map jpgs are sourced from NASA, although I found these particular ones after digging through Reddit's community of map enthusiasts.

## Dependencies
This project uses Dear ImGui and stb_image.h, both of which are already included in the project files.
[https://github.com/ocornut/imgui](https://github.com/ocornut/imgui)
[https://github.com/nothings/stb](https://github.com/nothings/stb)

Building requires the SDL2 development files.

## Build from Source

You need to have the SDL2 development files in order to `make` this project.
(We use the SDL2 implementation quickstart example from ImGui.)
```
sudo apt-get install libsdl2-dev
```
Then, simply call:
```
make
```

## Geo Databases
The program is currently hard-coded to use MaxMind's GeoLite2 databases for its geolocation.
It expects them to be in the same directory as the executable.

I have not included them in the repository, as they are under a non-permissive license.
If you would like the databases for yourself, they are provided free of charge after registering with MaxMind.
Alternatively, you can try your luck googling
```
GeoLite2-City-Blocks intitle:"Index of"
```
to see if some schmuck left the csvs public on their website.

To be clear, the proper files this program expects are:
"GeoLite2-City-Blocks-IPv4.csv"
and
"GeoLite2-City-Blocks-IPv6.csv".

This program can't process the mmdb versions of the databases.
I tried integrating MaxMind's libraries for the format into geoconn,
but they didn't play well with g++.

I haven't tested with the GeoIP2 databases (those cost money), but I assume it is much the same.

## License
geoconn is licensed under the GPL, see LICENSE for more information.
