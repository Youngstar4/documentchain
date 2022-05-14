DMS Core
==========

This is the official reference wallet for Documentchain's blockchain solution and the digital currency "DMS". It comprises the backbone of the peer-to-peer network. You can [download DMS Core](https://github.com/Krekeler/documentchain/releases) or [build it yourself](#building) using the guides below.

Running
---------------------
The following are some helpful notes on how to run DMS Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/dms-qt` (GUI) or
- `bin/dmsd` (headless)

### Windows

Unpack the files into a directory, and then run dms-qt.exe.

### OS X

Drag DMS-Qt to your applications folder, and then run DMS-Qt.

### Need Help?

* See the [support website](https://documentchain.org/start-now/)
for help and more information.
* Ask your questions in a chat. Links to chat rooms and social media 
can be found in the [website footer](https://documentchain.org/start-now/).

Building
---------------------
The following are developer notes on how to build DMS Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The DMS Core repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Document Revision](/dms-docs/document-revision-data.md)

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Traffic](reduce-traffic.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
