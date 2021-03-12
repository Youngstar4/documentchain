Gitian Building DMS Core
================

In this guide, you will find a brief description of the required steps that has been tested with DMS Core.
Detailed notes and background information can be found in the instructions for 
[Bitcoin Core](https://github.com/bitcoin-core/docs/blob/master/gitian-building.md) 
and [DMS Core](https://github.com/dashpay/dash/blob/master/doc/gitian-building.md).

Gitian is the deterministic build process that is used to build the DMS
Core executables. It provides a way to be reasonably sure that the
executables are really built from the source on GitHub. It also makes sure that
the same, tested dependencies are used and statically built into the executable.

Preparing the Gitian builder host
---------------------------------

The first step is to prepare the host environment that will be used to perform the Gitian builds. 
You need Ubuntu or Debian as environment. Depending on the OS version, there are 
different requirements that can lead to errors during execution.
We used [ubuntu-18.04.5-live-server-amd64.iso](https://ubuntu.com/download/server) for this tutorial.

Set up a new (virtual) machine with Ubuntu 18.04 Server.
As user name please use `gitianuser`.

After Installation
-------------------
SSH login for root users is disabled by default, so we'll enable that now.

Login to the VM using the username `gitianuser` you have chosen before. Set a password for the `root` user:

```bash
sudo su
passwd  # enter password for root
```
Allow root SSH login:
```bash
sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config # or edit sshd_config
systemctl restart ssh
exit
logout
```

Connecting to the VM
----------------------

You can connect to the VM using SSH and files can be copied using SFTP.
On Windows you can use [putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/download.html) or the Windows SSH client.

Setting up Ubuntu for Gitian building
--------------------------------------

In this section we will be setting up the host installation for Gitian building.
This only need to be performed once.

First log in as `root` to set up dependencies and make sure that the
user can use the sudo command. Type/paste the following in the terminal:

```bash
apt install git ruby sudo apt-cacher-ng qemu-utils debootstrap lxc python-cheetah parted kpartx bridge-utils make curl
# if you are using Debian:
apt install ubuntu-archive-keyring
```

Then set up LXC and the rest with the following, which is a complex jumble of settings and workarounds
(replace `gitianuser` if you are using a different username):

```bash
echo "%sudo ALL=NOPASSWD: /usr/bin/lxc-start" > /etc/sudoers.d/gitian-lxc
echo "%sudo ALL=NOPASSWD: /usr/bin/lxc-execute" >> /etc/sudoers.d/gitian-lxc
# make /etc/rc.local script that sets up bridge between guest and host
echo '#!/bin/sh -e' > /etc/rc.local
echo 'brctl addbr lxcbr0' >> /etc/rc.local
echo 'ifconfig lxcbr0 10.0.3.1/24 up' >> /etc/rc.local
echo 'iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE' >> /etc/rc.local
echo 'echo 1 > /proc/sys/net/ipv4/ip_forward' >> /etc/rc.local
echo 'exit 0' >> /etc/rc.local
# make sure that USE_LXC is always set when logging in as debian,
# and configure LXC IP addresses
echo 'export USE_LXC=1' >> /home/gitianuser/.profile
echo 'export GITIAN_HOST_IP=10.0.3.1' >> /home/gitianuser/.profile
echo 'export LXC_GUEST_IP=10.0.3.5' >> /home/gitianuser/.profile
reboot  # on WSL: start cmd as admin "net stop LxssManager" and "net start LxssManager"
```

At the end the VM is rebooted to make sure that the changes take effect. 

Installing Gitian
------------------

Re-login as the user `gitianuser`.

Install `python-vm-builder` package:

```bash
wget http://archive.ubuntu.com/ubuntu/pool/universe/v/vm-builder/vm-builder_0.12.4+bzr494.orig.tar.gz
echo "76cbf8c52c391160b2641e7120dbade5afded713afaa6032f733a261f13e6a8e  vm-builder_0.12.4+bzr494.orig.tar.gz" | sha256sum -c
# (verification -- must return OK)
tar -zxvf vm-builder_0.12.4+bzr494.orig.tar.gz
cd vm-builder-0.12.4+bzr494
sudo python setup.py install
cd ..
```

Clone the git repositories for DMS Core and Gitian.

```bash
git clone https://github.com/devrandom/gitian-builder.git
git clone https://github.com/Krekeler/documentchain.git
git clone https://github.com/dashpay/gitian.sigs.git
git clone https://github.com/dashpay/dash-detached-sigs.git
```

Setting up the Gitian image
-------------------------

Gitian needs a virtual image of the operating system to build in.
Currently this is Ubuntu Trusty x86_64.
This image will be copied and used every time that a build is started to
make sure that the build is deterministic.
Creating the image will take a while, but only has to be done once.

```bash
cd gitian-builder
bin/make-base-vm --lxc --arch amd64 --suite bionic
```

There will be a lot of warnings printed during the build of the image. These can be ignored.

Repeat this step when you have upgraded to a newer version of Gitian.


Getting and building the inputs
--------------------------------

Use the automated script found in [contrib/gitian-build.py](/contrib/gitian-build.py).
This will take several hours.

```bash
cd
cp documentchain/contrib/gitian-build.py ./
./gitian-build.py --setup -c signer=Githubuser version=master  # see Note
export NAME=Githubuser
export VERSION=master
./gitian-build.py --detach-sign --no-commit -j 2 -o lwm -b -c $NAME $VERSION
```
**Note**: if you get the error message *permission denied* from docker:
```bash
sudo usermod -a -G docker $USER
logout # and login again as gitianuser
```
**Notes**
* The Githubuser is necessary if you want to create signed binaries.
* `-j 1` specifies the number of tasks. You can increase the number to speed up, but this may cause memory overflow.
* `-o lwm` specifies the target platform: **l**inux, **w**indows, **m**ac.