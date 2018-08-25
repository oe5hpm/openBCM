# spec file for package obcm
#
# Copyright (c) 2013 DH8YMB Markus Baumann, Lippstadt, Germany
 
Summary: OpenBCM Packet Radio BBS
Name: obcm
Version: 1.07b13
Release: 1
License: GPL
Group: unsorted
URL: http://dnx274.org/baybox
Source: sobcm-%{version}.tgz
BuildRoot: /var/tmp/%{name}-%{version}-root
Distribution: none
Vendor: Markus Baumann, DH8YMB, Lippstadt, Germany
Packager: Markus Baumann DH8YMB <dh8ymb(at)web.de>
 
%description
OpenBCM is a full featured AX.25 packet radio mailbox for use
in ham radio as well as in citizen band with full store and forward
capabilities. It offers also a lot of TCP/IP services like HTTP,
NNTP, POP3, SMTP and TELNET. The software is available for Linux,
DOS and Windows NT/2000/XP.
 
Authors:
  Markus Baumann, DH8YMB
  Dietmar Zlabinger, OE3DZW
  Florian Radlheer, DL8MBT
  Johann Hanne, DH3MB
  Patrick Sesseler, DF3VI
  et al.
 
%prep
 
%setup
 
%build
make
 
%install
make rpminstall
rm -f %{buildroot}/bcm/bcm
mkdir -p %{buildroot}/bcm
mkdir -p %{buildroot}/bcm/msg
cp -f changes.txt %{buildroot}/bcm
cp -f /bcm/startbcm %{buildroot}/bcm
cp -f /bcm/msg/help.dl %{buildroot}/bcm/msg
cp -f /bcm/msg/help.gb %{buildroot}/bcm/msg
cp -f bcm %{buildroot}/bcm
 
%clean
rm -f /bcm/bcm
rm -f /bcm/changes.txt
 
%files
/bcm/bcm
/bcm/startbcm
/bcm/changes.txt
/bcm/msg/help.dl
/bcm/msg/help.gb
 
