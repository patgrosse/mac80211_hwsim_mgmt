# mac80211_hwsim_mgmt
[![Build Status](https://travis-ci.org/patgrosse/mac80211_hwsim_mgmt.svg?branch=master)](https://travis-ci.org/patgrosse/mac80211_hwsim_mgmt)

Management tool for mac80211_hwsim kernel module

## Features
* Create mac80211_hwsim stations after the kernel module has been loaded
* Delete mac80211_hwsim stations
* Set mac80211_hwsim properties per station
* Uses netlink genl messages for kernel communication

## Installation
Will install to `/usr/bin/hwsim_mgmt` and may require root
```bash
git clone https://github.com/patgrosse/mac80211_hwsim_mgmt.git
cd mac80211_hwsim_mgmt
make install
```

### About Set RSSI   
The feature Set RSSI requires minor changes in mac80211_hwsim: https://www.youtube.com/watch?v=gtaHCpaHBGc

### Requirements
* A kernel containing the mac80211_hwsim module
* libevent and at least libnl-2.0

## Usage
```
hwsim_mgmt [OPTION...]

 Modes: [-c [OPTION...]|-d|-x]
  -c, --create               Create a new radio
  -d, --delid=ID             Delete an existing radio by its id
  -x, --delname=NAME         Delete an existing radio by its name
  -k, --setrssi=NUM          Set RSSI to specific radio 

 Create options:
  -a, --alphareg=STR         reg_alpha2 hint
  -n, --name=NAME            The requested name (may not be available)
  -o, --channels=NUM         Number of concurrent channels
  -r, --customreg=REG        reg_domain ID int
  -t, --chanctx              Use chantx (flag)
  -v, --novif                No auto vif (flag)

 General:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```
