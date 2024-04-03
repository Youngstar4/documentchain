Masternode config
=======================

You can also find instructions on [documentchain.org](https://documentchain.org/support/masternodes)

DMS Core allows controlling multiple remote masternodes from a single wallet. The wallet needs to have a valid collateral output of 5000 DMS for each masternode and uses a configuration file named `masternode.conf` which can be found in the following data directory (depending on your operating system):
 * Windows: %APPDATA%\DMSCore\
 * Mac OS: ~/Library/Application Support/DMSCore/
 * Unix/Linux: ~/.dmscore/
You can open the file via the Tools menu too.

`masternode.conf` is a space separated text file. Each line consists of an alias, IP address followed by port, masternode private key, collateral output transaction id and collateral output index.

Example:
```
MN1 123.123.123.123:41319 5UFi4AmqjhYGqw1sKMfqjVvXeb57yL3aqw5VdWSG18x2xMAmags 7603c20a05258c208b58b0a0d77603b9fc93d47cfa403035f87f3ce0af814566 1
MN2 123.123.123.124:41319 5UL1E4xbdX1V5LKAe3g43PVYkYi1ksNUVXW1zamyGXQGeqHq4dk 5d898e78244f3206e0105f421cdb071d95d111a51cd88eb5511fc0dbf4bfd95f 0
```

In the example above:
* the collateral of 5000 DMS for `MN1` is output `1` of transaction 7603c20a05258c208b58b0a0d77603b9fc93d47cfa403035f87f3ce0af814566
* the collateral of 5000 DMS for `MN2` is output `0` of transaction 5d898e78244f3206e0105f421cdb071d95d111a51cd88eb5511fc0dbf4bfd95f

Enter `masternode outputs` in Debug console to get these values.

The following RPC commands are available (type `help masternode` in Console for more info):
* list-conf
* start-alias \<alias\>
* start-all
* start-missing
* start-disabled
* outputs
