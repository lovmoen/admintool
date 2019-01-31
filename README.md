Source Admin Tool with damon-mode
=================================

About
-----
- Fork of Drifter321/admintool
- Contains all functionality of original
- Additional functionality for running as a daemon, posting gameserver status to a REST endpoint
- commandline options to enable damon-mode
  - -d, --daemon => enable daemon-mode (no ui)
  - -s, --servers <filename> => json file containing servers to monitor
  - -r, --rest <url> => URL to post updates to

Third Party
-----------
- From original
  - [miniUPnP](https://github.com/miniupnp/miniupnp): Used for port mapping on UPnP enabled devices (primarily miniupnpc).
  - [libmaxminddb](https://github.com/maxmind/libmaxminddb): Used for GeoIP.
    - Original uses prebuilt version of this library. This fork uses installed libmaxminddb-dev on linux.
  - This product includes GeoLite2 data created by MaxMind, available from http://www.maxmind.com.
- Additional
  - [nlohmann/json](): Used to read json server file + creating json for REST.
